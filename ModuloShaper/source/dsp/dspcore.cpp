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
  using ID = ParameterID::ID;                                                            \
  auto &pv = param.value;                                                                \
                                                                                         \
  SmootherCommon<float>::setTime(pv[ID::smoothness]->getFloat());                        \
                                                                                         \
  interpInputGain.METHOD(pv[ID::inputGain]->getFloat());                                 \
  interpClipGain.METHOD(pv[ID::clipGain]->getFloat());                                   \
  interpOutputGain.METHOD(pv[ID::outputGain]->getFloat());                               \
  interpAdd.METHOD(pv[ID::add]->getFloat() * pv[ID::moreAdd]->getFloat());               \
  interpMul.METHOD(pv[ID::mul]->getFloat() * pv[ID::moreMul]->getFloat());               \
  interpCutoff.METHOD(pv[ID::lowpassCutoff]->getFloat());                                \
                                                                                         \
  shaperType = pv[ID::type]->getInt();                                                   \
  activateLowpass = pv[ID::lowpass]->getInt();                                           \
                                                                                         \
  bool hardclip = pv[ID::hardclip]->getInt();                                            \
  for (auto &shaper : shaperNaive) shaper.hardclip = hardclip;                           \
  for (auto &shaper : shaperBlep) shaper.hardclip = hardclip;                            \
                                                                                         \
  activateLimiter = pv[ID::limiter]->getInt();

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  for (auto &shaper : shaperNaive) shaper.reset();
  for (auto &shaper : shaperBlep) shaper.reset();
  for (auto &lp : lowpass) lp.reset();
  for (auto &lm : limiter) lm.reset();
  startup();
}

void DSPCore::startup() {}

size_t DSPCore::getLatency()
{
  auto latency = activateLimiter ? limiter[0].latency() : 0;
  if (shaperType == 1)
    return shaperNaive[0].latency() + latency;
  else if (shaperType == 2) // 4 point PolyBLEP residual.
    return 4 + latency;
  else if (shaperType == 3) // 8 point PolyBLEP residual.
    return 8 + latency;
  return latency;
}

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);

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
    auto clipGain = interpClipGain.process();
    auto outGain = interpOutputGain.process();
    auto add = interpAdd.process();
    auto mul = interpMul.process();
    auto cutoff = interpCutoff.process();

    if (mul > 1.0f) clipGain /= mul;

    if (activateLowpass) {
      lowpass[0].setCutoff(sampleRate, cutoff);
      lowpass[1].setCutoff(sampleRate, cutoff);
      frame[0] = lowpass[0].process(in0[i]);
      frame[1] = lowpass[1].process(in1[i]);
    } else {
      frame[0] = in0[i];
      frame[1] = in1[i];
    }

    switch (shaperType) {
      case 0: // Naive.
        shaperNaive[0].add = add;
        shaperNaive[1].add = add;
        shaperNaive[0].mul = mul;
        shaperNaive[1].mul = mul;

        frame[0] = clipGain * shaperNaive[0].process(inGain * frame[0]);
        frame[1] = clipGain * shaperNaive[1].process(inGain * frame[1]);
        break;

      case 1: // Naive 16x oversampling.
        shaperNaive[0].add = add;
        shaperNaive[1].add = add;
        shaperNaive[0].mul = mul;
        shaperNaive[1].mul = mul;

        frame[0] = clipGain * shaperNaive[0].process16x(inGain * frame[0]);
        frame[1] = clipGain * shaperNaive[1].process16x(inGain * frame[1]);
        break;

      case 2: // 4 point PolyBLEP residual.
        shaperBlep[0].add = add;
        shaperBlep[1].add = add;
        shaperBlep[0].mul = mul;
        shaperBlep[1].mul = mul;

        frame[0] = clipGain * float(shaperBlep[0].process4(inGain * frame[0]));
        frame[1] = clipGain * float(shaperBlep[1].process4(inGain * frame[1]));
        break;

      case 3: // 8 point PolyBLEP residual.
        shaperBlep[0].add = add;
        shaperBlep[1].add = add;
        shaperBlep[0].mul = mul;
        shaperBlep[1].mul = mul;

        frame[0] = clipGain * float(shaperBlep[0].process8(inGain * frame[0]));
        frame[1] = clipGain * float(shaperBlep[1].process8(inGain * frame[1]));
        break;
    }

    frame[0] *= outGain;
    frame[1] *= outGain;

    if (activateLimiter) {
      frame[0] = limiter[0].process(frame[0]);
      frame[1] = limiter[1].process(frame[1]);
    }

    out0[i] = frame[0];
    out1[i] = frame[1];
  }
}
