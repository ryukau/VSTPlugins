// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/fftw3/fftw3.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <mutex>
#include <numeric>
#include <vector>

namespace SomeDSP {

inline std::vector<float>
getNuttallFir(size_t nTap, float sampleRate, float cutoffHz, bool isHighpass)
{
  const auto nyquist = sampleRate / float(2);
  if (cutoffHz > nyquist) cutoffHz = nyquist;

  bool isEven = (nTap / 2 & 1) == 0;
  size_t end = nTap;
  if (isEven) --end; // Always use odd length FIR.

  std::vector<float> coefficient(nTap);

  auto mid = float(end - 1) / float(2);
  auto cutoff = float(twopi) * cutoffHz / sampleRate;
  for (size_t idx = 0; idx < end; ++idx) {
    float m = float(idx) - mid;
    float x = cutoff * m;
    coefficient[idx] = x == 0 ? float(1) : std::sin(x) / (x);
  }

  // Apply Nuttall window.
  float tpN = float(twopi) / float(end - 1);
  for (size_t n = 0; n < end; ++n) {
    auto c0 = float(0.3635819);
    auto c1 = float(0.4891775) * std::cos(tpN * n);
    auto c2 = float(0.1365995) * std::cos(tpN * n * float(2));
    auto c3 = float(0.0106411) * std::cos(tpN * n * float(3));
    coefficient[n] *= c0 - c1 + c2 - c3;
  }

  // Normalize to fix FIR scaling.
  float sum = std::accumulate(coefficient.begin(), coefficient.end(), float(0));
  for (size_t idx = 0; idx < end; ++idx) coefficient[idx] /= sum;

  if (isHighpass) {
    for (size_t idx = 0; idx < end; ++idx) coefficient[idx] = -coefficient[idx];
    coefficient[size_t(mid)] += float(1);
  }

  return coefficient;
}

template<typename Sample, size_t nTap> class DirectConvolver {
private:
  std::array<Sample, nTap> co{};
  std::array<Sample, nTap> buf{};

public:
  void setFir(std::vector<float> &source)
  {
    if (source.size() < nTap) return;
    std::copy(source.begin(), source.begin() + nTap, co.begin());
  }

  void reset() { buf.fill({}); }

  Sample process(Sample input)
  {
    std::rotate(buf.rbegin(), buf.rbegin() + 1, buf.rend());
    buf[0] = input;

    Sample output = 0;
    for (size_t n = 0; n < nTap; ++n) output += buf[n] * co[n];
    return output;
  }
};

class OverlapSaveConvolver {
private:
  // `fftwMutex` is used to lock FFTW3 calls except `fftw*_execute`.
  static std::mutex fftwMutex;

  static constexpr size_t nBuffer = 2;

  size_t half = 1;
  size_t bufSize = 2;
  size_t spcSize = 1; // spc = spectrum.

  std::array<float *, nBuffer> buf;
  std::complex<float> *spc;
  std::complex<float> *fir;
  float *flt; // filtered.
  float *coefficient;

  std::array<fftwf_plan, nBuffer> forwardPlan;
  fftwf_plan inversePlan;
  fftwf_plan firPlan;

  size_t front = 0;
  std::array<size_t, nBuffer> wptr{};
  size_t rptr = 0;
  size_t offset = 0;

public:
  void init(size_t nTap, size_t delay = 0)
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    offset = delay;

    half = nTap;
    bufSize = 2 * half;
    spcSize = nTap + 1;

    for (size_t idx = 0; idx < nBuffer; ++idx) {
      buf[idx] = (float *)fftwf_malloc(sizeof(float) * bufSize);
    }
    spc = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    flt = (float *)fftwf_malloc(sizeof(float) * bufSize);

    coefficient = (float *)fftwf_malloc(sizeof(float) * bufSize);
    std::fill(coefficient, coefficient + bufSize, float(0));

    fir = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    std::fill(fir, fir + spcSize, std::complex<float>(0, 0));

