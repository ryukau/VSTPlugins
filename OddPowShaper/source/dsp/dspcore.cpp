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

void DSPCore::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  SmootherCommon<float>::setTime(pv[ID::smoothness]->getFloat());

  interpDrive.reset(pv[ID::drive]->getFloat() * pv[ID::boost]->getFloat());
  interpOutputGain.reset(pv[ID::outputGain]->getFloat());

  for (auto &sp : shaper) sp.reset();
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
  using ID = ParameterID::ID;
  auto &pv = param.value;

  SmootherCommon<float>::setTime(pv[ID::smoothness]->getFloat());

  interpDrive.push(pv[ID::drive]->getFloat() * pv[ID::boost]->getFloat());
  interpOutputGain.push(pv[ID::outputGain]->getFloat());

  oversample = pv[ID::oversample]->getInt();
  for (auto &shpr : shaper) {
    shpr.flip = pv[ID::flip]->getInt();
    shpr.inverse = pv[ID::inverse]->getInt();
    shpr.order = pv[ID::order]->getInt();
  }

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
  for (uint32_t i = 0; i < length; ++i) {
    auto drive = interpDrive.process();
    auto outGain = interpOutputGain.process();

    frame[0] = in0[i];
    frame[1] = in1[i];

    shaper[0].drive = drive;
    shaper[1].drive = drive;

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
