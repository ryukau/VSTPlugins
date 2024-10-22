// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "filter.hpp"

#include <cmath>
#include <limits>
#include <numbers>
#include <random>

namespace SomeDSP {

// BLIT: band limited impulse train.
template<typename Sample> class BlitOscillator {
private:
  Sample phase = 0;

public:
  void reset() { phase = 0; }

  Sample process(Sample freqNormalized)
  {
    constexpr Sample pi = std::numbers::pi_v<Sample>;

    phase += freqNormalized;
    phase -= std::floor(phase);
    auto denom = std::sin(pi * phase);
    if (std::abs(denom) <= std::numeric_limits<Sample>::epsilon()) return Sample(1);
    auto M = Sample(2) * std::floor(Sample(0.5) / freqNormalized) + Sample(1);
    return freqNormalized * std::sin(pi * M * phase) / denom;
  }
};

template<typename Sample> class EnvelopedNoise {
private:
  Sample gain = Sample(1);
  Sample phase = 0;
  ExpSmoother<Sample> decay;
  LP1<Sample> lowpass;
  std::minstd_rand rng{0};

public:
  void reset(Sample decaySamples, Sample lowpassCutoff)
  {
    gain = Sample(1);
    phase = 0;

    decay.reset(
      std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples));
    lowpass.reset(lowpassCutoff);

    rng.seed(0);
  }

  // `lowpassCutoff` is normalized frequency in [0, 0.5).
  void push(Sample decaySamples, Sample lowpassCutoff)
  {
    decay.push(
      std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples));
    lowpass.push(lowpassCutoff);
  }

  Sample process(Sample freqNormalized)
  {
    phase += freqNormalized;
    if (phase >= Sample(1)) {
      phase -= std::floor(phase);
      gain = Sample(1);
    }
    gain *= decay.process();

    std::uniform_real_distribution<Sample> dist{Sample(-1), Sample(1)};
    return lowpass.process(gain) * dist(rng);
  }
};

} // namespace SomeDSP
