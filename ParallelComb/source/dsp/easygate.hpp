// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/basiclimiter.hpp"

#include <algorithm>
#include <array>

namespace SomeDSP {

template<typename Sample> class DoubleEMAFilterKp {
private:
  Sample v1 = 0;
  Sample v2 = 0;

public:
  void reset(Sample value = 0)
  {
    v1 = value;
    v2 = value;
  }

  Sample process(Sample input, Sample kp)
  {
    auto &&v0 = input;
    v1 += kp * (v0 - v1);
    v2 += kp * (v1 - v2);
    return v2;
  }
};

/**
If `threshold <= 0`, gate will always open.
*/
template<typename Sample> class EasyGate {
private:
  PeakHold<Sample> peakhold;
  DoubleEMAFilterKp<Sample> smoother;
  Sample threshold = 0;
  Sample openKp = Sample(1); // Smoother coefficient for opening gate.

public:
  void setup(Sample sampleRate, Sample holdSeconds)
  {
    auto holdSamples = size_t(sampleRate * holdSeconds);
    peakhold.resize(holdSamples);
    peakhold.setFrames(holdSamples);

    openKp = EMAFilter<double>::cutoffToP(sampleRate, Sample(200));
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

    openKp = EMAFilter<double>::cutoffToP(sampleRate, Sample(1000));
  }

  Sample process(Sample inAbs, Sample closeKp)
  {
    if (threshold <= 0) return Sample(1);
    return threshold <= peakhold.process(inAbs) ? smoother.process(Sample(1), openKp)
                                                : smoother.process(Sample(0), closeKp);
  }
};

} // namespace SomeDSP
