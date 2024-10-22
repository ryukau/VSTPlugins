// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/basiclimiter.hpp"
#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <limits>

namespace SomeDSP {

template<typename Sample> class SVF {
private:
  Sample ic1eq = 0;
  Sample ic2eq = 0;

  static constexpr Sample k
    = Sample(1) / Sample(halfSqrt2 - std::numeric_limits<Sample>::epsilon());

public:
  void reset()
  {
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample lowpass(Sample v0, Sample g)
  {
    auto v1 = (ic1eq + g * (v0 - ic2eq)) / (Sample(1) + g * (g + k));
    auto v2 = ic2eq + g * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v2;
  }

  Sample highpass(Sample v0, Sample g)
  {
    auto v1 = (ic1eq + g * (v0 - ic2eq)) / (Sample(1) + g * (g + k));
    auto v2 = ic2eq + g * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v0 - k * v1 - v2;
  }
};

/**
This implementation takes raw value instead of absolute value of input. It's a
specialization specific to AccumulativeRingMod, to skip a call to `std::abs()`.
*/
template<typename Sample> class EnvelopeFollower {
private:
  Sample value = 0;
  Sample release = Sample(1);

public:
  void reset(Sample resetValue = 0)
  {
    value = resetValue;
    release = Sample(1);
  }

  void prepare(Sample releaseSamples)
  {
    constexpr Sample eps = Sample(std::numeric_limits<float>::epsilon());
    release = std::pow(eps, Sample(1) / releaseSamples);
  }

  Sample process(Sample input)
  {
    // Gate at 0.0625 ~= -24 dB.
    if (input > Sample(0.0625) && input > value) value = input;
    return value *= release;
  }
};

/**
If `threshold <= 0`, gate will always open.
*/
template<typename Sample> class EasyGate {
private:
  PeakHold<Sample> peakhold;
  DoubleEMAFilter<Sample> smoother;
  Sample threshold = 0;

public:
  void setup(Sample sampleRate, Sample holdSeconds)
  {
    auto holdSamples = size_t(sampleRate * holdSeconds);
    peakhold.resize(holdSamples);
    peakhold.setFrames(holdSamples);

    smoother.setCutoff(sampleRate, Sample(200));
  }

  void reset()
  {
    peakhold.reset();
    smoother.reset(Sample(0));
  }

  void prepare(Sample sampleRate, Sample gateThresholdAmp)
  {
    threshold = gateThresholdAmp;
    if (threshold <= 0) smoother.reset(Sample(0));

    smoother.setCutoff(sampleRate, Sample(1000));
  }

  Sample process(Sample inAbs)
  {
    if (threshold <= 0) return Sample(1);
    return threshold <= peakhold.process(inAbs) ? smoother.process(Sample(1))
                                                : smoother.process(Sample(0));
  }
};

} // namespace SomeDSP
