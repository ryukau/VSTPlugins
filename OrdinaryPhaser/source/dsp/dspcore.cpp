// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float defaultTempo = 120.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  pitchSmoothingKp = EMAFilter<double>::secondToP(upRate, double(0.05));
  pitchReleaseKp = EMAFilter<double>::secondToP(upRate, double(4));

  for (auto &dly : feedbackDelay) dly.setup(this->sampleRate * upFold, maxDelayTime);

  reset();
  startup();
}

size_t DSPCore::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  SmootherCommon<double>::setTime(pv[ID::parameterSmoothingSecond]->getDouble());        \
                                                                                         \
  lfo.interpType = pv[ID::lfoInterpolation]->getInt();                                   \
  for (size_t idx = 0; idx < nLfoWavetable; ++idx) {                                     \
    lfo.source[idx + 1] = pv[ID::lfoWavetable0 + idx]->getFloat();                       \
  }                                                                                      \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  mix.METHOD(pv[ID::mix]->getDouble());                                                  \
  lfoPhaseOffset.METHOD(pv[ID::lfoPhaseOffset]->getDouble());                            \
  lfoPhaseConstant.METHOD(pv[ID::lfoPhaseConstant]->getDouble());                        \
  cutoffSpread.METHOD(pv[ID::cutoffSpread]->getDouble());                                \
  cutoffMinHz.METHOD(pv[ID::cutoffMinHz]->getDouble());                                  \
  cutoffMaxHz.METHOD(pv[ID::cutoffMaxHz]->getDouble());                                  \
  feedback.METHOD(pv[ID::feedback]->getDouble());                                        \
  delayTimeSamples.METHOD(pv[ID::delayTimeSeconds]->getDouble() * upRate);               \
  lfoToDelay.METHOD(pv[ID::lfoToDelayAmount]->getDouble());                              \
  inputToFeedbackGain.METHOD(pv[ID::inputToFeedbackGain]->getDouble());                  \
  inputToDelayTime.METHOD(pv[ID::inputToDelayTime]->getDouble());                        \
                                                                                         \
  lfoToDelayTuningType = pv[ID::lfoToDelayTuningType]->getInt();

void DSPCore::updateUpRate()
{
  auto fold = oversampling ? upFold : size_t(1);
  upRate = double(sampleRate) * fold;

  SmootherCommon<double>::setSampleRate(upRate);

  synchronizer.reset(upRate, defaultTempo, double(1));
  lfo.setup(upRate, double(0.1));
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  midiNotes.clear();
  noteStack.clear();

  notePitchToDelayTime.reset(double(1));
  notePitchToAllpassCutoff.reset(double(1));
  notePitchToDelayTimeRelease.reset(double(1));
  notePitchToAllpassCutoffRelease.reset(double(1));

  currentAllpassStage = pv[ID::stage]->getInt();
  previousAllpassStage = currentAllpassStage;
  transitionSamples = size_t(upRate * pv[ID::parameterSmoothingSecond]->getDouble());
  transitionCounter = 0;

  lfo.reset();

  previousInput.fill({});
  upsampleBuffer.fill({});
  feedbackBuffer.fill({});
  for (auto &channel : allpass) {
    for (auto &ap : channel) ap.reset();
  }
  for (auto &dly : feedbackDelay) dly.reset();
  for (auto &hb : halfbandIir) hb.reset();

  startup();
}

void DSPCore::startup() { synchronizer.reset(upRate, tempo, getTempoSyncInterval()); }

void DSPCore::setParameters()
{
  bool newOversampling = param.value[ParameterID::ID::oversampling]->getInt();
  if (oversampling != newOversampling) {
    oversampling = newOversampling;
    updateUpRate();
  }

  ASSIGN_PARAMETER(push);
}

