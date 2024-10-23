// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../../../lib/fftw3/fftw3.h"
#include "../parameter.hpp"
#include "spectralmask.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <mutex>
#include <numbers>
#include <numeric>

namespace SomeDSP {

// `fftwMutex` is used to lock FFTW3 calls except `fftw*_execute`. In other words, FFTW3
// isn't thread safe except `fftw*_execute` call.
extern std::mutex fftwMutex;

// Fast Walsh-Hadamard transform. In-place.
template<typename T> void fwht(int N, T *seq, bool inverse = false)
{
  for (int h = 2; h <= N; h *= 2) {
    int hf = h / 2;
    for (int i = 0; i < N; i += h) {
      for (int j = 0; j < hf; ++j) {
        auto u = seq[i + j];
        auto v = seq[i + j + hf];
        seq[i + j] = u + v;
        seq[i + j + hf] = u - v;
      }
    }
  }
  if (inverse) {
    for (int i = 0; i < N; ++i) seq[i] /= N;
  }
}

// Fast Walsh-Hadamard transform. Out-of-place.
template<typename T> void fwht(int N, T *x, T *y, bool inverse = false)
{
  for (int h = 2; h <= N; h *= 2) {
    int hf = h / 2;
    for (int i = 0; i < N; i += h) {
      for (int j = 0; j < hf; ++j) {
        auto u = x[i + j];
        auto v = x[i + j + hf];
        y[i + j] = u + v;
        y[i + j + hf] = u - v;
      }
    }
  }
  if (inverse) {
    for (int i = 0; i < N; ++i) x[i] /= N;
  }
}

// Out-of-place. No scaling on forward transform.
template<typename T> void haarTransformForward(int N, T *x, T *y)
{
  std::fill(y, y + N, T(0));
  y[0] = std::accumulate(x, x + N, T(0));

  auto size = N;
  auto half = size / 2;
  auto start = 0;
  for (int row = 1; row < N; ++row) {
    for (int idx = 0; idx < size; ++idx) {
      auto w = x[start + idx];
      y[row] += idx < half ? w : -w;
    }

    start += size;
    if (start >= N) {
      start = 0;
      size /= 2;
      half /= 2;
    }
  }
}

// Out-of-place.
template<typename T> void haarTransformBackward(int N, T *x, T *y)
{
  auto scaler = T(1) / T(N);

  std::fill(y, y + N, scaler * x[0]);

  auto size = N;
  auto half = size / 2;
  auto start = 0;
  for (int row = 1; row < N; ++row) {
    for (int idx = 0; idx < size; ++idx) {
      auto w = scaler * x[row];
      y[start + idx] += idx < half ? w : -w;
    }

    start += size;
    if (start >= N) {
      start = 0;
      size /= 2;
      half /= 2;
      scaler *= 2;
    }
  }
}

inline int bitReversal(int n, int k)
{
  int r = 0;
  for (int i = 0; i < k; ++i) r |= ((n >> i) & 1) << (k - i - 1);
  return r;
}

inline int binaryToGrayCode(int n) { return n ^ (n >> 1); }

inline int grayCodeToBinary(int n)
{
  int mask = n;
  while (mask) {
    mask >>= 1;
    n ^= mask;
  }
  return n;
}

template<int maxFrameSizeLog2> struct SideChainMask {
  static constexpr int maxFrameSize = int(1) << maxFrameSizeLog2;
  static constexpr int maxSpectrumSize = maxFrameSize / 2 + 1;

  float *bufW = nullptr;                  // buffer for write.
  float *bufMask = nullptr;               // buffer for read.
  std::complex<float> *spcMask = nullptr; // spectrum.

  static constexpr int planIndexOffset = 2; // Starts from 2^2.
  static constexpr int nPlan = maxFrameSizeLog2 - planIndexOffset + 1;
  std::array<fftwf_plan, nPlan> forwardPlan;

  SideChainMask()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    bufW = (float *)fftwf_malloc(sizeof(float) * maxFrameSize);
    bufMask = (float *)fftwf_malloc(sizeof(float) * maxFrameSize);
    spcMask = (std::complex<float> *)fftwf_malloc(
      sizeof(std::complex<float>) * maxSpectrumSize);
    for (int idx = 0; idx < nPlan; ++idx) {
      const auto spc_ = reinterpret_cast<fftwf_complex *>(spcMask);
      const int length = int(1) << (idx + 2);
      forwardPlan[idx] = fftwf_plan_dft_r2c_1d(length, bufW, spc_, FFTW_ESTIMATE);
    }

    reset();
  }

