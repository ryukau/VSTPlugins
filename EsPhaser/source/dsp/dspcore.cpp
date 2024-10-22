// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include "../../../lib/vcl/vectormath_exp.h"

#include <algorithm>
#include <numeric>

#define DSPCORE_NAME DSPCore_FixedInstruction

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  interpPhase.setRange(float(twopi));

  phaser[0].setup(this->sampleRate);
  phaser[1].setup(this->sampleRate);
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());               \
                                                                                         \
  interpMix.METHOD(param.value[ID::mix]->getFloat());                                    \
  interpFrequency.METHOD(                                                                \
    param.value[ID::frequency]->getFloat() * float(twopi) / sampleRate);                 \
  interpFreqSpread.METHOD(param.value[ID::freqSpread]->getFloat());                      \
  interpFeedback.METHOD(param.value[ID::feedback]->getFloat());                          \
                                                                                         \
  const float phaserRange = param.value[ID::range]->getFloat();                          \
  interpRange.METHOD(phaserRange);                                                       \
  interpMin.METHOD(                                                                      \
    Thiran2Phaser::getLfoMin(phaserRange, param.value[ID::min]->getFloat()));            \
                                                                                         \
  interpPhase.METHOD(param.value[ID::phase]->getFloat());                                \
  interpStereoOffset.METHOD(param.value[ID::stereoOffset]->getFloat());                  \
  interpCascadeOffset.METHOD(param.value[ID::cascadeOffset]->getFloat());

void DSPCORE_NAME::reset()
{
  using ID = ParameterID::ID;

  ASSIGN_PARAMETER(reset);

  auto phaserStage = param.value[ID::stage]->getInt();
  for (auto &ph : phaser) ph.reset(phaserStage);
  startup();
}

void DSPCORE_NAME::startup()
{
  for (size_t i = 0; i < phaser.size(); ++i) {
    phaser[i].phase = float(i) / phaser.size();
  }
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  ASSIGN_PARAMETER(push);

  auto phaserStage = param.value[ID::stage]->getInt();
  phaser[0].setStage(phaserStage);
  phaser[1].setStage(phaserStage);
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  auto len_f = float(length);
  SmootherCommon<float>::setBufferSize(float(len_f));
  phaser[0].interpStage.setBufferSize(float(len_f));
  phaser[1].interpStage.setBufferSize(float(len_f));

  for (size_t i = 0; i < length; ++i) {
    const auto freq = interpFrequency.process();
    const auto spread = interpFreqSpread.process();
    const auto feedback = interpFeedback.process();
    const auto range = interpRange.process();
    const auto min = interpMin.process();
    const auto phase = interpPhase.process();
    const auto stereo = interpStereoOffset.process();
    const auto cascade = interpCascadeOffset.process();

    const auto phaser0
      = phaser[0].process(in0[i], spread, cascade, phase, freq, feedback, range, min);
    const auto phaser1 = phaser[1].process(
      in0[i], spread, cascade, phase + stereo, freq, feedback, range, min);

    const auto mix = interpMix.process();
    out0[i] = in0[i] + mix * (phaser0 - in0[i]);
    out1[i] = in1[i] + mix * (phaser1 - in1[i]);
  }
}