    for (size_t idx = 0; idx < nBuffer; ++idx) {
      forwardPlan[idx] = fftwf_plan_dft_r2c_1d(
        int(bufSize), buf[idx], reinterpret_cast<fftwf_complex *>(spc), FFTW_ESTIMATE);
    }
    inversePlan = fftwf_plan_dft_c2r_1d(
      int(bufSize), reinterpret_cast<fftwf_complex *>(spc), flt, FFTW_ESTIMATE);
    firPlan = fftwf_plan_dft_r2c_1d(
      int(bufSize), coefficient, reinterpret_cast<fftwf_complex *>(fir), FFTW_ESTIMATE);
  }

  ~OverlapSaveConvolver()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    for (auto &fp : forwardPlan) fftwf_destroy_plan(fp);
    fftwf_destroy_plan(inversePlan);
    fftwf_destroy_plan(firPlan);

    for (auto &bf : buf) fftwf_free(bf);
    fftwf_free(spc);
    fftwf_free(fir);
    fftwf_free(flt);
    fftwf_free(coefficient);
  }

  void setFir(std::vector<float> &source, size_t start, size_t end)
  {
    std::copy(source.begin() + start, source.begin() + end, coefficient);

    // FFT scaling.
    for (size_t idx = 0; idx < half; ++idx) coefficient[idx] /= float(bufSize);

    fftwf_execute(firPlan);
  }

  void reset()
  {
    wptr[0] = half + offset;
    wptr[1] = offset;
    for (auto &w : wptr) w %= bufSize;
    front = wptr[1] < wptr[0] ? 0 : 1;
    rptr = half + offset % half;

    for (size_t idx = 0; idx < nBuffer; ++idx) {
      std::fill(buf[idx], buf[idx] + bufSize, float(0));
    }
    std::fill(spc, spc + spcSize, std::complex<float>(0, 0));
    std::fill(flt, flt + bufSize, float(0));
  }

  float process(float input)
  {
    buf[0][wptr[0]] = input;
    buf[1][wptr[1]] = input;

    for (auto &w : wptr) {
      if (++w >= bufSize) w = 0;
    }

    if (wptr[front] == 0) {
      fftwf_execute(forwardPlan[front]);
      for (size_t i = 0; i < spcSize; ++i) spc[i] *= fir[i];
      fftwf_execute(inversePlan);

      front ^= 1;
    }

    if (++rptr >= bufSize) rptr = half;
    return flt[rptr];
  }
};

/**
FFT convolver without latency.

It must be `lengthInPow2 > minBlockSizeInPowerOfTwo`.

Signal to noise ratio is around -120 dB (1 : 1e-6). Slightly worse than direct
overlap-add.

This is an impelmentation of minimum computation cost solution in following paper:
- William G. Gardner, 1993-11-11, "Efficient Convolution Without Latency"
*/
template<size_t lengthInPow2, size_t minBlockSizeInPow2 = 4> class ImmediateConvolver {
private:
  static constexpr size_t nTap = size_t(1) << lengthInPow2;
  static constexpr size_t nFftConvolver = lengthInPow2 - minBlockSizeInPow2;

  DirectConvolver<float, size_t(1) << minBlockSizeInPow2> firstConvolver;
  std::array<OverlapSaveConvolver, nFftConvolver> fftConvolver;
  std::array<float, nFftConvolver> sumBuffer{};

public:
  ImmediateConvolver()
  {
    static_assert(
      lengthInPow2 > minBlockSizeInPow2,
      "ImmediateConvolver: lengthInPow2 must be greater than minBlockSizeInPow2.");

    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      fftConvolver[idx].init(size_t(1) << (minBlockSizeInPow2 + idx));
    }

    reset();
  }

  inline size_t latency()
  {
    // Latency of FIR filter specific to `refreshFir()`.
    return nTap / 2 - 1;
  }

  void refreshFir(float sampleRate, float cutoffHz, bool isHighpass)
  {
    auto coefficient = getNuttallFir(nTap, sampleRate, cutoffHz, isHighpass);

    // Set FIR coefficients.
    firstConvolver.setFir(coefficient);
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      fftConvolver[idx].setFir(
        coefficient, size_t(1) << (minBlockSizeInPow2 + idx),
        size_t(1) << (minBlockSizeInPow2 + idx + 1));
    }
  }

  void setFir(std::vector<float> &source)
  {
    if (source.size() < nTap) source.resize(nTap);

    firstConvolver.setFir(source);
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      size_t start = size_t(1) << (minBlockSizeInPow2 + idx);
      size_t end = size_t(1) << (minBlockSizeInPow2 + idx + 1);
      fftConvolver[idx].setFir(source, start, end);
    }
  }

  void reset()
  {
    firstConvolver.reset();
    for (auto &conv : fftConvolver) conv.reset();
    sumBuffer.fill({});
  }

  float process(float input)
  {
    float output = std::accumulate(sumBuffer.begin(), sumBuffer.end(), float(0));

    for (size_t idx = 0; idx < nFftConvolver; ++idx)
      sumBuffer[idx] = fftConvolver[idx].process(input);

    return output + firstConvolver.process(input);
  }
};

