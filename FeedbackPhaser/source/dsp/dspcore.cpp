// (c) 2023 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T freqToSvfG(T normalizedFreq)
{
  static constexpr T minCutoff = T(0.00001);
  static constexpr T nyquist = T(0.49998);

  return std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * T(pi));
}

template<typename T> inline T addAsymmetry(T value, T asym)
{
  auto mult = value * asym;
  return mult > 0 ? std::copysign(mult, value) : value;
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

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
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::notePitchSlideSecond]->getDouble());   \
  pitchReleaseKp                                                                         \
    = EMAFilter<double>::secondToP(upRate, pv[ID::notePitchReleaseSecond]->getDouble()); \
                                                                                         \
  modType = pv[ID::modType]->getInt();                                                   \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  auto mix = pv[ID::mix]->getDouble();                                                   \
  feedbackMix.METHOD(std::abs(mix));                                                     \
  inputMixSign.METHOD(mix >= 0 ? double(1) : double(-1));                                \
  feedback.METHOD(pv[ID::feedback]->getDouble());                                        \
  feedbackClip.METHOD(pv[ID::feedbackClip]->getDouble());                                \
  feedbackHighpassG.METHOD(                                                              \
    freqToSvfG(pv[ID::feedbackHighpassHz]->getDouble() / upRate));                       \
  outputHighpassG.METHOD(freqToSvfG(pv[ID::outputHighpassHz]->getDouble() / upRate));    \
  if (modType == 2) {                                                                    \
    modAmount.METHOD(pv[ID::modAmount]->getDouble() * double(48) / upRate);              \
  } else {                                                                               \
    modAmount.METHOD(pv[ID::modAmount]->getDouble());                                    \
  }                                                                                      \
  modAsymmetry.METHOD(pv[ID::modAsymmetry]->getDouble());                                \
  auto modLowpassHz = pv[ID::modLowpassHz]->getDouble();                                 \
  modLowpassKp.METHOD(                                                                   \
    modLowpassHz >= Scales::cutoffHz.getMax()                                            \
      ? double(1)                                                                        \
      : EMAFilter<double>::cutoffToP(upRate, modLowpassHz));                             \
  allpassSpread.METHOD(pv[ID::allpassSpread]->getDouble());                              \
  allpassCenterCut.METHOD(pv[ID::allpassCenterHz]->getDouble() / upRate);

void DSPCore::updateUpRate()
{
  upRate = double(sampleRate) * fold[oversampling];

  SmootherCommon<double>::setSampleRate(upRate);
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  pv[ID::tooMuchFeedback]->setFromInt(0);
  tooMuchFeedback = false;

  midiNotes.clear();
  noteStack.clear();

  notePitchToAllpassCutoff.reset(double(1));
  notePitchToAllpassCutoffRelease.reset(double(1));

  currentAllpassStage = pv[ID::stage]->getInt();
  previousAllpassStage = currentAllpassStage;
  transitionSamples = size_t(upRate * pv[ID::parameterSmoothingSecond]->getDouble());
  transitionCounter = 0;

  feedbackBuffer.fill({});
  for (auto &x : modLowpass) x.reset();
  for (auto &channel : allpass) {
    for (auto &x : channel) x.reset();
  }
  for (auto &x : feedbackHighpass) x.reset();
  for (auto &x : outputHighpass) x.reset();

  for (auto &x : upSampler) x.reset();
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

  tooMuchFeedback = pv[ID::tooMuchFeedback]->getInt();
}

