// (c) 2022 Takamitsu Endo
//
// This file is part of LongPhaser.
//
// LongPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LongPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LongPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float defaultTempo = 120.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  pitchSmoothingKp = EMAFilter<double>::secondToP(upRate, double(0.01));

  for (auto &channel : allpass) {
    for (auto &ap : channel) ap.setup(this->sampleRate * upFold, maxDelayTime);
  }

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
  lfoPhaseConstant.METHOD(pv[ID::lfoPhaseConstant]->getDouble());                        \
  lfoPhaseOffset.METHOD(pv[ID::lfoPhaseOffset]->getDouble());                            \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  mix.METHOD(pv[ID::mix]->getDouble());                                                  \
  outerFeed.METHOD(pv[ID::outerFeed]->getDouble());                                      \
  innerFeed.METHOD(pv[ID::innerFeed]->getDouble());                                      \
  lfoToInnerFeed.METHOD(pv[ID::lfoToInnerFeed]->getDouble());                            \
  delayTimeSpread.METHOD(pv[ID::delayTimeSpread]->getDouble());                          \
  delayTimeCenterSamples.METHOD(pv[ID::delayTimeCenterSeconds]->getDouble() * upRate);   \
  delayTimeRateLimit.METHOD(pv[ID::delayTimeRateLimit]->getDouble());                    \
  lfoToDelayTimeOctave.METHOD(pv[ID::lfoToDelayTimeOctave]->getDouble());                \
  inputToDelayTime.METHOD(pv[ID::inputToDelayTime]->getDouble());                        \
                                                                                         \
  delayTimeModType = pv[ID::delayTimeModType]->getInt();

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

  notePitchInv.reset(double(1));

  currentAllpassStage = pv[ID::stage]->getInt();
  previousAllpassStage = currentAllpassStage;
  transitionSamples = size_t(upRate * pv[ID::parameterSmoothingSecond]->getDouble());
  transitionCounter = 0;

  lfo.reset();

  previousInput.fill({});
  feedbackBuffer.fill({});
  upsampleBuffer.fill({});
  for (auto &channel : allpass) {
    for (auto &ap : channel) ap.reset();
  }
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
  notePitchInv.process(pitchSmoothingKp);

  lfoPhaseConstant.process();
  lfoPhaseOffset.process();
  outputGain.process();
  mix.process();
  outerFeed.process();
  innerFeed.process();
  lfoToInnerFeed.process();
  delayTimeSpread.process();
  delayTimeCenterSamples.process();
  delayTimeRateLimit.process();
  lfoToDelayTimeOctave.process();
  inputToDelayTime.process();

  lfo.offset[0] = lfoPhaseConstant.getValue() + lfoPhaseOffset.getValue();
  lfo.offset[1] = lfoPhaseConstant.getValue() - lfoPhaseOffset.getValue();
  lfo.process(synchronizer.process());

  auto sig0 = frame[0] + outerFeed.getValue() * feedbackBuffer[0];
  auto sig1 = frame[1] + outerFeed.getValue() * feedbackBuffer[1];

  auto fm0 = frame[0] * inputToDelayTime.getValue();
  auto fm1 = frame[1] * inputToDelayTime.getValue();

  auto upRange = double(1) - std::abs(innerFeed.getValue());
  auto downRange = double(2) - upRange;
  if (upRange > lfoToInnerFeed.getValue()) upRange = lfoToInnerFeed.getValue();
  if (downRange > lfoToInnerFeed.getValue()) downRange = lfoToInnerFeed.getValue();
  auto innerFeed0 = lfo.output[0];
  auto innerFeed1 = lfo.output[1];
  innerFeed0 *= lfo.output[0] * innerFeed.getValue() > 0 ? upRange : downRange;
  innerFeed1 *= lfo.output[1] * innerFeed.getValue() > 0 ? upRange : downRange;
  innerFeed0 += innerFeed.getValue();
  innerFeed1 += innerFeed.getValue();

  if (delayTimeModType == 0) { // Multiply
    auto dlyTimeLfo0 = std::exp2(fm0 + lfo.output[0] * lfoToDelayTimeOctave.getValue());
    auto dlyTimeLfo1 = std::exp2(fm1 + lfo.output[1] * lfoToDelayTimeOctave.getValue());
    for (size_t idx = 0; idx < maxAllpass; ++idx) {
      auto base = notePitchInv.getValue() * delayTimeCenterSamples.getValue()
        / (double(1) + idx * delayTimeSpread.getValue());
      sig0 = allpass[0][idx].process(
        sig0, dlyTimeLfo0 * base, delayTimeRateLimit.getValue(), innerFeed0);
      sig1 = allpass[1][idx].process(
        sig1, dlyTimeLfo1 * base, delayTimeRateLimit.getValue(), innerFeed1);
    }
  } else { // Add
    auto dlyTimeLfo0
      = (fm0 + lfo.output[0] * lfoToDelayTimeOctave.getValue()) * upRate / double(8);
    auto dlyTimeLfo1
      = (fm1 + lfo.output[1] * lfoToDelayTimeOctave.getValue()) * upRate / double(8);
    for (size_t idx = 0; idx < maxAllpass; ++idx) {
      auto base = notePitchInv.getValue() * delayTimeCenterSamples.getValue()
        / (double(1) + idx * delayTimeSpread.getValue());
      sig0 = allpass[0][idx].process(
        sig0, dlyTimeLfo0 + base, delayTimeRateLimit.getValue(), innerFeed0);
      sig1 = allpass[1][idx].process(
        sig1, dlyTimeLfo1 + base, delayTimeRateLimit.getValue(), innerFeed1);
    }
  }

  auto apOut0 = allpass[0][currentAllpassStage].output;
  auto apOut1 = allpass[1][currentAllpassStage].output;

  // Process cross-fade only when allpass stage is changed.
  if (transitionCounter > 0) {
    --transitionCounter;
    auto ratio = double(transitionCounter) / double(transitionSamples);
    apOut0 += ratio * (allpass[0][previousAllpassStage].output - apOut0);
    apOut1 += ratio * (allpass[1][previousAllpassStage].output - apOut1);
  }

  feedbackBuffer[0] = lerp(double(frame[0]), apOut0, mix.getValue());
  feedbackBuffer[1] = lerp(double(frame[1]), apOut1, mix.getValue());

  frame[0] = feedbackBuffer[0] * outputGain.getValue();
  frame[1] = feedbackBuffer[1] * outputGain.getValue();
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
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
  notePitchInv.push(calcNotePitch(info.pitch));

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    // notePitchInv.push(double(1));
  } else {
    notePitchInv.push(calcNotePitch(noteStack.back().pitch));
  }
}

double DSPCore::calcNotePitch(double note, double equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  // Pitch is inversed because it will be multiplied to delay time.
  auto scale = pv[ID::notePitchScaling]->getDouble();
  auto center = pv[ID::notePitchOrigin]->getDouble();
  return std::exp2(scale * (center - note) / equalTemperament);
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
