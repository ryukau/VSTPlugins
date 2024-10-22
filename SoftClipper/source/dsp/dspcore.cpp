// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

void DSPCore::reset()
{
  for (auto &shpr : shaper) shpr.reset();
  startup();
}

void DSPCore::startup() { ASSIGN_PARAMETER(reset); }

size_t DSPCore::getLatency() { return oversample ? shaper[0].latency() : 0; }

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);
  oversample = param.value[ID::oversample]->getInt();
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

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
