// (c) 2022 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/fftw3/fftw3.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <numeric>

namespace SomeDSP {

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

class FFTConvolver {
private:
  static constexpr size_t nBuffer = 2;

  size_t half = 1;
  size_t bufSize = 2;
  size_t spcSize = 1; // spc = spectrum.

  float *buf;
  std::complex<float> *spc;
  std::complex<float> *fir;
  std::array<float *, nBuffer> flt; // filtered.

  fftwf_plan forwardPlan;
  std::array<fftwf_plan, nBuffer> inversePlan;

  size_t front = 0;
  size_t wptr = 0;
  std::array<size_t, nBuffer> rptr{};

public:
  void init(size_t nTap)
  {
    half = nTap;
    bufSize = 2 * half;
    spcSize = nTap + 1;

    buf = (float *)fftwf_malloc(sizeof(float) * bufSize);
    spc = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    for (size_t idx = 0; idx < nBuffer; ++idx) {
      flt[idx] = (float *)fftwf_malloc(sizeof(float) * bufSize);
    }

    fir = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    std::fill(fir, fir + spcSize, std::complex<float>(0, 0));

    forwardPlan = fftwf_plan_dft_r2c_1d(
      int(bufSize), buf, reinterpret_cast<fftwf_complex *>(spc), FFTW_ESTIMATE);
    for (size_t idx = 0; idx < nBuffer; ++idx) {
      inversePlan[idx] = fftwf_plan_dft_c2r_1d(
        int(bufSize), reinterpret_cast<fftwf_complex *>(spc), flt[idx], FFTW_ESTIMATE);
    }
  }

  ~FFTConvolver()
  {
    fftwf_destroy_plan(forwardPlan);
    for (auto &ip : inversePlan) fftwf_destroy_plan(ip);

    fftwf_free(buf);
    fftwf_free(spc);
    fftwf_free(fir);
    for (auto &fl : flt) fftwf_free(fl);
  }

  void setFir(std::vector<float> &source, size_t start, size_t end)
  {
    float *coefficient = (float *)fftwf_malloc(sizeof(float) * bufSize);
    std::copy(source.begin() + start, source.begin() + end, coefficient);
    std::fill(coefficient + half, coefficient + bufSize, float(0));

    // FFT scaling.
    for (size_t idx = 0; idx < half; ++idx) coefficient[idx] /= float(bufSize);

    auto firPlan = fftwf_plan_dft_r2c_1d(
      int(bufSize), coefficient, reinterpret_cast<fftwf_complex *>(fir), FFTW_ESTIMATE);
    fftwf_execute(firPlan);

    fftwf_destroy_plan(firPlan);
    fftwf_free(coefficient);
  }

  void reset()
  {
    front = 0;
    wptr = 0;
    rptr[0] = 1;
    rptr[1] = 1 + half;

    std::fill(buf, buf + bufSize, float(0));
    std::fill(spc, spc + spcSize, std::complex<float>(0, 0));
    for (size_t idx = 0; idx < nBuffer; ++idx) {
      std::fill(flt[idx], flt[idx] + bufSize, float(0));
    }
  }

  float process(float input)
  {
    buf[wptr] = input;

    if (++wptr >= half) {
      wptr = 0;
      front ^= 1;

      fftwf_execute(forwardPlan);
      for (size_t i = 0; i < spcSize; ++i) spc[i] *= fir[i];
      fftwf_execute(inversePlan[front]);
    }

    float output = flt[0][rptr[0]] + flt[1][rptr[1]];
    for (auto &r : rptr) {
      if (++r >= bufSize) r = 0;
    }
    return output;
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
  std::array<FFTConvolver, nFftConvolver> fftConvolver;
  std::array<float, nFftConvolver + 1> sumBuffer{};

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

    sumBuffer.back() = firstConvolver.process(input);

    for (size_t idx = 0; idx < nFftConvolver; ++idx)
      sumBuffer[idx] = fftConvolver[idx].process(input);

    return output;
  }
};

} // namespace SomeDSP
