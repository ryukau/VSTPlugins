// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../parameter.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace SomeDSP {

struct SpectralParameter {
  int sideChain = 0;     // 0: false, 1: true.
  int reportLatency = 0; // 0: false, 1: true.
  int frameSizeLog2 = 2;
  int frmSize = 4;
  TransformType transform = TransformType::fft;
  MaskWaveform maskWaveform = MaskWaveform::cosine;
  float dryWetMix = float(1);
  float feedback = 0;
  float spectralShift = 0;
  float octaveDown = 0;
  float maskMix = float(1);
  float maskPhase = 0;
  float maskFreq = 0;
  float maskThreshold = 0;
  float maskRotation = 0;
};

// Returns a gain for normalization.
inline void fillMask(int maskSize, float *mask, SpectralParameter &prm)
{
  switch (prm.maskWaveform) {
    default: { // case MaskWaveform::cosine: {
      constexpr float twopi = float(2) * std::numbers::pi_v<float>;
      float phase = prm.maskPhase;
      for (int idx = 0; idx < maskSize; ++idx) {
        phase -= std::floor(phase);
        mask[idx] = std::cos(twopi * phase);
        mask[idx] = std::lerp(float(1), mask[idx], prm.maskMix);
        phase += prm.maskFreq;
      }
    } break;
    case MaskWaveform::square: {
      float phase = prm.maskPhase;
      for (int idx = 0; idx < maskSize; ++idx) {
        phase -= std::floor(phase);
        mask[idx] = phase < float(0.5) ? float(1) : float(-1);
        mask[idx] = std::lerp(float(1), mask[idx], prm.maskMix);
        phase += prm.maskFreq;
      }
    } break;
    case MaskWaveform::sawtoothUp: {
      float phase = prm.maskPhase;
      for (int idx = 0; idx < maskSize; ++idx) {
        phase -= std::floor(phase);
        mask[idx] = float(2) * phase - float(1);
        mask[idx] = std::lerp(float(1), mask[idx], prm.maskMix);
        phase += prm.maskFreq;
      }
    } break;
    case MaskWaveform::sawtoothDown: {
      float phase = prm.maskPhase;
      for (int idx = 0; idx < maskSize; ++idx) {
        phase -= std::floor(phase);
        mask[idx] = float(1) - float(2) * phase;
        mask[idx] = std::lerp(float(1), mask[idx], prm.maskMix);
        phase += prm.maskFreq;
      }
    } break;
    case MaskWaveform::noise: {
      std::minstd_rand rng{unsigned(2048 * prm.maskFreq)};
      std::uniform_real_distribution<float> dist{
        float(1) + float(-2) * prm.maskMix, float(1)};

      int startIndex = int((prm.maskPhase - std::floor(prm.maskPhase)) * maskSize);
      for (int idx = startIndex; idx < maskSize; ++idx) mask[idx] = dist(rng);
      for (int idx = 0; idx < startIndex; ++idx) mask[idx] = dist(rng);
    } break;
  }
}

} // namespace SomeDSP
