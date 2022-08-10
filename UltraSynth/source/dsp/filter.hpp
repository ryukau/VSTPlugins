// (c) 2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include <algorithm>
#include <cmath>

namespace SomeDSP {

namespace SVFTool {

template<typename T> inline T freqToG(T normalizedFreq)
{
  constexpr double minCutoff = 0.00001;
  constexpr double nyquist = 0.49998;

  return float(std::tan(std::clamp(double(normalizedFreq), minCutoff, nyquist) * pi));
}

template<typename T> inline T qToK(T Q) { return T(1) / Q; }

} // namespace SVFTool

template<typename Sample, size_t cascade> class SerialSVF {
private:
  std::array<Sample, cascade> ic1eq{};
  std::array<Sample, cascade> ic2eq{};

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

  void reset()
  {
    gSmoother.reset(SVFTool::freqToG(Sample(0.49)));
    kSmoother.reset(Sample(1));
    ic1eq.fill(0);
    ic2eq.fill(0);
  }

  Sample
  lowpass(Sample input, Sample gTarget, Sample kTarget, Sample decayKp, Sample absMix)
  {
    auto gp = gSmoother.processKp(g.processKp(gTarget, decayKp), smootherKp);
    auto kp = kSmoother.processKp(k.processKp(kTarget, decayKp), smootherKp);

    auto v0 = input;
    for (size_t n = 0; n < cascade; ++n) {
      auto v1 = (ic1eq[n] + gp * (v0 - ic2eq[n])) / (Sample(1) + gp * (gp + kp));
      auto v2 = ic2eq[n] + gp * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0 = v2 + absMix * (std::abs(v2) - v2);
    }
    return v0;
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
