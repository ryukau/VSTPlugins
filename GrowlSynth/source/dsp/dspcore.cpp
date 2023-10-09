// (c) 2023 Takamitsu Endo
//
// This file is part of GrowlSynth.
//
// GrowlSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrowlSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrowlSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr double defaultTempo = double(120);

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

  for (auto &x : blitFormant.lpComb) x.setup(upRate, double(1));
  modComb.setup(upRate, double(0.5), double(Scales::maxTimeSpreadSeconds.getMax()));

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  upRate = pv[ID::overSampling]->getInt() ? upFold * sampleRate : sampleRate;            \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());   \
  noteFrequency.METHOD(                                                                  \
    (pv[ID::tuningA4Hz]->getInt() + a4HzOffset) * pitchBend / upRate);                   \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  envelopeAM.METHOD(pv[ID::envelopeAM]->getDouble() * double(48000) / upRate);           \
  pulseGain.METHOD(pv[ID::pulseGain]->getDouble());                                      \
  pulsePitchOctave.METHOD(pv[ID::pulsePitchOctave]->getDouble());                        \
  pulseBendOctave.METHOD(pv[ID::pulseBendOctave]->getDouble());                          \
  pulseFormantOctave.METHOD(                                                             \
    std::exp2(pv[ID::pulseFormantOctave]->getDouble()) / upRate);                        \
  pulsePitchModMix.METHOD(pv[ID::frequencyModulationMix]->getDouble());                  \
  breathGain.METHOD(pv[ID::breathGain]->getDouble());                                    \
  breathFormantOctave.METHOD(                                                            \
    std::exp2(pv[ID::breathFormantOctave]->getDouble()) / upRate);                       \
  combFollowNote.METHOD(pv[ID::combFollowNote]->getDouble());                            \
  combFeedbackFollowEnvelope.METHOD(pv[ID::combFeedbackFollowEnvelope]->getDouble());    \
                                                                                         \
  breathNoise.METHOD(                                                                    \
    pv[ID::breathDecaySeconds]->getDouble() * upRate,                                    \
    pv[ID::breathAMLowpassCutoffHz]->getDouble() / upRate);                              \
  noteGate.prepare(upRate, pv[ID::envelopeReleaseSeconds]->getDouble());                 \
  safetyHighpass.METHOD(                                                                 \
    pv[ID::safetyHighpassHz]->getDouble() / sampleRate,                                  \
    std::numbers::sqrt2_v<double> / double(2));

#define ASSIGN_MOD_COMB_PARAMETER(METHOD)                                                \
  const auto combSamples = upRate / pv[ID::combDelayFrequencyHz]->getDouble();           \
  const auto combLowpassHz = pv[ID::combLowpassHz]->getDouble() / upRate;                \
  const auto combHighpassHz = pv[ID::combHighpassHz]->getDouble() / upRate;              \
  const auto combAllpassCut = pv[ID::combAllpassCut]->getDouble() / combSamples;         \
  const auto combAllpassMod = pv[ID::combAllpassMod]->getDouble();                       \
  const auto combAllpassQ = pv[ID::combAllpassQ]->getDouble();                           \
  const auto combEnergyLossThreshold = pv[ID::combEnergyLossThreshold]->getDouble();     \
  const auto combFeedbackGain = pv[ID::combFeedbackGain]->getDouble();                   \
  const auto combDelayTimeMod = std::exp2(pv[ID::combDelayTimeMod]->getDouble());        \
  const auto combDelayTimeSlewRate = pv[ID::combDelayTimeSlewRate]->getDouble();         \
  const auto maxTimeSpreadSamples = upRate * pv[ID::maxTimeSpreadSeconds]->getDouble();  \
  modComb.METHOD(                                                                        \
    modCombScaler, combLowpassHz, combHighpassHz, combAllpassCut, combAllpassMod,        \
    combAllpassQ, combEnergyLossThreshold, combSamples, combFeedbackGain,                \
    combDelayTimeMod, combDelayTimeSlewRate, maxTimeSpreadSamples);

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  startup();

  noteNumber = 69.0;
  velocity = 0;

  mainEnvelope.reset();
  releaseSmoother.reset();
  accumulateAM.reset();
  blitOsc.reset();
  breathFormant.reset();
  modCombScaler.reset();
  ASSIGN_MOD_COMB_PARAMETER(reset);
  noteGate.reset();
  halfbandIir.reset();
}

