// (c) 2022 Takamitsu Endo
//
// This file is part of MaybeSnare.
//
// MaybeSnare is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MaybeSnare is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MaybeSnare.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr double defaultTempo = double(120);

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

inline double calcOscillatorPitch(double octave, double cent)
{
  return std::exp2(octave - octaveOffset + cent / 1200.0);
}

inline double calcPitch(double semitone, double equalTemperament = 12.0)
{
  return std::exp2(semitone / equalTemperament);
}

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = sampleRate;
  upRate = sampleRate * upFold;

  constexpr auto smoothingTimeSecond = 0.2;

  SmootherCommon<double>::setSampleRate(upRate);
  SmootherCommon<double>::setTime(smoothingTimeSecond);

  batterSide.setup(upRate, 1.0);
  snareSide.setup(upRate, 1.0);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  upRate = pv[ID::overSampling]->getInt() ? upFold * sampleRate : sampleRate;            \
                                                                                         \
  enableBatterModEnv = pv[ID::enableBatterModEnv]->getInt();                             \
  enableSnareModEnv = pv[ID::enableSnareModEnv]->getInt();                               \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  snareSidePitch.push(std::exp2(pv[ID::snareSideOctaveOffset]->getDouble()));            \
                                                                                         \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());   \
  frequencyHz.METHOD(pitchBend *(pv[ID::tuningA4Hz]->getInt() + a4HzOffset));            \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble() / pulseAmp);                         \
  fdnMix.METHOD(pv[ID::fdnMix]->getDouble());                                            \
  impactNoiseMix.METHOD(pv[ID::impactNoiseMix]->getDouble());                            \
  couplingAmount.METHOD(pv[ID::couplingAmount]->getDouble());                            \
  couplingSafetyReduction.METHOD(pv[ID::couplingSafetyReduction]->getDouble());          \
                                                                                         \
  batterShape.METHOD(pv[ID::batterFdnShape]->getDouble());                               \
  batterFeedback.METHOD(pv[ID::batterFdnFeedback]->getDouble());                         \
  batterModulation.METHOD(pv[ID::batterFdnModulationAmount]->getDouble());               \
  batterInterpRate.METHOD(pv[ID::batterFdnInterpRate]->getDouble());                     \
  batterMinModulation.METHOD(                                                            \
    double(1) - pv[ID::batterFdnMaxModulationRatio]->getDouble());                       \
  for (size_t idx = 0; idx < fdnSize; ++idx) {                                           \
    batterSide.lowpass.METHOD##CutoffAt(                                                 \
      idx, pv[ID::batterFdnLowpassCutoffHz]->getDouble() / upRate,                       \
      pv[ID::batterFdnLowpassQ0 + idx]->getDouble());                                    \
    batterSide.highpass.METHOD##CutoffAt(                                                \
      idx, pv[ID::batterFdnHighpassCutoffHz]->getDouble() / upRate,                      \
      pv[ID::batterFdnHighpassQ0 + idx]->getDouble());                                   \
  }                                                                                      \
                                                                                         \
  snareShape.METHOD(pv[ID::snareFdnShape]->getDouble());                                 \
  snareFeedback.METHOD(pv[ID::snareFdnFeedback]->getDouble());                           \
  snareModulation.METHOD(pv[ID::snareFdnModulationAmount]->getDouble());                 \
  snareInterpRate.METHOD(pv[ID::snareFdnInterpRate]->getDouble());                       \
  snareMinModulation.METHOD(                                                             \
    double(1) - pv[ID::snareFdnMaxModulationRatio]->getDouble());                        \
  for (size_t idx = 0; idx < fdnSize; ++idx) {                                           \
    snareSide.lowpass.METHOD##CutoffAt(                                                  \
      idx, pv[ID::snareFdnLowpassCutoffHz]->getDouble() / upRate,                        \
      pv[ID::snareFdnLowpassQ0 + idx]->getDouble());                                     \
    snareSide.highpass.METHOD##CutoffAt(                                                 \
      idx, pv[ID::snareFdnHighpassCutoffHz]->getDouble() / upRate,                       \
      pv[ID::snareFdnHighpassQ0 + idx]->getDouble());                                    \
  }

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  previousSeed = pv[ID::fdnSeed]->getInt();
  rng.seed(previousSeed);

  std::normal_distribution<float> dist{}; // mean 0, stddev 1.
  for (auto &row : batterFdnMatrixRandomBase) {
    for (auto &value : row) value = dist(rng);
  }
  for (auto &row : snareFdnMatrixRandomBase) {
    for (auto &value : row) value = dist(rng);
  }

  noteNumber = 69.0;
  velocity = 0;

  bufBatter = 0;
  bufSnare = 0;
  couplingEnvelope = 0;
  couplingDecay = 0;
  pulse.reset();
  batterModEnvelope.reset();
  snareModEnvelope.reset();
  batterSide.reset();
  snareSide.reset();
  halfbandIir.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);

  auto seed = pv[ID::fdnSeed]->getInt();
  if (previousSeed != seed) {
    previousSeed = seed;
    rng.seed(previousSeed);

    std::normal_distribution<float> dist{};
    for (auto &row : batterFdnMatrixRandomBase) {
      for (auto &value : row) value = dist(rng);
    }
    for (auto &row : snareFdnMatrixRandomBase) {
      for (auto &value : row) value = dist(rng);
    }
  }
}