void DSPCore::processFrame(std::array<double, 2> &frame)
{
  notePitchToDelayTimeRelease.processKp(
    notePitchToDelayTime.process(pitchSmoothingKp), pitchReleaseKp);
  notePitchToAllpassCutoffRelease.processKp(
    notePitchToAllpassCutoff.process(pitchSmoothingKp), pitchReleaseKp);

  lfoPhaseConstant.process();
  lfoPhaseOffset.process();

  outputGain.process();
  mix.process();
  cutoffSpread.process();
  auto cutMinHz = cutoffMinHz.process();
  auto cutMaxHz = cutoffMaxHz.process();
  feedback.process();
  delayTimeSamples.process();
  lfoToDelay.process();
  inputToFeedbackGain.process();
  inputToDelayTime.process();

  lfo.offset[0] = lfoPhaseConstant.getValue() + lfoPhaseOffset.getValue();
  lfo.offset[1] = lfoPhaseConstant.getValue() - lfoPhaseOffset.getValue();
  lfo.process(synchronizer.process());

  if (cutMinHz > cutMaxHz) std::swap(cutMaxHz, cutMaxHz);
  auto rangeHz = double(0.5) * (cutMaxHz - cutMinHz);
  auto centerHz = cutMinHz + rangeHz;
  auto apCut0 = (centerHz + rangeHz * lfo.output[0]) / upRate;
  auto apCut1 = (centerHz + rangeHz * lfo.output[1]) / upRate;

  std::array<double, 2> dt{};
  auto delayTimeBase = delayTimeSamples.process() * notePitchToDelayTimeRelease.v2;
  auto baseTime0
    = delayTimeBase * lerp(double(1), std::abs(frame[0]), inputToDelayTime.getValue());
  auto baseTime1
    = delayTimeBase * lerp(double(1), std::abs(frame[1]), inputToDelayTime.getValue());
  switch (lfoToDelayTuningType) {
    case 0: { // Exp Mul.
      auto amount = double(8) * lfoToDelay.getValue();
      dt[0] = baseTime0 * std::exp2(amount * lfo.output[0]);
      dt[1] = baseTime1 * std::exp2(amount * lfo.output[1]);
    } break;
    case 1: { // Linear Mul.
      dt[0] = baseTime0 * (double(1) + lfoToDelay.getValue() * lfo.output[0]);
      dt[1] = baseTime1 * (double(1) + lfoToDelay.getValue() * lfo.output[1]);
    } break;
    case 2: { // Add
      auto range = lfoToDelay.getValue() * maxDelayTime * upRate;
      dt[0] = baseTime0 + lfo.output[0] * range;
      dt[1] = baseTime1 + lfo.output[1] * range;
    } break;
    case 3: { // Fill Lower
      auto range0 = lfoToDelay.getValue() * double(0.5) * baseTime0;
      auto range1 = lfoToDelay.getValue() * double(0.5) * baseTime1;
      dt[0] = baseTime0 - range0 + lfo.output[0] * range0;
      dt[1] = baseTime1 - range1 + lfo.output[1] * range1;
    } break;
    case 4: { // Fill Higher
      auto range0
        = lfoToDelay.getValue() * double(0.5) * (maxDelayTime * upRate - baseTime0);
      auto range1
        = lfoToDelay.getValue() * double(0.5) * (maxDelayTime * upRate - baseTime1);
      dt[0] = baseTime0 + range0 + lfo.output[0] * range0;
      dt[1] = baseTime1 + range1 + lfo.output[1] * range1;
    } break;
  }

  auto clippedIn0 = std::tanh(frame[0]);
  auto clippedIn1 = std::tanh(frame[1]);
  auto am0 = lerp(double(1), clippedIn0, inputToFeedbackGain.getValue());
  auto am1 = lerp(double(1), clippedIn1, inputToFeedbackGain.getValue());

  auto sig0 = frame[0]
    + am0 * feedbackDelay[0].process(feedback.getValue() * feedbackBuffer[0], dt[0]);
  auto sig1 = frame[1]
    + am1 * feedbackDelay[1].process(feedback.getValue() * feedbackBuffer[1], dt[1]);

  for (size_t idx = 0; idx < maxAllpass; ++idx) {
    auto multiplier
      = notePitchToAllpassCutoffRelease.v2 * (double(1) + idx * cutoffSpread.getValue());
    sig0 = allpass[0][idx].process(sig0, apCut0 * multiplier);
    sig1 = allpass[1][idx].process(sig1, apCut1 * multiplier);
  }

  auto apOut0 = allpass[0][currentAllpassStage].output();
  auto apOut1 = allpass[1][currentAllpassStage].output();

  // Process cross-fade only when allpass stage is changed.
  if (transitionCounter > 0) {
    --transitionCounter;
    auto ratio = double(transitionCounter) / double(transitionSamples);
    apOut0 += ratio * (allpass[0][previousAllpassStage].output() - apOut0);
    apOut1 += ratio * (allpass[1][previousAllpassStage].output() - apOut1);
  }

  feedbackBuffer[0] = lerp(double(frame[0]), apOut0, mix.getValue());
  feedbackBuffer[1] = lerp(double(frame[1]), apOut1, mix.getValue());

  frame[0] = feedbackBuffer[0] * outputGain.getValue();
  frame[1] = feedbackBuffer[1] * outputGain.getValue();
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    upRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(), beatsElapsed,
    !isTempoSyncing || !isPlaying);

  if (transitionCounter == 0) {
    currentAllpassStage = pv[ID::stage]->getInt();
    if (previousAllpassStage != currentAllpassStage) {
      auto scale = lerp(
        1.0, 0.2,
        std::min(previousAllpassStage, currentAllpassStage) / double(maxAllpass - 1));
      auto smoothingSecond
        = std::min(double(2), pv[ID::parameterSmoothingSecond]->getDouble());
      transitionSamples = size_t(1) + size_t(scale * upRate * smoothingSecond);
      transitionCounter = transitionSamples;
    }
  }

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (oversampling) {
      // Crude up-sampling with linear interpolation.
      upsampleBuffer[0] = {
        double(0.5) * (previousInput[0] + in0[i]),
        double(0.5) * (previousInput[1] + in1[i])};
      upsampleBuffer[1] = {double(in0[i]), double(in1[i])};

      for (size_t j = 0; j < upFold; ++j) processFrame(upsampleBuffer[j]);

      out0[i] = halfbandIir[0].process({upsampleBuffer[0][0], upsampleBuffer[1][0]});
      out1[i] = halfbandIir[1].process({upsampleBuffer[0][1], upsampleBuffer[1][1]});
    } else {
      upsampleBuffer[0] = {double(in0[i]), double(in1[i])};

      processFrame(upsampleBuffer[0]);

      out0[i] = upsampleBuffer[0][0];
      out1[i] = upsampleBuffer[0][1];
    }

    previousInput[0] = in0[i];
    previousInput[1] = in1[i];
  }

  if (transitionCounter == 0) previousAllpassStage = currentAllpassStage;
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto pitchToTime
    = calcNotePitch(info.pitch, -pv[ID::notePitchToDelayTime]->getDouble());
  notePitchToDelayTime.push(pitchToTime);
  notePitchToDelayTimeRelease.reset(pitchToTime);

  auto pitchToCutoff
    = calcNotePitch(info.pitch, pv[ID::notePitchToAllpassCutoff]->getDouble());
  notePitchToAllpassCutoff.push(pitchToCutoff);
  notePitchToAllpassCutoffRelease.reset(pitchToCutoff);

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    notePitchToDelayTime.push(double(1));
    notePitchToAllpassCutoff.push(double(1));
  } else {
    notePitchToDelayTime.push(
      calcNotePitch(noteStack.back().pitch, pv[ID::notePitchToDelayTime]->getDouble()));
    notePitchToAllpassCutoff.push(calcNotePitch(
      noteStack.back().pitch, pv[ID::notePitchToAllpassCutoff]->getDouble()));
  }
}

double DSPCore::calcNotePitch(double note, double scale, double equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto center = pv[ID::notePitchOrigin]->getDouble();
  return std::max(
    std::exp2(scale * (note - center) / equalTemperament),
    std::numeric_limits<double>::epsilon());
}

double DSPCore::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getDouble();
  if (lfoRate > Scales::lfoRate.getMax()) return 0;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto upper = pv[ID::lfoTempoUpper]->getDouble() + double(1);
  auto lower = pv[ID::lfoTempoLower]->getDouble() + double(1);
  return pv[ID::lfoTempoSync]->getInt()
    ? (4 * timeSigUpper * upper) / (timeSigLower * lower * lfoRate)
    : (4 * upper) / (lower * lfoRate);
}