  ~SideChainMask()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    for (auto &x : forwardPlan) fftwf_destroy_plan(x);
    if (spcMask) fftwf_free(spcMask);
    if (bufMask) fftwf_free(bufMask);
    if (bufW) fftwf_free(bufW);
  }

  void reset(int indexOffset = 0)
  {
    std::fill(bufW, bufW + maxFrameSize, float(0));
    std::fill(bufMask, bufMask + maxFrameSize, float(0));
    std::fill(spcMask, spcMask + maxSpectrumSize, std::complex<float>{0, 0});
  }

  void push(float input, const int bufIndex) { bufW[bufIndex] = input; }
  void processFft(const int planIndex) { fftwf_execute(forwardPlan[planIndex]); }
  void processFwht(const int size) { fwht(size, bufW, bufMask, false); }
  void processHaar(const int size) { haarTransformForward(size, bufW, bufMask); }
};

template<int maxFrameSizeLog2> struct SpectralDelay {
public:
  static constexpr int maxFrameSize = int(1) << maxFrameSizeLog2;
  static constexpr int maxSpectrumSize = maxFrameSize / 2 + 1;

  int bufIndex = 0;

  float *bufW = nullptr;   // buffer for write.
  float *bufR = nullptr;   // buffer for read.
  float *bufTmp = nullptr; // temporary buffer.

  std::complex<float> *spcSrc = nullptr; // source spectrum.
  std::complex<float> *spcTmp = nullptr; // temporary spectrum.

  std::array<float, maxFrameSize> mask{};

  static constexpr int planIndexOffset = 2; // Starts from 2^2.
  static constexpr int nPlan = maxFrameSizeLog2 - planIndexOffset + 1;
  std::array<fftwf_plan, nPlan> forwardPlan;
  std::array<fftwf_plan, nPlan> inversePlan;

  SideChainMask<maxFrameSizeLog2> side;

  SpectralDelay()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    bufW = (float *)fftwf_malloc(sizeof(float) * maxFrameSize);
    bufR = (float *)fftwf_malloc(sizeof(float) * maxFrameSize);
    bufTmp = (float *)fftwf_malloc(sizeof(float) * maxFrameSize);

    constexpr int spcAllocSize = sizeof(std::complex<float>) * maxSpectrumSize;
    spcSrc = (std::complex<float> *)fftwf_malloc(spcAllocSize);
    spcTmp = (std::complex<float> *)fftwf_malloc(spcAllocSize);

    for (int idx = 0; idx < nPlan; ++idx) {
      const auto spc_ = reinterpret_cast<fftwf_complex *>(spcSrc);
      const int length = int(1) << (idx + 2);
      forwardPlan[idx] = fftwf_plan_dft_r2c_1d(length, bufW, spc_, FFTW_ESTIMATE);
      inversePlan[idx] = fftwf_plan_dft_c2r_1d(length, spc_, bufR, FFTW_ESTIMATE);
    }

    reset();
  }

  ~SpectralDelay()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    for (auto &x : inversePlan) fftwf_destroy_plan(x);
    for (auto &x : forwardPlan) fftwf_destroy_plan(x);

    if (spcTmp) fftwf_free(spcTmp);
    if (spcSrc) fftwf_free(spcSrc);

    if (bufTmp) fftwf_free(bufTmp);
    if (bufR) fftwf_free(bufR);
    if (bufW) fftwf_free(bufW);
  }

  void reset(int indexOffset = 0)
  {
    bufIndex = indexOffset;
    std::fill(bufW, bufW + maxFrameSize, float(0));
    std::fill(bufR, bufR + maxFrameSize, float(0));
    std::fill(bufTmp, bufTmp + maxFrameSize, float(0));
    std::fill(spcSrc, spcSrc + maxSpectrumSize, std::complex<float>{0, 0});
    std::fill(spcTmp, spcTmp + maxSpectrumSize, std::complex<float>{0, 0});
    mask.fill({});
    side.reset();
  }

  float process(float input, float sideIn, SpectralParameter &prm)
  {
    const auto output = bufR[bufIndex];
    bufW[bufIndex] = input;
    side.push(sideIn, bufIndex);
    if (++bufIndex < prm.frmSize) return output;
    bufIndex = 0;

    const auto planIndex = prm.frameSizeLog2 - planIndexOffset;
    fftwf_execute(forwardPlan[planIndex]);

    const auto spectrumSize = prm.frmSize / 2 + 1;

    if (prm.sideChain) {
      side.processFft(planIndex);
      const auto gain = float(1) / std::sqrt(float(prm.frmSize));
      for (int idx = 0; idx < spectrumSize; ++idx) {
        spcSrc[idx] *= std::abs(side.spcMask[idx]) * gain;
      }
    }

    fillMask(spectrumSize, mask.data(), prm);

    const int spcShift = prm.spectralShift * spectrumSize;
    std::rotate(spcTmp, spcTmp + spcShift, spcTmp + spectrumSize);

    spcTmp[0] *= float(1) - prm.octaveDown;

    for (int idx = 0; idx < spectrumSize; ++idx) {
      spcTmp[idx] *= prm.feedback;
      const auto maskValue = mask[idx] > prm.maskThreshold
        ? mask[idx] * std::polar(float(1), prm.maskRotation * mask[idx])
        : float(0);

      if ((idx & 1) == 0) {
        auto value = spcTmp[idx];
        spcTmp[idx] = (float(1) - prm.octaveDown) * value;
        spcTmp[idx / 2] += prm.octaveDown * value;
      } else {
        auto value = spcTmp[idx];
        spcTmp[idx] = (float(1) - prm.octaveDown) * value;
        spcTmp[idx / 2] += prm.octaveDown * value / float(2);
        spcTmp[idx / 2 + 1] += prm.octaveDown * value / float(2);
      }

      spcTmp[idx] += spcSrc[idx] / float(prm.frmSize) * maskValue;
      spcSrc[idx] = spcTmp[idx] * std::abs(maskValue);
    }

    fftwf_execute(inversePlan[planIndex]);
    return output;
  }

  float processFwht(float input, float sideIn, SpectralParameter &prm)
  {
    const auto output = bufW[bufIndex];
    bufW[bufIndex] = input;
    side.push(sideIn, bufIndex);
    if (++bufIndex < prm.frmSize) return output;
    bufIndex = 0;

    fwht(prm.frmSize, bufW, false);

    if (prm.sideChain) {
      side.processFwht(prm.frmSize);
      for (int idx = 0; idx < prm.frmSize; ++idx) bufW[idx] *= side.bufMask[idx];
    }

    const int spcShift = prm.spectralShift * prm.frmSize;
    std::rotate(bufR, bufR + spcShift, bufR + prm.frmSize);

    fillMask(prm.frmSize, mask.data(), prm);

    auto rotation = prm.maskRotation / float(2) / std::numbers::pi_v<float>;
    rotation -= std::floor(rotation);
    int rotIdx = int(prm.frmSize * rotation);

    bufR[0] *= float(1) - prm.octaveDown;

    for (int idx = 0; idx < prm.frmSize; ++idx) {
      // Using an implementation detail that `prm.frmSize` is always 2^n.
      const auto permSrc = (idx + rotIdx) & (prm.frmSize - 1);
      const auto permuted = binaryToGrayCode(bitReversal(permSrc, prm.frameSizeLog2));
      bufR[idx] *= prm.feedback;

      if ((idx & 1) == 0) {
        auto value = bufR[idx];
        bufR[idx] = (float(1) - prm.octaveDown) * value;
        bufR[idx / 2] += prm.octaveDown * value;
      } else {
        auto value = bufR[idx];
        bufR[idx] = (float(1) - prm.octaveDown) * value;
        bufR[idx / 2] += prm.octaveDown * value / float(2);
        bufR[idx / 2 + 1] += prm.octaveDown * value / float(2);
      }

      const auto maskValue = mask[idx] > prm.maskThreshold ? mask[idx] : 0;
      bufR[idx] += bufW[permuted] * maskValue / float(prm.frmSize);
      bufW[permuted] = bufR[idx] * std::abs(maskValue);
    }

    fwht(prm.frmSize, bufW, false);
    return output;
  }

  float processHaar(float input, float sideIn, SpectralParameter &prm)
  {
    const auto output = bufW[bufIndex];
    bufW[bufIndex] = input;
    side.push(sideIn, bufIndex);
    if (++bufIndex < prm.frmSize) return output;
    bufIndex = 0;

    haarTransformForward(prm.frmSize, bufW, bufTmp);

    if (prm.sideChain) {
      side.processHaar(prm.frmSize);
      for (int idx = 0; idx < prm.frmSize; ++idx) bufTmp[idx] *= side.bufMask[idx];
    }

    const int spcShift = prm.spectralShift * prm.frmSize;
    std::rotate(bufR, bufR + spcShift, bufR + prm.frmSize);

    auto rotation = prm.maskRotation / float(2) / std::numbers::pi_v<float>;
    rotation -= std::floor(rotation);
    int rotIdx = int(prm.frmSize * rotation);

    bufR[0] *= float(1) - prm.octaveDown;

    fillMask(prm.frmSize, mask.data(), prm);
    for (int idx = 0; idx < prm.frmSize; ++idx) {
      bufR[idx] *= prm.feedback;

      if ((idx & 1) == 0) {
        auto value = bufR[idx];
        bufR[idx] = (float(1) - prm.octaveDown) * value;
        bufR[idx / 2] += prm.octaveDown * value;
      } else {
        auto value = bufR[idx];
        bufR[idx] = (float(1) - prm.octaveDown) * value;
        bufR[idx / 2] += prm.octaveDown * value / float(2);
        bufR[idx / 2 + 1] += prm.octaveDown * value / float(2);
      }

      const auto maskValue = mask[idx] > prm.maskThreshold ? mask[idx] : 0;
      bufR[idx] *= std::abs(maskValue);

      const auto src
        = std::lerp(bufTmp[idx], bufTmp[(idx + rotIdx) & (prm.frmSize - 1)], rotation);
      bufR[idx] += src * maskValue;
    }

    haarTransformBackward(prm.frmSize, bufR, bufW);
    return output;
  }
};

