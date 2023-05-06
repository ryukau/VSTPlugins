// (c) 2020-2022 Takamitsu Endo
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

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>

inline float maxAbs(const size_t length, const float *buffer)
{
  float max = 0.0f;
  for (size_t i = 0; i < length; ++i) max = std::max(max, std::fabs(buffer[i]));
  return max;
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  auto &pv = param.value;                                                                \
                                                                                         \
  SmootherCommon<float>::setTime(pv[ID::smoothness]->getFloat());                        \
                                                                                         \
  interpInputGain.METHOD(pv[ID::inputGain]->getFloat());                                 \
  interpOutputGain.METHOD(pv[ID::outputGain]->getFloat());                               \
  interpMul.METHOD(pv[ID::mul]->getFloat() * pv[ID::moreMul]->getFloat());               \
                                                                                         \
  oversample = pv[ID::oversample]->getInt();                                             \
  for (auto &shpr : shaper) shpr.hardclip = pv[ID::hardclip]->getInt();

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  for (auto &shpr : shaper) shpr.reset();
  for (auto &lm : limiter) lm.reset();
  startup();
}

void DSPCore::startup() {}

size_t DSPCore::getLatency()
{
  auto &&latency = activateLimiter ? limiter[0].latency() : 0;
  latency += oversample ? shaper[0].latency() : 0;
  return latency;
}

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);

  activateLimiter = pv[ID::limiter]->getInt();
  for (auto &lm : limiter) {
    lm.prepare(
      sampleRate, pv[ID::limiterRelease]->getFloat(),
      pv[ID::limiterThreshold]->getFloat());
  }
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  param.value[ParameterID::guiInputGain]->setFromFloat(
    std::max(maxAbs(length, in0), maxAbs(length, in1)));

  std::array<float, 2> frame{};
  for (size_t i = 0; i < length; ++i) {
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

    if (activateLimiter) {
      frame[0] = limiter[0].process(frame[0]);
      frame[1] = limiter[1].process(frame[1]);
    }

    out0[i] = frame[0];
    out1[i] = frame[1];
  }
}
