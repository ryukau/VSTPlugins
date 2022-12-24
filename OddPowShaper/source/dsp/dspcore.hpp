// (c) 2020-2022 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lightlimiter.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "oddpowshaper.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  GlobalParameter param;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

private:
  float sampleRate = 44100.0f;
  float maxGain = 0.0f;

  std::array<OddPowShaper<float>, 2> shaper;
  std::array<LightLimiter<float, 64>, 2> limiter;

  bool oversample = true;
  bool activateLimiter = true;
  ExpSmoother<float> interpDrive;
  ExpSmoother<float> interpOutputGain;
};
