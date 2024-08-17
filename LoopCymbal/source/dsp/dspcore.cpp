// (c) 2024 Takamitsu Endo
//
// This file is part of LoopCymbal.
//
// LoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

constexpr double defaultTempo = double(120);
constexpr double releaseTimeSecond = double(0.05);

template<size_t nAllpass, typename Rng>
inline auto prepareSerialAllpassTime(double upRate, double allpassMaxTimeHz, Rng &rng)
{
  std::array<double, nAllpass> delaySamples{};
  const auto scaler = std::max(
    double(0), std::ceil(upRate * nAllpass / allpassMaxTimeHz) - double(2) * nAllpass);
  double sumSamples = 0;
  std::uniform_real_distribution<double> dist{double(0), double(1)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    delaySamples[idx] = dist(rng);
    sumSamples += delaySamples[idx];
  }
  double sumFraction = 0;
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    const auto samples = double(2) + scaler * delaySamples[idx] / sumSamples;
    delaySamples[idx] = std::floor(samples);
    sumFraction += samples - delaySamples[idx];
  }
  delaySamples[0] += std::round(sumFraction);
  return delaySamples;
}

template<typename Rng>
inline double pitchRatio(double pitch, double spread, double rndCent, Rng &rng)
{
  const auto rndRange = rndCent / double(1200);
  std::uniform_real_distribution<double> dist{-rndRange, rndRange};
  return std::lerp(double(1), pitch, spread) * std::exp2(dist(rng));
}

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = sampleRate;
  upRate = sampleRate * upFold;

  SmootherCommon<double>::setTime(double(0.2));

  const auto maxDelayTimeSamples = upRate * 2 * Scales::delayTimeSecond.getMax();
  for (auto &x : serialAllpass1) x.setup(maxDelayTimeSamples);
  for (auto &x : serialAllpass2) x.setup(maxDelayTimeSamples);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  useExternalInput = pv[ID::useExternalInput]->getInt();                                 \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  externalInputGain.METHOD(pv[ID::externalInputGain]->getDouble());                      \
  delayTimeModAmount.METHOD(                                                             \
    pv[ID::delayTimeModAmount]->getDouble() * upRate / double(48000));                   \
  allpassFeed1.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed1]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassFeed2.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed2]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassMixSpike.METHOD(pv[ID::allpassMixSpike]->getDouble());                          \
  allpassMixAltSign.METHOD(pv[ID::allpassMixAltSign]->getDouble());                      \
  highShelfCutoff.METHOD(                                                                \
    EMAFilter<double>::cutoffToP(upRate, pv[ID::highShelfFrequencyHz]->getDouble()));    \
  highShelfGain.METHOD(pv[ID::highShelfGain]->getDouble());                              \
  lowShelfCutoff.METHOD(                                                                 \
    EMAFilter<double>::cutoffToP(upRate, pv[ID::lowShelfFrequencyHz]->getDouble()));     \
  lowShelfGain.METHOD(pv[ID::lowShelfGain]->getDouble());                                \
  stereoBalance.METHOD(pv[ID::stereoBalance]->getDouble());                              \
  stereoMerge.METHOD(pv[ID::stereoMerge]->getDouble() / double(2));                      \
                                                                                         \
  auto gain = pv[ID::outputGain]->getDouble();                                           \
  outputGain.METHOD(gain);                                                               \
                                                                                         \
  envelopeNoise.setTime(pv[ID::noiseDecaySeconds]->getDouble() * upRate);                \
  if (!pv[ID::release]->getInt() && noteStack.empty()) {                                 \
    envelopeRelease.setTime(releaseTimeSecond *upRate);                                  \
  }                                                                                      \
                                                                                         \
  updateDelayTime();

void DSPCore::updateUpRate()
{
  upRate = sampleRate * fold[overSampling];
  SmootherCommon<double>::setSampleRate(upRate);
}

void DSPCore::updateDelayTime()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  paramRng.seed(pv[ID::seed]->getInt());
  const auto delayTimeBase = pv[ID::delayTimeBaseSecond]->getDouble() * upRate;
  const auto delayTimeRandom = pv[ID::delayTimeRandomSecond]->getDouble() * upRate;
  std::uniform_real_distribution<double> delayTimeDist{
    double(0), double(delayTimeRandom)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    const auto timeHarmonics = delayTimeBase / double(idx + 1);
    serialAllpass1[0].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass1[1].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass2[0].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
    serialAllpass2[1].timeInSamples[idx] = timeHarmonics + delayTimeDist(paramRng);
  }
}

