// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include <algorithm>
#include <cmath>

namespace SomeDSP {

namespace SVFTool {

constexpr double minCutoff = 0.00001;
constexpr double nyquist = 0.49998;

template<typename T> inline T freqToG(T normalizedFreq)
{
  return float(std::tan(std::clamp(double(normalizedFreq), minCutoff, nyquist) * pi));
}

template<typename T> inline T qToK(T Q) { return T(1) / Q; }

} // namespace SVFTool

template<typename Sample> class SerialSVF {
private:
  Sample ic1eq = 0;
  Sample ic2eq = 0;

  EMAFilter<Sample> g;
  EMAFilter<Sample> k;
  EMAFilter<Sample> gSmoother;
  EMAFilter<Sample> kSmoother;

  Sample smootherKp = Sample(1);

public:
  void noteOn(Sample normalizedFreq, Sample Q)
  {
    g.reset(SVFTool::freqToG(normalizedFreq));
    k.reset(SVFTool::qToK(Q));
  }

  void setSmootherSecond(Sample sampleRate, Sample second)
  {
    smootherKp = Sample(EMAFilter<double>::secondToP(sampleRate, second));
  }

  void reset(Sample gValue, Sample kValue)
  {
    g.reset(gValue);
    k.reset(kValue);
    gSmoother.reset(gValue);
    kSmoother.reset(kValue);
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample lowpass(Sample input, Sample gTarget, Sample kTarget, Sample decayKp)
  {
    auto gp = gSmoother.processKp(g.processKp(gTarget, decayKp), smootherKp);
    auto kp = kSmoother.processKp(k.processKp(kTarget, decayKp), smootherKp);

    auto v0 = input;
    auto v1 = (ic1eq + gp * (v0 - ic2eq)) / (Sample(1) + gp * (gp + kp));
    auto v2 = ic2eq + gp * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v2;
  }
};

template<typename Sample> struct OnePoleHighpass {
  Sample g = 0;
  Sample s = 0;

  void reset() { s = 0; }

  void setCutoff(Sample normalizedFreq) { g = Sample(pi) * normalizedFreq; }

  Sample process(Sample x0)
  {
    auto y = (x0 - s) / (Sample(1) + g);
    s += 2 * y * g;
    return y;
  }
};

template<typename Sample> class NoteGate {
private:
  Sample signal = 0;
  DoubleEMAFilter<Sample> filter;

public:
  bool isResting() { return filter.v2 < std::numeric_limits<Sample>::epsilon(); }

  void rest()
  {
    signal = Sample(0);
    filter.reset(Sample(0));
  }

  void trigger()
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
