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

  Sample gain = 0; // Gain to normalize peak to 1.

  inline void initState()
  {
    v1_A = 0;
    v2_A = 0;
    v1_D = Sample(1);
    v2_D = Sample(1);
  }

public:
  void reset()
  {
    initState();
    gain = 0;
  }

  void noteOn(Sample targetAmplitude, Sample attackTimeSamples, Sample decayTimeSamples)
  {
    // Using `double` for minimization. `float` is inaccurate over 10^4 samples.
    auto kA = samplesToKp<double>(attackTimeSamples);
    auto kD = samplesToKp<double>(decayTimeSamples);

    if (kA == 1.0 || kD == 1.0) {
      gain = Sample(1);
      k_A = Sample(kA);
      k_D = Sample(kD);
    } else {
      auto result = minimizeScalarBrent<double>(
        [&](double n) { return doubleEmaEnvelopeD0Negative<double>(n, kA, kD); });

      auto peak = -result.second;
      gain
        = peak < std::numeric_limits<double>::epsilon() ? Sample(1) : Sample(1.0 / peak);
      k_A = Sample(kA);
      k_D = Sample(kD);
    }
    gain *= targetAmplitude;

    initState();
  }

  Sample process()
  {
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
  void reset(Sample value = 0)
  {
    signal = value;
    filter.reset(value);
  }

  void prepare(Sample sampleRate, Sample seconds)
  {
    if (seconds < std::numeric_limits<Sample>::epsilon())
      filter.kp = Sample(1);
    else
      filter.setCutoff(sampleRate, Sample(1) / seconds);
  }

  bool isTerminated() { return filter.v2 <= std::numeric_limits<Sample>::epsilon(); }
  void changeSustainGain(Sample sustainGain) { signal = sustainGain; }
  void release() { signal = Sample(0); }

  Sample process() { return filter.process(signal); }
};

/**
Use with DoubleEmaADEnvelope.

When note-on comes in, and DoubleEmaADEnvelope has to reset, move the current value of
DoubleEmaADEnvelope to TransitionReleaseSmoother using `prepare()`. `process()` output of
DoubleEmaADEnvelope and TransitionReleaseSmoother are summed together.
*/
template<typename Sample> class TransitionReleaseSmoother {
private:
  Sample v0 = 0;
  Sample decay = 0;

public:
  void reset() { v0 = 0; }

  // decaySamples = sampleRate * seconds.
  void prepare(Sample value, Sample decaySamples)
  {
    v0 += value;
    decay = std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples);
  }

  Sample process() { return v0 *= decay; }
};

template<typename Sample> class AccumulateAM {
private:
  Sample phase = 0;

public:
  void reset() { phase = 0; }

  // `modulationAmount = amount * 48000 / sampleRate`.
  // `mix` is in [0, 1].
  Sample process(Sample input, Sample modulationAmount, Sample mix)
  {
    auto am = std::cos(Sample(twopi) * (phase += modulationAmount * input));
    return input * std::lerp(Sample(1), am, mix);
  }
};

} // namespace SomeDSP
