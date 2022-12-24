// (c) 2022 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#ifdef USE_VECTORCLASS
  #include "../../../lib/vcl/vectorclass.h"
#endif

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "limiter.hpp"
#include "polyphase.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

using UpSamplerFir = UpSamplerFir8Fold<float>;
using DownSamplerFir = DownSamplerFir8Fold<float>;

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
  std::array<float, 2> processStereoLink(float in0, float in1);

  float sampleRate = 44100.0f;

  ExpSmoother<float> interpStereoLink;

  std::array<Limiter<float>, 2> limiter;
  std::array<NaiveConvolver<float, HighEliminationFir<float>>, 2> highEliminator;
  std::array<FirPolyPhaseUpSampler<float, UpSamplerFir>, 2> upSampler;
  std::array<FirDownSampler<float, DownSamplerFir>, 2> downSampler;
};
