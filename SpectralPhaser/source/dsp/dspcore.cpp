// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

using Sample = DSPCore::Sample;

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = Sample(sampleRate);

  SmootherCommon<Sample>::setSampleRate(sampleRate);

  lfoSyncRate = EMAFilter<double>::secondToP(sampleRate, Sample(0.013));

  reset();
  startup();
}

size_t DSPCore::getLatency() { return spcParam.reportLatency ? spcParam.frmSize : 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  SmootherCommon<Sample>::setTime(pv[ID::parameterSmoothingSecond]->getFloat());         \
                                                                                         \
  spcParam.sideChain = pv[ID::sideChainSwitch]->getInt();                                \
  spcParam.reportLatency = pv[ID::reportLatency]->getInt();                              \
  spcParam.frameSizeLog2 = pv[ID::frameSize]->getInt() + maxFrameSizeStart;              \
  spcParam.frmSize = int(1) << spcParam.frameSizeLog2;                                   \
                                                                                         \
  spcParam.transform = static_cast<TransformType>(pv[ID::transform]->getInt());          \
  if (spcParam.transform != previousTransform) {                                         \
    for (auto &x : spc) x.reset(spcParam.frameSizeLog2);                                 \
  }                                                                                      \
  previousTransform = spcParam.transform;                                                \
                                                                                         \
  spcParam.maskWaveform = static_cast<MaskWaveform>(pv[ID::maskWaveform]->getInt());     \
                                                                                         \
  lfoWaveform = static_cast<LfoWaveform>(pv[ID::lfoWaveform]->getInt());                 \
  lfoWaveMod.METHOD(pv[ID::lfoWaveMod]->getFloat());                                     \
  lfoRate.METHOD(pv[ID::lfoRate]->getFloat() / sampleRate);                              \
  lfoStereoPhaseOffset.METHOD(pv[ID::lfoStereoPhaseOffset]->getFloat());                 \
  lfoInitialPhase.METHOD(pv[ID::lfoInitialPhase]->getFloat());                           \
                                                                                         \
  feedback.METHOD(pv[ID::feedback]->getFloat());                                         \
  const auto spcShift = Sample(0.5) * pv[ID::spectralShift]->getFloat();                 \
  spectralShift.METHOD(spcShift - std::floor(spcShift));                                 \
  octaveDown.METHOD(pv[ID::octaveDown]->getFloat());                                     \
                                                                                         \
  constexpr Sample twopi = Sample(2) * std::numbers::pi_v<Sample>;                       \
  maskMix.METHOD(pv[ID::maskMix]->getFloat());                                           \
  maskPhase.METHOD(pv[ID::maskPhase]->getFloat());                                       \
  maskFreq.METHOD(pv[ID::maskFreq]->getFloat() / spcParam.frmSize);                      \
  maskThreshold.METHOD(pv[ID::maskThreshold]->getFloat());                               \
  maskRotation.METHOD(pv[ID::maskRotation]->getFloat() * twopi);                         \
  lfoToMaskMix.METHOD(pv[ID::lfoToMaskMix]->getFloat());                                 \
  lfoToMaskPhase.METHOD(pv[ID::lfoToMaskPhase]->getFloat());                             \
  lfoToMaskFreq.METHOD(pv[ID::lfoToMaskFreq]->getFloat());                               \
  lfoToMaskThreshold.METHOD(pv[ID::lfoToMaskThreshold]->getFloat());                     \
  lfoToMaskRotation.METHOD(pv[ID::lfoToMaskRotation]->getFloat() * twopi);               \
  lfoToSpectralShift.METHOD(pv[ID::lfoToSpectralShift]->getFloat() * Sample(2));         \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getFloat());                                     \
  dryWetMix.METHOD(pv[ID::dryWetMix]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  lfoTargetFreq = getTempoSyncFrequency();
  lfo.reset(lfoInitialPhase.getValue(), lfoTargetFreq);
  for (auto &x : spc) x.reset(spcParam.frameSizeLog2);

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

// Output range is in [0, 1].
inline Sample phaseToWave(Sample phase, Sample mod, LfoWaveform waveform)
{
  switch (waveform) {
    case LfoWaveform::triSaw: {
      constexpr Sample eps = std::numeric_limits<Sample>::epsilon();
      const auto pw = mod * Sample(0.5) + Sample(0.5);
      if (pw <= eps) return Sample(1) - phase;
      if (pw >= Sample(1) - eps) return phase;
      return phase < pw ? phase / pw : (Sample(1) - phase) / (Sample(1) - pw);
    } break;
  }
  // LfoWaveform::sine
  constexpr Sample twopi = Sample(2) * std::numbers::pi_v<Sample>;
  return std::sin(twopi * (phase + mod * std::sin(twopi * phase)));
}

std::array<Sample, 2>
DSPCore::processFrame(int frameIndex, Sample in0, Sample in1, Sample side0, Sample side1)
{
  lfoInitialPhase.process();

  const auto processLfoSync = [&]() {
    auto samplesElapsed = sampleRate * beatsElapsed * 60 / tempo;
    auto targetPhase = Sample((samplesElapsed + frameIndex) * lfoTargetFreq);
    targetPhase -= std::floor(targetPhase);
    return lfo.processSync(
      targetPhase + lfoInitialPhase.getValue(), lfoTargetFreq, lfoSyncRate);
  };

  auto modPhase0 = isPlaying ? processLfoSync() : lfo.process(lfoTargetFreq);
  auto modPhase1 = modPhase0 + lfoStereoPhaseOffset.process();
  modPhase1 -= std::floor(modPhase1);

  lfoWaveMod.process();
  auto mod0 = phaseToWave(modPhase0, lfoWaveMod.getValue(), lfoWaveform);
  auto mod1 = phaseToWave(modPhase1, lfoWaveMod.getValue(), lfoWaveform);

  spcParam.dryWetMix = dryWetMix.process();
  spcParam.feedback = feedback.process();
  spectralShift.process();
  octaveDown.process();
  maskMix.process();
  maskPhase.process();
  maskFreq.process();
  maskThreshold.process();
  maskRotation.process();
  lfoToMaskMix.process();
  lfoToMaskPhase.process();
  lfoToMaskFreq.process();
  lfoToMaskThreshold.process();
  lfoToMaskRotation.process();
  lfoToSpectralShift.process();
  lfoToOctaveDown.process();

  const auto modulateParam = [&](Sample unipoler) {
    const auto bipoler = unipoler - Sample(0.5);

    spcParam.spectralShift
      = spectralShift.getValue() + lfoToSpectralShift.getValue() * bipoler;
    spcParam.spectralShift = spcParam.spectralShift - std::floor(spcParam.spectralShift);

    spcParam.octaveDown = octaveDown.getValue() + lfoToOctaveDown.getValue() * unipoler;
    spcParam.octaveDown -= std::floor(spcParam.octaveDown);

    spcParam.maskMix = std::clamp(
      maskMix.getValue() + lfoToMaskMix.getValue() * bipoler, Sample(0), Sample(1));
    spcParam.maskPhase = maskPhase.getValue() + lfoToMaskPhase.getValue() * unipoler;
    spcParam.maskFreq
      = maskFreq.getValue() * std::exp2(Sample(6) * lfoToMaskFreq.getValue() * unipoler);

    spcParam.maskThreshold
      = maskThreshold.getValue() + lfoToMaskThreshold.getValue() * unipoler;
    spcParam.maskThreshold
      = std::clamp(Sample(2) * spcParam.maskThreshold - Sample(1), Sample(-1), Sample(1));

    spcParam.maskRotation
      = maskRotation.getValue() + lfoToMaskRotation.getValue() * unipoler;
  };

  modulateParam(mod0);
  auto sig0 = spc[0].process(in0, side0, spcParam);

  modulateParam(mod1);
  auto sig1 = spc[1].process(in1, side1, spcParam);

  outputGain.process();
  return {
    outputGain.getValue() * sig0,
    outputGain.getValue() * sig1,
  };
}

void DSPCore::process(
  const size_t length,
  const float *in0,
  const float *in1,
  const float *side0,
  const float *side1,
  float *out0,
  float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<Sample>::setBufferSize(Sample(length));
  lfoTargetFreq = getTempoSyncFrequency();

  for (int i = 0; i < length; ++i) {
    auto frame = processFrame(
      i, Sample(in0[i]), Sample(in1[i]), Sample(side0[i]), Sample(side1[i]));
    out0[i] = float(frame[0]);
    out1[i] = float(frame[1]);
  }
}

Sample DSPCore::getTempoSyncFrequency()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getDouble();
  if (lfoRate > Scales::lfoRate.getMax()) return 0;

  auto upper = (pv[ID::lfoTempoUpper]->getDouble() + double(1)) * timeSigLower;
  auto lower = pv[ID::lfoTempoLower]->getDouble() + double(1);

  //
  // Simplified version of following expressions.
  //
  // ```
  // syncBeat       = upper / (lower * lfoRate);
  // secondsPerBeat = 60 / tempoInBpm;
  // lfoFreq        = 1 / (syncBeat * secondPerBeat * sampleRate);
  // ```
  return Sample((tempo * lower * lfoRate) / (60 * upper * sampleRate));
}