double DSPCore::processSample()
{
  interpPitch.process(pitchSmoothingKp);
  snareSidePitch.process(pitchSmoothingKp);
  frequencyHz.process(pitchSmoothingKp);

  outputGain.process();
  fdnMix.process();
  impactNoiseMix.process();
  couplingAmount.process();
  couplingSafetyReduction.process();
  batterShape.process();
  batterFeedback.process();
  batterModulation.process();
  batterInterpRate.process();
  batterMinModulation.process();
  snareShape.process();
  snareFeedback.process();
  snareModulation.process();
  snareInterpRate.process();
  snareMinModulation.process();

  auto batterModEnv = enableBatterModEnv ? batterModEnvelope.process() : double(1);
  auto snareModEnv = enableSnareModEnv ? snareModEnvelope.process() : double(1);

  auto pulseOut = pulse.process(rng, impactNoiseMix.getValue());

  for (size_t idx = 0; idx < fdnSize; ++idx) {
    auto batterOvertone = lerp(double(idx + 1), circularMode[idx], batterShape.getValue())
      + fdnAdditionalOvertone[idx];
    batterSide.delay.setDelayTimeAt(
      idx, upRate, batterOvertone, interpPitch.getValue() * frequencyHz.getValue());

    auto snareOvertone = lerp(double(idx + 1), circularMode[idx], snareShape.getValue())
      + fdnAdditionalOvertone[idx];
    snareSide.delay.setDelayTimeAt(
      idx, upRate, snareOvertone,
      interpPitch.getValue() * frequencyHz.getValue() * snareSidePitch.getValue());
  }

  auto batterOut = batterSide.process(
    pulseOut + bufBatter, batterFeedback.getValue(),
    batterModEnv * batterModulation.getValue(), batterInterpRate.getValue(),
    batterMinModulation.getValue());
  auto snareOut = snareSide.process(
    bufSnare, snareFeedback.getValue(), snareModEnv * snareModulation.getValue(),
    snareInterpRate.getValue(), snareMinModulation.getValue());

  auto cpl = couplingEnvelope * couplingAmount.getValue();
  bufBatter = std::clamp(cpl * snareOut, double(-1000), double(1000));
  bufSnare = std::clamp(-cpl * batterOut, double(-1000), double(1000));

  couplingEnvelope *= couplingDecay;
  auto bufPeak = std::max(std::abs(bufBatter), std::abs(bufSnare));
  if (bufPeak > double(1)) {
    couplingEnvelope *= couplingSafetyReduction.getValue();
  }

  return lerp(batterOut, snareOut, fdnMix.getValue()) * outputGain.getValue();
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  bool overSampling = pv[ID::overSampling]->getInt();

  std::array<double, 2> halfbandInput{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (overSampling) {
      for (size_t j = 0; j < upFold; ++j) halfbandInput[j] = processSample();
      auto output = float(halfbandIir.process(halfbandInput));
      out0[i] = output;
      out1[i] = output;
    } else {
      auto output = float(processSample());
      out0[i] = output;
      out1[i] = output;
    }
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto eps = std::numeric_limits<double>::epsilon();

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  auto pitchBend
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());
  auto baseFreq = pitchBend * (pv[ID::tuningA4Hz]->getInt() + a4HzOffset);
  auto snareOffsetPitch = std::exp2(pv[ID::snareSideOctaveOffset]->getDouble());
  if (pv[ID::slideAtNoteOn]->getInt()) {
    interpPitch.push(notePitch);
    snareSidePitch.push(snareOffsetPitch);
    frequencyHz.push(baseFreq);
  } else {
    interpPitch.reset(notePitch);
    snareSidePitch.reset(snareOffsetPitch);
    frequencyHz.reset(baseFreq);
  }

  velocity = velocityMap.map(info.velocity);

  if (pv[ID::gainNormalization]->getInt()) {
    pulseAmp = pv[ID::impactAmplitude]->getDouble()
      * std::max(pv[ID::couplingAmount]->getDouble(), double(1));
  } else {
    pulseAmp = double(1);
  }
  if (pulseAmp < eps) pulseAmp = eps;
  outputGain.reset(pv[ID::outputGain]->getDouble() / pulseAmp);

  couplingEnvelope = pv[ID::couplingAmount]->getDouble() / double(2);
  auto couplingDecaySamples = upRate * pv[ID::couplingDecaySeconds]->getDouble()
    * velocityToCouplingDecayMap.map(info.velocity);
  couplingDecay = std::pow(
    double(std::numeric_limits<float>::epsilon()), double(1) / couplingDecaySamples);

  pulse.noteOn(
    upRate, velocity * pv[ID::impactAmplitude]->getDouble(),
    upRate * pv[ID::impactDecaySeconds]->getDouble(),
    upRate * pv[ID::impactNoiseDecaySeconds]->getDouble(),
    pv[ID::impactNoiseLowpassHz]->getDouble());

  batterModEnvelope.noteOn(
    upRate, pv[ID::batterModEnvSustainSeconds]->getDouble(),
    pv[ID::batterModEnvReleaseSeconds]->getDouble());
  snareModEnvelope.noteOn(
    upRate, pv[ID::snareModEnvSustainSeconds]->getDouble(),
    pv[ID::snareModEnvReleaseSeconds]->getDouble());

  // FDN feedback matrix.
  std::uniform_int_distribution<unsigned> seedDist{
    0, std::numeric_limits<unsigned>::max()};
  batterSide.randomOrthogonal(
    seedDist(rng), pv[ID::batterFdnMatrixIdentityAmount]->getDouble(),
    pv[ID::fdnRandomMatrix]->getDouble(), batterFdnMatrixRandomBase);
  snareSide.randomOrthogonal(
    seedDist(rng), pv[ID::snareFdnMatrixIdentityAmount]->getDouble(),
    pv[ID::fdnRandomMatrix]->getDouble(), snareFdnMatrixRandomBase);

  // FDN feedback gain bias.
  auto impactPosition = pv[ID::impactCenterRimPosition]->getDouble();
  auto start = double(1) + impactPosition;
  auto slope = double(-2) * impactPosition / (double(fdnSize - 1));
  for (size_t i = 0; i < fdnSize; ++i) {
    auto gain = start + i * slope;
    batterSide.inputGain[i] = gain * gain;
  }

  // FDN delay time (pitch).
  auto fdnRandomOvertone = pv[ID::fdnRandomOvertone]->getDouble();
  std::uniform_real_distribution<double> overtoneDist{0.0, 1.0};
  for (size_t idx = 0; idx < fdnSize; ++idx) {
    fdnAdditionalOvertone[idx] = fdnRandomOvertone * overtoneDist(rng);

    auto batterOvertone = lerp(double(idx + 1), circularMode[idx], batterShape.getValue())
      + fdnAdditionalOvertone[idx];
    batterSide.delay.resetDelayTimeAt(
      idx, upRate, batterOvertone, interpPitch.getValue() * frequencyHz.getValue());

    auto snareOvertone = lerp(double(idx + 1), circularMode[idx], snareShape.getValue())
      + fdnAdditionalOvertone[idx];
    snareSide.delay.resetDelayTimeAt(
      idx, upRate, snareOvertone,
      interpPitch.getValue() * frequencyHz.getValue() * snareSidePitch.getValue());
  }

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

  if (!noteStack.empty() && pv[ID::slideAtNoteOff]->getInt()) {
    noteNumber = noteStack.back().noteNumber;
    interpPitch.push(calcNotePitch(noteNumber));
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto semitone = pv[ID::tuningSemitone]->getInt() - double(semitoneOffset + 57);
  auto cent = pv[ID::tuningCent]->getDouble() / double(100);
  auto equalTemperament = pv[ID::tuningET]->getInt() + 1;
  return std::exp2((note + semitone + cent) / equalTemperament);
}