template<int maxFrameSizeLog2> class SpectralProcessor {
private:
  static constexpr int maxFrameSize = int(1) << maxFrameSizeLog2;

  int prevFrameSizeLog2 = 0;
  int fadeIndex = 0;
  std::array<float, maxFrameSize> latencyDelay{};
  std::array<SpectralDelay<maxFrameSizeLog2>, 2> spc;

public:
  void reset(int frameSizeLog2)
  {
    prevFrameSizeLog2 = frameSizeLog2;
    fadeIndex = 0;
    latencyDelay.fill(0);

    const auto length = int(1) << frameSizeLog2;
    for (int i = 0; i < spc.size(); ++i) spc[i].reset((i * length) / spc.size());
  }

  float process(float input, float sideIn, SpectralParameter &prm)
  {
    if (prevFrameSizeLog2 != prm.frameSizeLog2) {
      prevFrameSizeLog2 = prm.frameSizeLog2;
      spc[0].bufIndex = 0;
      spc[1].bufIndex = prm.frmSize / 2;
      fadeIndex = 0;
    }

    std::array<float, 2> out{};
    switch (prm.transform) {
      default: { // case TransformType::fft
        out[0] = spc[0].process(input, sideIn, prm);
        out[1] = spc[1].process(input, sideIn, prm);
      } break;
      case TransformType::fwht: {
        out[0] = spc[0].processFwht(input, sideIn, prm);
        out[1] = spc[1].processFwht(input, sideIn, prm);
      } break;
      case TransformType::haar: {
        out[0] = spc[0].processHaar(input, sideIn, prm);
        out[1] = spc[1].processHaar(input, sideIn, prm);
      } break;
    }

    constexpr auto twopi = float(2) * std::numbers::pi_v<float>;

    if (fadeIndex >= prm.frmSize) fadeIndex = 0;
    const auto crossFade
      = float(0.5) + float(0.5) * std::cos(twopi * float(fadeIndex) / float(prm.frmSize));

    const auto delayed = latencyDelay[fadeIndex];
    latencyDelay[fadeIndex] = input;

    ++fadeIndex;

    const auto mixed = std::lerp(out[0], out[1], crossFade);
    return prm.reportLatency ? std::lerp(delayed, mixed, prm.dryWetMix)
                             : std::lerp(input, mixed, prm.dryWetMix);
  }
};

template<typename Sample> struct LfoPhase {
  Sample phase = 0;
  Sample lfoFreq = 0;

  void reset(Sample initialPhase, Sample targetFreq)
  {
    phase = initialPhase;
    lfoFreq = targetFreq;
  }

  Sample processSync(Sample targetPhase, Sample targetFreq, Sample syncRate)
  {
    phase += lfoFreq;
    phase -= std::floor(phase);

    // Sync phase.
    constexpr auto tolerance = Sample(1) / Sample(1024);
    auto d1 = targetPhase - phase;
    auto diff = d1 < 0 ? (d1 + 1 < -d1 ? d1 + 1 : d1) : (-(d1 - 1) < d1 ? d1 - 1 : d1);
    auto absed = std::abs(diff);
    phase += syncRate * (absed >= tolerance ? absed : diff);

    // Sync freq.
    lfoFreq += syncRate * (targetFreq - lfoFreq);

    return phase;
  }

  Sample process(Sample freqNormalized)
  {
    auto output = phase;
    phase += freqNormalized;
    phase -= std::floor(phase);
    return output;
  }
};

} // namespace SomeDSP
