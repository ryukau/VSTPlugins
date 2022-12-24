// (c) 2020-2022 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lightlimiter.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "moduloshaper.hpp"

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

  std::array<ModuloShaper<float>, 2> shaperNaive;
  std::array<ModuloShaperPolyBLEP<double>, 2> shaperBlep;
  std::array<Butter8Lowpass<float>, 2> lowpass;
  std::array<LightLimiter<float, 64>, 2> limiter;

  size_t shaperType = 0; /* 0: naive, 1: oversample, 2: P-BLEP4, 3: P-BLEP8 */
  bool activateLowpass = true;
  bool activateLimiter = true;
  ExpSmoother<float> interpInputGain;
  ExpSmoother<float> interpClipGain;
  ExpSmoother<float> interpOutputGain;
  ExpSmoother<float> interpAdd;
  ExpSmoother<float> interpMul;
  ExpSmoother<float> interpCutoff;
};
