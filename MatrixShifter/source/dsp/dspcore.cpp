// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

inline std::array<float, 2> calcPhaseOffset(float offset)
{
  if (offset < 0) return {-offset, 0};
  return {0, offset};
}

float DSPCore::getTempoSyncInterval()
{
  // Multiplying with 4 because a beat is 1/4.
  auto lfoRate = param.value[ParameterID::lfoRate]->getFloat();
  if (lfoRate >= Scales::lfoRate.getMax()) return 0.0f;
  return 4.0f * lfoRate * (param.value[ParameterID::lfoSyncUpper]->getFloat() + 1)
    / (param.value[ParameterID::lfoSyncLower]->getFloat() + 1);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (auto &shf : shifter) shf.setup(this->sampleRate, maxShiftDelaySeconds);

  syncer.reset(float(sampleRate), 120.0f, 1.0f);
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  interpGain.METHOD(param.value[ID::gain]->getFloat());                                  \
                                                                                         \
  auto shiftMix = param.value[ID::shiftMix]->getFloat();                                 \
  auto bypassMix = 1 - shiftMix;                                                         \
  interpShiftFeedbackGain.METHOD(param.value[ID::shiftFeedbackGain]->getFloat());        \
  interpShiftFeedbackCutoff.METHOD(                                                      \
    freqToNote(param.value[ID::shiftFeedbackCutoff]->getFloat()));                       \
  interpSectionGain.METHOD(param.value[ID::invertEachSection]->getInt() ? -1.0f : 1.0f); \
                                                                                         \
  interpLfoLrPhaseOffset.METHOD(param.value[ID::lfoLrPhaseOffset]->getFloat());          \
  interpLfoToDelay.METHOD(param.value[ID::lfoToDelay]->getFloat());                      \
  interpLfoSkew.METHOD(param.value[ID::lfoSkew]->getFloat());                            \
  interpLfoToPitchShift.METHOD(param.value[ID::lfoToPitchShift]->getFloat());            \
  interpLfoToFeedbackCutoff.METHOD(param.value[ID::lfoToFeedbackCutoff]->getFloat());    \
                                                                                         \
  auto shiftMul = param.value[ID::shiftSemiMultiplier]->getFloat();                      \
  for (size_t x = 0; x < nSerial; ++x) {                                                 \
    interpShiftDelay[x].METHOD(param.value[ID::shiftDelay0 + x]->getFloat());            \
    interpShiftGain[x].METHOD(shiftMix *param.value[ID::shiftGain0 + x]->getFloat());    \
    for (size_t y = 0; y < nParallel; ++y) {                                             \
      interpShiftHz[x][y].METHOD(                                                        \
        shiftMul *param.value[ID::shiftSemi0 + nParallel * x + y]->getFloat());          \
    }                                                                                    \
  }                                                                                      \
  interpShiftGain.back().METHOD(bypassMix);                                              \
                                                                                         \
  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

void DSPCore::reset()
{
  using ID = ParameterID::ID;

  startup();
  for (auto &shf : shifter) shf.reset();

  lfoOut.fill(0);
  lfoDelay.fill(0);
  feedbackCutoffHz.fill(0);
  lfoHz.fill(0);

  ASSIGN_PARAMETER(reset);
}

void DSPCore::startup() { syncer.reset(sampleRate, tempo, getTempoSyncInterval()); }

void DSPCore::setParameters()
{
  using ID = ParameterID::ID;

  ASSIGN_PARAMETER(push);
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  // When tempo-sync is off, use 120 BPM.
  bool isTempoSyncing = param.value[ParameterID::lfoTempoSync]->getInt();
  syncer.prepare(
    sampleRate, isTempoSyncing ? tempo : 120.0f, getTempoSyncInterval(), beatsElapsed,
    !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    const auto phaseOffset = calcPhaseOffset(interpLfoLrPhaseOffset.process());

    auto cutoff = interpShiftFeedbackCutoff.process();
    auto lfoSkew = interpLfoSkew.process();
    auto lfoDelayAmt = interpLfoToDelay.process();
    auto lfoCutoffAmt = interpLfoToFeedbackCutoff.process();
    auto lfoPhase = syncer.process();
    for (size_t j = 0; j < lfo.size(); ++j) {
      lfoOut[j] = lfo[j].process(lfoPhase, phaseOffset[j], lfoSkew);
      lfoDelay[j] = 1.0f + lfoDelayAmt * (lfoOut[j] - 1.0f);

      if (lfoCutoffAmt >= 0.0) {
        feedbackCutoffHz[j] = noteToFreq(
          cutoff + lfoCutoffAmt * lfoOut[j] * (maxFeedbackCutoffNote - cutoff));
      } else {
        feedbackCutoffHz[j] = noteToFreq(
          cutoff - lfoCutoffAmt * lfoOut[j] * (minFeedbackCutoffNote - cutoff));
      }
    }

    const auto lfoToPitchShift = interpLfoToPitchShift.process();
    if (lfoToPitchShift >= 0.0) {
      lfoHz[0] = 1.0f + lfoToPitchShift * (lfoOut[0] - 1.0f);
      lfoHz[1] = 1.0f + lfoToPitchShift * (lfoOut[1] - 1.0f);
    } else {
      lfoHz[0] = 1.0f - lfoToPitchShift * (lfoOut[1] - 1.0f);
      lfoHz[1] = 1.0f - lfoToPitchShift * (lfoOut[0] - 1.0f);
    }

    for (size_t x = 0; x < nSerial; ++x) {
      auto delay = interpShiftDelay[x].process();
      shifter[0].seconds[x] = lfoDelay[0] * delay;
      shifter[1].seconds[x] = lfoDelay[1] * delay;

      auto gain = interpShiftGain[x].process();
      shifter[0].gain[x] = gain;
      shifter[1].gain[x] = gain;

      for (size_t y = 0; y < nParallel; ++y) {
        auto hz = interpShiftHz[x][y].process();
        shifter[0].hz[x][y] = lfoHz[0] * hz;
        shifter[1].hz[x][y] = lfoHz[1] * hz;
      }
    }
    auto bypassGain = interpShiftGain.back().process();
    shifter[0].bypassGain = bypassGain;
    shifter[1].bypassGain = bypassGain;

    const auto gain = interpGain.process();
    const auto fbGain = interpShiftFeedbackGain.process();
    const auto sectionGain = interpSectionGain.process();
    out0[i] = gain
      * shifter[0].process(
        sampleRate, in0[i], phaseOffset[0], fbGain, feedbackCutoffHz[0], sectionGain);
    out1[i] = gain
      * shifter[1].process(
        sampleRate, in1[i], phaseOffset[1], fbGain, feedbackCutoffHz[1], sectionGain);
  }
}