void DSPCore::reset()
{
  noteNumber = 57.0;
  velocity = 0;

  overSampling = param.value[ParameterID::ID::overSampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  startup();

  impulse = 0;
  envelopeNoise.reset();
  envelopeRelease.reset();
  feedbackBuffer1.fill(double(0));
  feedbackBuffer2.fill(double(0));
  for (auto &x : serialAllpass1) x.reset();
  for (auto &x : serialAllpass2) x.reset();

  for (auto &x : halfbandInput) x.fill({});
  for (auto &x : halfbandIir) x.reset();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;
  noiseRng.seed(pv[ID::seed]->getInt());
}

void DSPCore::setParameters()
{
  size_t newOverSampling = param.value[ParameterID::ID::overSampling]->getInt();
  if (overSampling != newOverSampling) {
    overSampling = newOverSampling;
    updateUpRate();
  }
  ASSIGN_PARAMETER(push);
}

std::array<double, 2> DSPCore::processFrame(const std::array<double, 2> &externalInput)
{
  const auto envRelease = envelopeRelease.process();

  const auto extGain = externalInputGain.process();
  const auto timeModAmt = delayTimeModAmount.process();

  auto apGain1 = allpassFeed1.process();
  auto apGain2 = allpassFeed2.process();
  apGain1 = std::lerp(apGain1 * double(0.5), apGain1, envRelease);
  apGain2 = std::lerp(apGain2 * double(0.5), apGain2, envRelease);

  const auto apMixSpike = allpassMixSpike.process();
  const auto apMixSign = allpassMixAltSign.process();
  const auto hsCut = highShelfCutoff.process();
  const auto hsGain = highShelfGain.process() * envRelease;
  const auto lsCut = lowShelfCutoff.process();
  const auto lsGain = lowShelfGain.process();
  const auto balance = stereoBalance.process();
  const auto merge = stereoMerge.process();
  const auto outGain = outputGain.process() * envRelease;

  std::uniform_real_distribution<double> dist{double(-1), double(1)};
  const auto noiseEnv = envelopeNoise.process();
  std::array<double, 2> excitation{
    -apGain1 * feedbackBuffer1[0], -apGain1 * feedbackBuffer1[1]};
  if (impulse != 0) {
    excitation[0] += impulse;
    excitation[1] += impulse;
    impulse = 0;
  } else {
    excitation[0] += noiseEnv * dist(noiseRng);
    excitation[1] += noiseEnv * dist(noiseRng);
  }

  if (useExternalInput) {
    excitation[0] += externalInput[0] * extGain;
    excitation[1] += externalInput[1] * extGain;
  }

  // Normalize amplitude.
  const auto pitchRatio = interpPitch.process(pitchSmoothingKp);
  const auto normalizeGain = nAllpass
    * std::lerp(double(1) / std::sqrt(hsCut / (double(2) - hsCut)), hsGain, hsGain);
  auto ap1Out0
    = std::lerp(serialAllpass1[0].sum(apMixSign), feedbackBuffer1[0], apMixSpike)
    * normalizeGain;
  auto ap1Out1
    = std::lerp(serialAllpass1[1].sum(apMixSign), feedbackBuffer1[1], apMixSpike)
    * normalizeGain;

  feedbackBuffer1[0] = serialAllpass1[0].process(
    excitation[0], hsCut, hsGain, lsCut, lsGain, apGain1, pitchRatio, timeModAmt);
  feedbackBuffer1[1] = serialAllpass1[1].process(
    excitation[1], hsCut, hsGain, lsCut, lsGain, apGain1, pitchRatio, timeModAmt);

  auto cymbal0
    = std::lerp(serialAllpass2[0].sum(apMixSign), feedbackBuffer2[0], apMixSpike)
    * normalizeGain;
  auto cymbal1
    = std::lerp(serialAllpass2[1].sum(apMixSign), feedbackBuffer2[1], apMixSpike)
    * normalizeGain;
  feedbackBuffer2[0] = serialAllpass2[0].process(
    ap1Out0 - apGain2 * feedbackBuffer2[0], hsCut, hsGain, lsCut, lsGain, apGain2,
    pitchRatio, timeModAmt);
  feedbackBuffer2[1] = serialAllpass2[1].process(
    ap1Out1 - apGain2 * feedbackBuffer2[1], hsCut, hsGain, lsCut, lsGain, apGain2,
    pitchRatio, timeModAmt);

  constexpr auto eps = std::numeric_limits<double>::epsilon();
  if (balance < -eps) {
    cymbal0 *= double(1) + balance;
  } else if (balance > eps) {
    cymbal1 *= double(1) - balance;
  }
  return {
    outGain * std::lerp(cymbal0, cymbal1, merge),
    outGain * std::lerp(cymbal1, cymbal0, merge),
  };
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  std::array<double, 2> prevExtIn = halfbandInput[0];
  std::array<double, 2> frame{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    const double extIn0 = in0 == nullptr ? 0 : in0[i];
    const double extIn1 = in1 == nullptr ? 0 : in1[i];

    if (overSampling) {
      frame = processFrame({
        double(0.5) * (prevExtIn[0] + extIn0),
        double(0.5) * (prevExtIn[1] + extIn1),
      });
      halfbandInput[0][0] = frame[0];
      halfbandInput[1][0] = frame[1];

      frame = processFrame({extIn0, extIn1});
      halfbandInput[0][1] = frame[0];
      halfbandInput[1][1] = frame[1];

      frame[0] = halfbandIir[0].process(halfbandInput[0]);
      frame[1] = halfbandIir[1].process(halfbandInput[1]);
      out0[i] = float(frame[0]);
      out1[i] = float(frame[1]);
    } else {
      frame = processFrame({extIn0, extIn1});
      out0[i] = float(frame[0]);
      out1[i] = float(frame[1]);
    }

    prevExtIn = {extIn0, extIn1};
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto eps = std::numeric_limits<double>::epsilon();

  noteStack.push_back(info);

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  interpPitch.push(notePitch);

  velocity = velocityMap.map(info.velocity);

  if (pv[ID::resetSeedAtNoteOn]->getInt()) noiseRng.seed(pv[ID::seed]->getInt());

  impulse = velocity;
  envelopeNoise.noteOn(velocity);
  envelopeRelease.noteOn(double(1));
  envelopeRelease.setTime(1, true);
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

  if (!noteStack.empty()) {
    noteNumber = noteStack.back().noteNumber;
    interpPitch.push(calcNotePitch(noteNumber));
  } else {
    if (!pv[ID::release]->getInt()) envelopeRelease.setTime(releaseTimeSecond * upRate);
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto centerNote = double(60);
  auto pitchBendCent
    = pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble() / double(1200);
  auto cent = pv[ID::tuningCent]->getDouble() / double(100);
  auto notePitchAmount = pv[ID::notePitchAmount]->getDouble();
  return std::exp2(
    pitchBendCent + (notePitchAmount * (note - centerNote) + cent) / double(12));
}
