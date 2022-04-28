// (c) 2020-2022 Takamitsu Endo
//
// This file is part of SoftClipper.
//
// SoftClipper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoftClipper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoftClipper.  If not, see <https://www.gnu.org/licenses/>.

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

inline float maxAbs(const size_t length, const float *buffer)
{
  float max = 0.0f;
  for (size_t i = 0; i < length; ++i) max = std::max(max, std::fabs(buffer[i]));
  return max;
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  maxGain = 0.0f;                                                                        \
                                                                                         \
  using ID = ParameterID::ID;                                                            \
                                                                                         \
  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());               \
                                                                                         \
  interpInputGain.METHOD(param.value[ID::inputGain]->getFloat());                        \
  interpOutputGain.METHOD(param.value[ID::outputGain]->getFloat());                      \
  interpClip.METHOD(param.value[ID::clip]->getFloat());                                  \
  interpOrder.METHOD(                                                                    \
    float(param.value[ID::orderInteger]->getInt())                                       \
    + param.value[ID::orderFraction]->getFloat());                                       \
  interpRatio.METHOD(param.value[ID::ratio]->getFloat());                                \
  interpSlope.METHOD(param.value[ID::slope]->getFloat());

void DSPCORE_NAME::reset()
{
  for (auto &shpr : shaper) shpr.reset();
  startup();
}

void DSPCORE_NAME::startup() { ASSIGN_PARAMETER(reset); }

size_t DSPCORE_NAME::getLatency() { return oversample ? shaper[0].latency() : 0; }

void DSPCORE_NAME::setParameters()
{
  ASSIGN_PARAMETER(push);
  oversample = param.value[ID::oversample]->getInt();
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(float(length));

  param.value[ParameterID::guiInputGain]->setFromFloat(
    std::max(maxAbs(length, in0), maxAbs(length, in1)));

  for (uint32_t i = 0; i < length; ++i) {
    auto inGain = interpInputGain.process();
    auto outGain = interpOutputGain.process();
    auto clip = interpClip.process();
    auto order = interpOrder.process();
    auto ratio = interpRatio.process();
    auto slope = interpSlope.process();

    shaper[0].set(clip, order, ratio, slope);
    shaper[1].set(clip, order, ratio, slope);

    if (oversample) {
      out0[i] = outGain * shaper[0].process16(inGain * in0[i]);
      out1[i] = outGain * shaper[1].process16(inGain * in1[i]);
    } else {
      out0[i] = outGain * shaper[0].process(inGain * in0[i]);
      out1[i] = outGain * shaper[1].process(inGain * in1[i]);
    }
  }
}
