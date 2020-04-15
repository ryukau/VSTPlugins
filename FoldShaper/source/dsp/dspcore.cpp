// (c) 2020 Takamitsu Endo
//
// This file is part of FoldShaper.
//
// FoldShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FoldShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FoldShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include "../../../lib/vcl/vectormath_exp.h"

#include <algorithm>
#include <numeric>

#if INSTRSET >= 10
#define DSPCORE_NAME DSPCore_AVX512
#elif INSTRSET >= 8
#define DSPCORE_NAME DSPCore_AVX2
#elif INSTRSET >= 7
#define DSPCORE_NAME DSPCore_AVX
#else
#error Unsupported instruction set
#endif

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &shpr : shaper) shpr.reset();
  startup();
}

void DSPCORE_NAME::startup() {}

void DSPCORE_NAME::setParameters(float tempo)
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpInputGain.push(param.value[ID::inputGain]->getFloat());
  interpOutputGain.push(param.value[ID::outputGain]->getFloat());
  interpMul.push(param.value[ID::mul]->getFloat() * param.value[ID::moreMul]->getFloat());

  oversample = param.value[ID::oversample]->getInt();
  for (auto &shpr : shaper) shpr.hardclip = param.value[ID::hardclip]->getInt();
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> frame;
  for (uint32_t i = 0; i < length; ++i) {
    SmootherCommon<float>::setBufferIndex(i);

    auto inGain = interpInputGain.process();
    auto outGain = interpOutputGain.process();
    auto mul = interpMul.process();

    frame[0] = inGain * in0[i];
    frame[1] = inGain * in1[i];

    shaper[0].multiply = mul;
    shaper[1].multiply = mul;

    if (oversample) {
      frame[0] = outGain * shaper[0].process16(frame[0]);
      frame[1] = outGain * shaper[1].process16(frame[1]);
    } else {
      frame[0] = outGain * shaper[0].process(frame[0]);
      frame[1] = outGain * shaper[1].process(frame[1]);
    }

    out0[i] = std::clamp(frame[0], -128.0f, 128.0f);
    out1[i] = std::clamp(frame[1], -128.0f, 128.0f);
  }
}
