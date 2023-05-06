// (c) 2022 Takamitsu Endo
//
// This file is part of CombDistortion.
//
// CombDistortion is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CombDistortion is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CombDistortion.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  pitchSmoothingKp = EMAFilter<double>::secondToP(upRate, double(0.05));

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
  pitchReleaseKp                                                                         \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteReleaseSeconds]->getDouble());     \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  mix.METHOD(pv[ID::mix]->getDouble());                                                  \
  feedback.METHOD(pv[ID::feedback]->getDouble());                                        \
  feedbackHighpassKp.METHOD(                                                             \
    EMAFilter<double>::cutoffToP(upRate, pv[ID::feedbackHighpassHz]->getDouble()));      \
                                                                                         \
  const auto lowpassHz = pv[ID::feedbackLowpassHz]->getDouble();                         \
  const auto lowpassKp = lowpassHz >= Scales::lowpassHz.getMax()                         \
    ? double(1)                                                                          \
    : EMAFilter<double>::cutoffToP(upRate, lowpassHz);                                   \
  feedbackLowpassKp.METHOD(lowpassKp);                                                   \
                                                                                         \
  delayTimeSamples.METHOD(pv[ID::delayTimeSeconds]->getDouble() * upRate);               \
  amMix.METHOD(pv[ID::amMix]->getDouble());                                              \
  amClipGain.METHOD(                                                                     \
    pv[ID::amClipGainNegative]->getInt() ? -pv[ID::amClipGain]->getDouble()              \
                                         : pv[ID::amClipGain]->getDouble());             \
  fmMix.METHOD(pv[ID::fmMix]->getDouble());                                              \
  fmAmount.METHOD(pv[ID::fmAmount]->getDouble());                                        \
  fmClip.METHOD(pv[ID::fmClip]->getDouble());

void DSPCore::updateUpRate()
{
  constexpr std::array<size_t, 3> fold{1, upFold, upFold};
  upRate = double(sampleRate) * fold[oversampling];

  SmootherCommon<double>::setSampleRate(upRate);
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  midiNotes.clear();
  noteStack.clear();

  notePitchToDelayTime.reset(double(1));
  notePitchToDelayTimeRelease.reset(double(1));

  feedbackBuffer.fill({});

  for (auto &x : upSampler) x.reset();
  for (auto &x : feedbackHighpass) x.reset();
  for (auto &x : feedbackLowpass) x.reset();
  for (auto &x : feedbackDelay) x.reset();
  for (auto &x : decimationLowpass) x.reset();
  for (auto &x : halfbandIir) x.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  size_t newOversampling = param.value[ParameterID::ID::oversampling]->getInt();
  if (oversampling != newOversampling) {
    oversampling = newOversampling;
    updateUpRate();
  }

  ASSIGN_PARAMETER(push);
}

std::array<double, 2> DSPCore::processFrame(const std::array<double, 2> &frame)
{
  notePitchToDelayTimeRelease.processKp(
    notePitchToDelayTime.process(pitchSmoothingKp), pitchReleaseKp);

  outputGain.process();
  mix.process();
  feedback.process();
  feedbackHighpassKp.process();
  feedbackLowpassKp.process();
  delayTimeSamples.process();
  amMix.process();
  amClipGain.process();
  fmMix.process();
  fmAmount.process();
  fmClip.process();

  auto delayTimeBase = delayTimeSamples.process() * notePitchToDelayTimeRelease.v2;

  auto fm0 = std::min(fmAmount.getValue() * std::abs(frame[0]), fmClip.getValue());
  auto fm1 = std::min(fmAmount.getValue() * std::abs(frame[1]), fmClip.getValue());
  auto baseTime0 = delayTimeBase * lerp(double(1), fm0, fmMix.getValue());
  auto baseTime1 = delayTimeBase * lerp(double(1), fm1, fmMix.getValue());

  auto clippedIn0 = std::tanh(amClipGain.getValue() * frame[0]);
  auto clippedIn1 = std::tanh(amClipGain.getValue() * frame[1]);
  auto am0 = lerp(double(1), clippedIn0, amMix.getValue());
  auto am1 = lerp(double(1), clippedIn1, amMix.getValue());

  auto hp0
    = feedbackHighpass[0].process(feedbackBuffer[0], feedbackHighpassKp.getValue());
  auto hp1
    = feedbackHighpass[1].process(feedbackBuffer[1], feedbackHighpassKp.getValue());
  auto fb0 = feedbackLowpass[0].processKp(hp0, feedbackLowpassKp.getValue());
  auto fb1 = feedbackLowpass[1].processKp(hp1, feedbackLowpassKp.getValue());

  auto sig0
    = frame[0] + am0 * feedbackDelay[0].process(feedback.getValue() * fb0, baseTime0);
  auto sig1
    = frame[1] + am1 * feedbackDelay[1].process(feedback.getValue() * fb1, baseTime1);

  feedbackBuffer[0] = lerp(frame[0], sig0, mix.getValue());
  feedbackBuffer[1] = lerp(frame[1], sig1, mix.getValue());

  return {
    feedbackBuffer[0] * outputGain.getValue(),
    feedbackBuffer[1] * outputGain.getValue(),
  };
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);

    if (oversampling == 2) { // 16x.
      for (size_t j = 0; j < upFold; ++j) {
        auto frame = processFrame({upSampler[0].output[j], upSampler[1].output[j]});
        decimationLowpass[0].push(frame[0]);
        decimationLowpass[1].push(frame[1]);
        upSampler[0].output[j] = decimationLowpass[0].output();
        upSampler[1].output[j] = decimationLowpass[1].output();
      }
      out0[i] = halfbandIir[0].process(
        {upSampler[0].output[0], upSampler[0].output[upFold / 2]});
      out1[i] = halfbandIir[1].process(
        {upSampler[1].output[0], upSampler[1].output[upFold / 2]});
    } else if (oversampling == 1) { // Incomplete 16x.
      for (size_t j = 0; j < upFold / 2; ++j) {
        auto frame = processFrame({upSampler[0].output[j], upSampler[1].output[j]});
        decimationLowpass[0].push(frame[0]);
        decimationLowpass[1].push(frame[1]);
        upSampler[0].output[j] = decimationLowpass[0].output();
        upSampler[1].output[j] = decimationLowpass[1].output();
      }
      out0[i] = halfbandIir[0].process({upSampler[0].output[0], double(0)});
      out1[i] = halfbandIir[1].process({upSampler[1].output[0], double(0)});
    } else { // 1x.
      auto frame = processFrame({upSampler[0].output[0], upSampler[1].output[0]});
      out0[i] = frame[0];
      out1[i] = frame[1];
    }
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto pitchToTime
    = calcNotePitch(info.pitch, -pv[ID::notePitchToDelayTime]->getDouble());
  notePitchToDelayTime.push(pitchToTime);
  notePitchToDelayTimeRelease.reset(pitchToTime);

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
  } else {
    notePitchToDelayTime.push(
      calcNotePitch(noteStack.back().pitch, pv[ID::notePitchToDelayTime]->getDouble()));
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