void DSPCore::startup() { rng.seed(0); }

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);
  ASSIGN_MOD_COMB_PARAMETER(push);
}

double DSPCore::processSample()
{
  interpPitch.process(pitchSmoothingKp);
  noteFrequency.process(pitchSmoothingKp);

  outputGain.process();
  envelopeAM.process();
  pulseGain.process();
  pulsePitchOctave.process();
  pulseBendOctave.process();
  pulsePitchModMix.process();
  pulseFormantOctave.process();
  breathGain.process();
  breathFormantOctave.process();
  combFollowNote.process();
  combFeedbackFollowEnvelope.process();

  if (noteGate.isTerminated()) return 0;

  auto envOut = mainEnvelope.process() + releaseSmoother.process();
  auto envAm = accumulateAM.process(envOut, envelopeAM.getValue(), double(1));

  auto frequency = interpPitch.getValue() * noteFrequency.getValue();
  auto freqMod
    = pulseBendOctave.getValue() * std::lerp(envAm, envOut, pulsePitchModMix.getValue());
  auto pulsePitchRatio = std::exp2(pulsePitchOctave.getValue() + freqMod);
  auto s0 = blitFormant.process(
    pulseGain.getValue() * envAm * blitOsc.process(pulsePitchRatio * frequency),
    pulseFormantOctave.getValue() * pulsePitchRatio);

  auto noise = breathFormant.process(
    envAm * breathGain.getValue() * breathNoise.process(pulsePitchRatio * frequency),
    std::exp2(pulseBendOctave.getValue() * envOut) * breathFormantOctave.getValue());

  auto fbMod = std::lerp(double(1), envOut, combFeedbackFollowEnvelope.getValue());
  auto combInvPitchRatio
    = double(1) / std::lerp(double(1), interpPitch.getValue(), combFollowNote.getValue());
  auto sig = modComb.process(s0 + noise, combInvPitchRatio, -envAm, fbMod);
  return outputGain.getValue() * noteGate.process() * sig;
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  bool overSampling = pv[ID::overSampling]->getInt();
  bool isSafetyHighpassEnabled = pv[ID::safetyHighpassEnable]->getInt();

  std::array<double, 2> halfbandInput{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (overSampling) {
      for (size_t j = 0; j < upFold; ++j) halfbandInput[j] = processSample();
      auto sig = float(halfbandIir.process(halfbandInput));
      if (isSafetyHighpassEnabled) sig = safetyHighpass.process(sig);
      out0[i] = sig;
      out1[i] = sig;
    } else {
      auto sig = float(processSample());
      if (isSafetyHighpassEnabled) sig = safetyHighpass.process(sig);
      out0[i] = sig;
      out1[i] = sig;
    }
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
  auto pitchBend
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());
  auto baseFreq = pitchBend * (pv[ID::tuningA4Hz]->getInt() + a4HzOffset) / upRate;
  if (pv[ID::slideAtNoteOn]->getInt()) {
    interpPitch.push(notePitch);
    noteFrequency.push(baseFreq);
  } else {
    interpPitch.reset(notePitch);
    noteFrequency.reset(baseFreq);
  }

  velocity = velocityMap.map(info.velocity);

  outputGain.reset(pv[ID::outputGain]->getDouble());

  releaseSmoother.prepare(
    mainEnvelope.process(), pv[ID::envelopeTransitionSeconds]->getDouble() * upRate);
  mainEnvelope.noteOn(
    velocity, pv[ID::envelopeAttackSeconds]->getDouble() * upRate,
    pv[ID::envelopeDecaySeconds]->getDouble() * upRate);

  if (noteGate.isTerminated()) {
    releaseSmoother.reset();
    accumulateAM.reset();
    noteGate.reset(double(1));

    modCombScaler.refresh(rng, pv[ID::combDelayFrequencyRandom]->getDouble());

    formantRng.seed(pv[ID::formantSeed]->getInt());
    blitFormant.refresh(formantRng);
    breathFormant.refresh(formantRng);

    ASSIGN_MOD_COMB_PARAMETER(reset);
  } else {
    noteGate.changeSustainGain(double(1));
  }
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
    noteGate.release();
  } else if (pv[ID::slideAtNoteOff]->getInt()) {
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
