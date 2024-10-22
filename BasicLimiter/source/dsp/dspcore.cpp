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

  for (auto &lm : limiter)
    lm.resize(size_t(UpSamplerFir::upfold * maxAttackSeconds * this->sampleRate) + 1);

  reset();
  startup();
}

size_t DSPCore::getLatency()
{
  bool truepeak = param.value[ParameterID::truePeak]->getInt();
  auto latency = limiter[0].latency(truepeak ? UpSamplerFir::upfold : 1);
  if (truepeak) {
    latency += 1 + UpSamplerFir::intDelay + DownSamplerFir::intDelay
      + HighEliminationFir<float>::delay;
  }
  return latency;
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  interpStereoLink.METHOD(pv[ID::limiterStereoLink]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  pv[ID::overshoot]->setFromFloat(1.0);

  for (auto &lm : limiter) lm.reset(pv[ID::limiterThreshold]->getFloat());
  for (auto &he : highEliminator) he.reset();
  for (auto &us : upSampler) us.reset();
  for (auto &ds : downSampler) ds.reset();
  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);

  auto &&rate = param.value[ParameterID::truePeak]->getInt()
    ? UpSamplerFir::upfold * sampleRate
    : sampleRate;
  for (auto &lm : limiter) {
    lm.prepare(
      rate, pv[ID::limiterAttack]->getFloat(), pv[ID::limiterSustain]->getFloat(),
      pv[ID::limiterRelease]->getFloat(), pv[ID::limiterThreshold]->getFloat(),
      pv[ID::limiterGate]->getFloat());
  }
}

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

std::array<float, 2> DSPCore::processStereoLink(float in0, float in1)
{
  auto &&stereoLink = interpStereoLink.process();
  auto &&abs0 = std::fabs(in0);
  auto &&abs1 = std::fabs(in1);
  auto &&absMax = std::max(abs0, abs1);
  return {lerp(abs0, absMax, stereoLink), lerp(abs1, absMax, stereoLink)};
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  if (param.value[ParameterID::truePeak]->getInt()) {
    constexpr size_t upfold = UpSamplerFir::upfold;
    for (size_t i = 0; i < length; ++i) {
      auto sig0 = highEliminator[0].process(in0[i]);
      auto sig1 = highEliminator[1].process(in1[i]);

      upSampler[0].process(sig0);
      upSampler[1].process(sig1);

      std::array<std::array<float, upfold>, 2> expanded;
      for (size_t j = 0; j < upfold; ++j) {
        auto &&tp0 = upSampler[0].output[j];
        auto &&tp1 = upSampler[1].output[j];

        auto &&inAbs = processStereoLink(tp0, tp1);

        expanded[0][j] = limiter[0].process(tp0, inAbs[0]);
        expanded[1][j] = limiter[1].process(tp1, inAbs[1]);
      }

      out0[i] = downSampler[0].process(expanded[0]);
      out1[i] = downSampler[1].process(expanded[1]);
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      auto &&inAbs = processStereoLink(in0[i], in1[i]);
      out0[i] = limiter[0].process(in0[i], inAbs[0]);
      out1[i] = limiter[1].process(in1[i], inAbs[1]);
    }
  }

  auto maxOut = std::max(maxAbs(length, out0), maxAbs(length, out1));
  auto &paramClippingPeak = param.value[ParameterID::overshoot];
  auto &&previousPeak = paramClippingPeak->getFloat();
  if (maxOut > previousPeak) paramClippingPeak->setFromFloat(maxOut);
}