class FixedIntDelayVector {
public:
  std::vector<float> buf{};
  size_t ptr = 0;

  void resize(size_t size) { buf.resize(size); }
  void reset(float value = 0) { std::fill(buf.begin(), buf.end(), value); }

  float process(float input)
  {
    if (++ptr >= buf.size()) ptr -= buf.size();
    auto output = buf[ptr];
    buf[ptr] = input;
    return output;
  }
};

/**
A variation of convolver without latency.

SplitConvolver splits filter kernel into several blocks, then compute the blocks in
different timings. This is a mitigation of CPU load spikes that's caused by FFT.

Memory usage can be reduced by sharing input buffer.
*/
template<size_t nBlock, size_t blockSizeInPow2, size_t minBlockSizeInPow2 = 4>
class SplitConvolver {
public:
  static constexpr size_t nTap = nBlock * (size_t(1) << blockSizeInPow2);
  static constexpr size_t nFftConvolver = nBlock - 2;
  static constexpr size_t blockSize = size_t(1) << blockSizeInPow2;

  ImmediateConvolver<blockSizeInPow2, minBlockSizeInPow2> immediateConvolver;
  OverlapSaveConvolver firstFftConvolver;
  std::array<OverlapSaveConvolver, nFftConvolver> fftConvolver;
  std::array<FixedIntDelayVector, nFftConvolver> outputDelay;
  float buf = 0;

  SplitConvolver()
  {
    firstFftConvolver.init(blockSize, blockSize / nBlock);
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      size_t offset = (idx + 2) * blockSize / nBlock;
      fftConvolver[idx].init(blockSize, offset);
      outputDelay[idx].resize((idx + 1) * blockSize + 1);
    }
  }

  void reset()
  {
    immediateConvolver.reset();

    firstFftConvolver.reset();
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      fftConvolver[idx].reset();
      outputDelay[idx].reset();
    }
    buf = 0;
  }

  void refreshFir(float sampleRate, float cutoffHz, bool isHighpass)
  {
    auto coefficient = getNuttallFir(nTap, sampleRate, cutoffHz, isHighpass);

    immediateConvolver.setFir(coefficient);
    firstFftConvolver.setFir(coefficient, blockSize, 2 * blockSize);
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      fftConvolver[idx].setFir(coefficient, (idx + 2) * blockSize, (idx + 3) * blockSize);
    }
  }

  float process(float input)
  {
    auto output = immediateConvolver.process(input);
    output += buf;
    buf = firstFftConvolver.process(input);
    for (size_t idx = 0; idx < nFftConvolver; ++idx) {
      auto value = fftConvolver[idx].process(input);
      output += outputDelay[idx].process(value);
    }
    return output;
  }
};

} // namespace SomeDSP