std::array<double, 2> DSPCore::processFrame(
  const std::array<double, 2> &frame, const std::array<double, 2> &modSig)
{
  notePitchToAllpassCutoffRelease.processKp(
    notePitchToAllpassCutoff.process(pitchSmoothingKp), pitchReleaseKp);

  const auto outGain = outputGain.process();
  const auto fbMix = feedbackMix.process();
  const auto inMixSign = inputMixSign.process();
  const auto fbGain = feedback.process();
  const auto fbClip = feedbackClip.process();
  const auto fbHpG = feedbackHighpassG.process();
  const auto outHpG = outputHighpassG.process();
  const auto modAmt = modAmount.process();
  const auto modAsym = modAsymmetry.process();
  const auto modLpKp = modLowpassKp.process();
  const auto apSpread = allpassSpread.process();
  const auto apCenterCut = allpassCenterCut.process();

  constexpr double eps = (double)std::numeric_limits<float>::epsilon();

  std::array<double, 2> apCut{};
  if (modType == 0) { // Exp. Mul.
    auto apModLower = double(-256) * (modAsym <= 0 ? double(1) + modAsym : double(1));
    auto apModUpper = double(+256) * (modAsym <= 0 ? double(1) : double(1) - modAsym);
    auto apMod0 = std::clamp(modAmt * modSig[0], apModLower, apModUpper);
    auto apMod1 = std::clamp(modAmt * modSig[1], apModLower, apModUpper);
    apCut[0] = std::min(std::exp2(apMod0) * apCenterCut, double(1 - eps));
    apCut[1] = std::min(std::exp2(apMod1) * apCenterCut, double(1 - eps));
  } else if (modType == 1) { // Lin. Mul.
    auto asym = modAsym >= 0 ? double(1) - modAsym : double(-1) - modAsym;
    auto apMod0 = addAsymmetry(double(1) / double(128) * modAmt * modSig[0], asym);
    auto apMod1 = addAsymmetry(double(1) / double(128) * modAmt * modSig[1], asym);
    apCut[0] = std::min(std::abs(apMod0 * apCenterCut), double(1 - eps));
    apCut[1] = std::min(std::abs(apMod1 * apCenterCut), double(1 - eps));
  } else { // Add.
    auto asym = modAsym >= 0 ? double(1) - modAsym : double(-1) - modAsym;
    auto apMod0 = addAsymmetry(modAmt * modSig[0], asym);
    auto apMod1 = addAsymmetry(modAmt * modSig[1], asym);
    apCut[0] = std::min(std::abs(apMod0 + apCenterCut), double(1 - eps));
    apCut[1] = std::min(std::abs(apMod1 + apCenterCut), double(1 - eps));
  }
  apCut[0] = modLowpass[0].processKp(apCut[0], modLpKp);
  apCut[1] = modLowpass[1].processKp(apCut[1], modLpKp);

  auto sig0 = frame[0] + fbGain * feedbackBuffer[0];
  auto sig1 = frame[1] + fbGain * feedbackBuffer[1];
  for (size_t idx = 0; idx < maxAllpass; ++idx) {
    auto multiplier = notePitchToAllpassCutoffRelease.v2 * (double(1) + idx * apSpread);
    sig0 = allpass[0][idx].process(sig0, apCut[0] * multiplier);
    sig1 = allpass[1][idx].process(sig1, apCut[1] * multiplier);
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

  auto out0 = lerp(inMixSign * frame[0], apOut0, fbMix);
  auto out1 = lerp(inMixSign * frame[1], apOut1, fbMix);
  feedbackBuffer[0] = feedbackHighpass[0].highpass(out0, fbHpG);
  feedbackBuffer[1] = feedbackHighpass[1].highpass(out1, fbHpG);
  feedbackBuffer[0] = std::clamp(feedbackBuffer[0], -fbClip, fbClip);
  feedbackBuffer[1] = std::clamp(feedbackBuffer[1], -fbClip, fbClip);

  const auto &buf = feedbackBuffer;
  if (
    !tooMuchFeedback
    && (buf[0] <= -fbClip || buf[0] >= fbClip || buf[1] <= -fbClip || buf[1] >= fbClip))
  {
    tooMuchFeedback = true;
  }

  return {
    outGain * outputHighpass[0].highpass(out0, outHpG),
    outGain * outputHighpass[1].highpass(out1, outHpG),
  };
}

void DSPCore::process(
  const size_t length,
  const float *in0,
  const float *in1,
  const float *in2,
  const float *in3,
  float *out0,
  float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  bool enableSidechain = pv[ID::modSideChain]->getInt();
  const float *side0 = enableSidechain ? in2 : in0;
  const float *side1 = enableSidechain ? in3 : in1;

  SmootherCommon<double>::setBufferSize(double(length));

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

    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);
    upSampler[2].process(side0[i]);
    upSampler[3].process(side1[i]);

    if (oversampling == 2) { // 8x.
      for (size_t j = 0; j < upFold; ++j) {
        auto frame = processFrame(
          {upSampler[0].output[j], upSampler[1].output[j]},
          {upSampler[2].output[j], upSampler[3].output[j]});
        decimationLowpass[0].push(frame[0]);
        decimationLowpass[1].push(frame[1]);
        upSampler[0].output[j] = decimationLowpass[0].output();
        upSampler[1].output[j] = decimationLowpass[1].output();
      }
      out0[i] = halfbandIir[0].process(
        {upSampler[0].output[0], upSampler[0].output[upFold / 2]});
      out1[i] = halfbandIir[1].process(
        {upSampler[1].output[0], upSampler[1].output[upFold / 2]});
    } else if (oversampling == 1) { // 2x.
      const size_t mid = upFold / 2;
      for (size_t j = 0; j < upFold; j += mid) {
        auto frame = processFrame(
          {upSampler[0].output[j], upSampler[1].output[j]},
          {upSampler[2].output[j], upSampler[3].output[j]});
        upSampler[0].output[j] = frame[0];
        upSampler[1].output[j] = frame[1];
      }
      out0[i]
        = halfbandIir[0].process({upSampler[0].output[0], upSampler[0].output[mid]});
      out1[i]
        = halfbandIir[1].process({upSampler[1].output[0], upSampler[1].output[mid]});
    } else { // 1x.
      auto frame = processFrame(
        {upSampler[0].output[0], upSampler[1].output[0]},
        {upSampler[2].output[0], upSampler[3].output[0]});
      out0[i] = frame[0];
      out1[i] = frame[1];
    }
  }

  if (transitionCounter == 0) previousAllpassStage = currentAllpassStage;

  // Send a value to GUI.
  if (tooMuchFeedback) pv[ID::tooMuchFeedback]->setFromInt(1);
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

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
    notePitchToAllpassCutoff.push(double(1));
  } else {
    notePitchToAllpassCutoff.push(calcNotePitch(
      noteStack.back().pitch, pv[ID::notePitchToAllpassCutoff]->getDouble()));
  }
}

double DSPCore::calcNotePitch(double note, double scale, double equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto center = pv[ID::notePitchCenter]->getDouble();
  return std::max(
    std::exp2(scale * (note - center) / equalTemperament),
    std::numeric_limits<double>::epsilon());
}
