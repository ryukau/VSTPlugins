// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

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

    openKp = EMAFilter<double>::cutoffToP(sampleRate, Sample(1000));
  }

  void reset()
  {
    peakhold.reset();
    smoother.reset(Sample(0));
  }

  void prepare(Sample gateThresholdAmp)
  {
    threshold = gateThresholdAmp;
    if (threshold <= 0) smoother.reset(Sample(0));
  }

  Sample process(Sample inAbs, Sample closeKp)
  {
    if (threshold <= 0) return Sample(1);
    return threshold <= peakhold.process(inAbs) ? smoother.process(Sample(1), openKp)
                                                : smoother.process(Sample(0), closeKp);
  }
};

} // namespace SomeDSP
