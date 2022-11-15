// (c) 2022 Takamitsu Endo
//
// This file is part of MembraneSynth.
//
// MembraneSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MembraneSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MembraneSynth.  If not, see <https://www.gnu.org/licenses/>.

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

  fdn.setup(upRate, 1.0);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  upRate = pv[ID::overSampling]->getInt() ? upFold * sampleRate : sampleRate;            \
                                                                                         \
  enableModEnv = pv[ID::enableModEnv]->getInt();                                         \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble() / pulseAmp);                         \
                                                                                         \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());   \
  frequencyHz.METHOD(pitchBend *(pv[ID::tuningA4Hz]->getInt() + a4HzOffset));            \
                                                                                         \
  fdnShape.METHOD(pv[ID::fdnShape]->getDouble());                                        \
  fdnFeedback.METHOD(pv[ID::fdnFeedback]->getDouble());                                  \
  fdnModulation.METHOD(pv[ID::fdnModulation]->getDouble());                              \
  fdnInterpRate.METHOD(pv[ID::fdnInterpRate]->getDouble());                              \
  fdnMinModulation.METHOD(double(1) - pv[ID::fdnMaxModulation]->getDouble());            \
                                                                                         \
  for (size_t idx = 0; idx < fdnSize; ++idx) {                                           \
    fdn.lowpass.METHOD##CutoffAt(                                                        \
      idx, pv[ID::fdnLowpassCutoffHz]->getDouble() / upRate,                             \
      pv[ID::fdnLowpassQ0 + idx]->getDouble());                                          \
    fdn.highpass.METHOD##CutoffAt(                                                       \
      idx, pv[ID::fdnHighpassCutoffHz]->getDouble() / upRate,                            \
      pv[ID::fdnHighpassQ0 + idx]->getDouble());                                         \
  }

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  previousSeed = pv[ID::fdnSeed]->getInt();
  rng.seed(previousSeed);

  std::normal_distribution<float> dist{}; // mean 0, stddev 1.
  for (auto &row : fdnMatrixRandomBase) {
    for (auto &value : row) value = dist(rng);
  }

  noteNumber = 69.0;
  velocity = 0;

  pulse.reset();
  modulationEnvelope.reset();
  fdn.reset();
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

    std::normal_distribution<float> dist{};
    for (auto &row : fdnMatrixRandomBase) {
      for (auto &value : row) value = dist(rng);
    }
  }
}

double DSPCore::processSample()
{
  interpPitch.process(pitchSmoothingKp);
  frequencyHz.process(pitchSmoothingKp);

  outputGain.process();
  fdnShape.process();
  fdnFeedback.process();
  fdnModulation.process();
  fdnInterpRate.process();
  fdnMinModulation.process();

  auto modEnv = enableModEnv ? modulationEnvelope.process() : double(1);

  auto sig = pulse.process();

  for (size_t idx = 0; idx < fdnSize; ++idx) {
    auto overtone = lerp(double(idx + 1), circularMode[idx], fdnShape.getValue())
      + fdnAdditionalOvertone[idx];
    fdn.delay.setDelayTimeAt(
      idx, upRate, overtone, interpPitch.getValue() * frequencyHz.getValue());
  }
  sig = fdn.process(
    sig, fdnFeedback.getValue(), fdnModulation.getValue(),
    modEnv * fdnInterpRate.getValue(), fdnMinModulation.getValue());

  return sig * outputGain.getValue();
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
  if (pv[ID::slideAtNoteOn]->getInt()) {
    interpPitch.push(notePitch);
    frequencyHz.push(baseFreq);
  } else {
    interpPitch.reset(notePitch);
    frequencyHz.reset(baseFreq);
  }

  velocity = velocityMap.map(info.velocity);

  pulseAmp = pv[ID::gainNormalization]->getInt() ? pv[ID::pulseAmplitude]->getDouble()
                                                 : double(1);
  if (pulseAmp < eps) pulseAmp = eps;
  outputGain.reset(pv[ID::outputGain]->getDouble() / pulseAmp);

  pulse.noteOn(
    velocity * pv[ID::pulseAmplitude]->getDouble(),
    upRate * pv[ID::pulseDecaySeconds]->getDouble());

  modulationEnvelope.noteOn(
    upRate, pv[ID::modEnvSustainSeconds]->getDouble(),
    pv[ID::modEnvReleaseSeconds]->getDouble());

  std::uniform_int_distribution<unsigned> seedDist{
    0, std::numeric_limits<unsigned>::max()};
  fdn.randomOrthogonal(
    seedDist(rng), pv[ID::fdnMatrixIdentityAmount]->getDouble(),
    pv[ID::fdnRandomMatrix]->getDouble(), fdnMatrixRandomBase);

  auto impactPosition = pv[ID::impactCenterRimPosition]->getDouble();
  auto start = double(1) + impactPosition;
  auto slope = double(-2) * impactPosition / (double(fdnSize - 1));
  for (size_t i = 0; i < fdnSize; ++i) {
    auto gain = start + i * slope;
    fdn.inputGain[i] = gain * gain;
  }

  auto fdnRandomOvertone = pv[ID::fdnRandomOvertone]->getDouble();
  std::uniform_real_distribution<double> overtoneDist{0.0, 1.0};
  for (size_t idx = 0; idx < fdnSize; ++idx) {
    fdnAdditionalOvertone[idx] = fdnRandomOvertone * overtoneDist(rng);

    auto overtone = lerp(double(idx + 1), circularMode[idx], fdnShape.getValue())
      + fdnAdditionalOvertone[idx];
    fdn.delay.resetDelayTimeAt(
      idx, upRate, overtone, interpPitch.getValue() * frequencyHz.getValue());
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
