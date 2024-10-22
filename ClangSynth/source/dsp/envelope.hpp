// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../../../common/dsp/solver.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace SomeDSP {

// Non-recursive form of DoubleEMAEnvelope output. Negated because `minimizeScalarBrent`
// finds minimum.
template<typename T> T doubleEmaEnvelopeD0Negative(T n, T k_A, T k_D)
{
  auto A = std::pow(T(1) - k_A, n + T(1)) * (k_A * n + k_A + T(1));
  auto D = std::pow(T(1) - k_D, n + T(1)) * (k_D * n + k_D + T(1));
  return (A - T(1)) * D;
}

template<typename T> T samplesToKp(T timeInSamples)
{
  if (timeInSamples < std::numeric_limits<T>::epsilon()) return T(1);
  auto y = T(1) - std::cos(T(twopi) / timeInSamples);
  return -y + std::sqrt(y * (y + T(2)));
}

template<typename Sample> class DoubleEmaADEnvelope {
private:
  Sample v1_A = 0;
  Sample v2_A = 0;

  Sample v1_D = 0;
  Sample v2_D = 0;

  Sample k_A = Sample(1);
  Sample k_D = Sample(1);

  Sample gain = Sample(1); // Gain to normalize peak to 1.

  size_t peakPoint = 0;
  size_t attackCounter = 0; // Used for voice stealing.

public:
  void reset()
  {
    v1_A = 0;
    v2_A = 0;
    v1_D = Sample(1);
    v2_D = Sample(1);
    attackCounter = 0;
  }

  bool isAttacking() { return attackCounter < peakPoint; }

  void noteOn(Sample targetAmplitude, Sample attackTimeSamples, Sample decayTimeSamples)
  {
    // Using `double` for minimization. `float` is inaccurate over 10^4 samples.
    auto kA = samplesToKp<double>(attackTimeSamples);
    auto kD = samplesToKp<double>(decayTimeSamples);

    if (kA == 1.0 || kD == 1.0) {
      gain = Sample(1);
      k_A = Sample(kA);
      k_D = Sample(kD);
      peakPoint = size_t(attackTimeSamples);
    } else {
      auto result = minimizeScalarBrent<double>(
        [&](double n) { return doubleEmaEnvelopeD0Negative<double>(n, kA, kD); });

      peakPoint = size_t(result.first) + 1;
      auto peak = -result.second;
      gain
        = peak < std::numeric_limits<double>::epsilon() ? Sample(1) : Sample(1.0 / peak);
      k_A = Sample(kA);
      k_D = Sample(kD);
    }
    gain *= targetAmplitude;

    reset();
  }

  Sample process()
  {
    if (attackCounter < peakPoint) ++attackCounter;

    v1_A += k_A * (Sample(1) - v1_A);
    v2_A += k_A * (v1_A - v2_A);

    v1_D += k_D * (Sample(0) - v1_D);
    v2_D += k_D * (v1_D - v2_D);

    return gain * v2_A * v2_D;
  }
};

template<typename Sample> class NoteGate {
private:
  Sample signal = 0;
  DoubleEMAFilter<Sample> filter;

public:
  void reset()
  {
    signal = Sample(1);
    filter.reset(Sample(1));
  }

  void prepare(Sample sampleRate, Sample seconds)
  {
    if (seconds < std::numeric_limits<Sample>::epsilon())
      filter.kp = Sample(1);
    else
      filter.setCutoff(sampleRate, Sample(1) / seconds);
  }

  void release() { signal = Sample(0); }

  Sample process() { return filter.process(signal); }
};

} // namespace SomeDSP
