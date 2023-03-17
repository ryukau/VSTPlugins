// (c) 2022 Takamitsu Endo
//
// This file is part of RingModSpacer.
//
// RingModSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RingModSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RingModSpacer.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/basiclimiter.hpp"
#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  DSPCore() {}

  GlobalParameter param;
  bool isPlaying = false;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length,
    const float *in0,
    const float *in1,
    const float *in2,
    const float *in3,
    float *out0,
    float *out1);

private:
  std::array<double, 2> processFrame(const std::array<double, 4> &frame);

  double sampleRate = 44100;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> sideMix;
  ExpSmoother<double> ringSubtractMix;
  ExpSmoother<double> inputGain;
  ExpSmoother<double> sideGain;

  std::array<IntDelay<double>, 4> delay;
  std::array<BasicLimiter<double>, 4> limiter;
};
