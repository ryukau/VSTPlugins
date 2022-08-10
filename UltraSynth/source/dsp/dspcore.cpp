// (c) 2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

inline float calcPitch(int octave, int overtone, float cent)
{
  octave -= octaveOffset;
  overtone += 1;
  return overtone * std::exp2(octave + cent / 1200.0f);
}

inline float calcPitch(float semitone, float equalTemperament = 12.0f)
{
  return std::exp2(semitone / equalTemperament);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  auto upRate = float(sampleRate) * upFold;

  constexpr auto smoothingTimeSecond = 0.2f;

  baseRateKp = float(EMAFilter<double>::secondToP(sampleRate, smoothingTimeSecond));

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(smoothingTimeSecond);

  highpass.setCutoff(float(20) / upRate);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  pitchSmoothingKp = float(                                                              \
    EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getFloat()));      \
  lowpassCutoffDecayKp = float(                                                          \
    EMAFilter<double>::secondToP(upRate, pv[ID::lowpassCutoffDecaySecond]->getFloat())); \
                                                                                         \
  interpOutputGain.METHOD(pv[ID::outputGain]->getFloat());                               \
                                                                                         \
  gainAttackKp                                                                           \
    = float(EMAFilter<double>::secondToP(upRate, pv[ID::gainAttackSecond]->getFloat())); \
  gainDecayKp                                                                            \
    = float(EMAFilter<double>::secondToP(upRate, pv[ID::gainDecaySecond]->getFloat()));  \
                                                                                         \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getFloat() * pv[ID::pitchBend]->getFloat());     \
  interpFrequencyHz.METHOD(pitchBend *(pv[ID::tuningA4Hz]->getInt() + a4HzOffset));      \
                                                                                         \
  interpOsc1FrequencyOffsetPitch.METHOD(calcPitch(                                       \
    pv[ID::osc1Octave]->getFloat(), pv[ID::osc1Overtone]->getFloat(),                    \
    pv[ID::osc1FineTuneCent]->getFloat()));                                              \
  interpOsc2FrequencyOffsetPitch.METHOD(calcPitch(                                       \
    pv[ID::osc2Octave]->getFloat(), pv[ID::osc2Overtone]->getFloat(),                    \
    pv[ID::osc2FineTuneCent]->getFloat()));                                              \
  interpOsc1WaveShape.METHOD(pv[ID::osc1WaveShape]->getFloat());                         \
  interpOsc2WaveShape.METHOD(pv[ID::osc2WaveShape]->getFloat());                         \
  interpPhaseModFromLowpassToOsc1.METHOD(pv[ID::phaseModFromLowpassToOsc1]->getFloat()); \
  interpPhaseModFromOsc1ToOsc2.METHOD(pv[ID::phaseModFromOsc1ToOsc2]->getFloat());       \
  interpPhaseModFromOsc2ToOsc1.METHOD(pv[ID::phaseModFromOsc2ToOsc1]->getFloat());       \
  interpOscMix.METHOD(pv[ID::oscMix]->getFloat());                                       \
                                                                                         \
  auto keyFollow                                                                         \
    = float(1) + pv[ID::lowpassKeyFollow]->getFloat() * (noteNumber - float(1));         \
  auto cutoff = keyFollow * pv[ID::lowpassCutoffHz]->getFloat() / upRate;                \
  interpSvfG.METHOD(SVFTool::freqToG(cutoff));                                           \
  interpSvfK.METHOD(SVFTool::qToK(pv[ID::lowpassQ]->getFloat()));                        \
  interpSvfRectMix.METHOD(pv[ID::lowpassRectification]->getFloat());                     \
                                                                                         \
  interpSaturationMix.METHOD(pv[ID::saturationMix]->getFloat());                         \
  interpSustain.METHOD(pv[ID::gainSustainAmplitude]->getFloat());                        \
                                                                                         \
  releaseEnvelope.prepare(upRate, pv[ID::gainReleaseSecond]->getFloat());                \
  svf.setSmootherSecond(upRate, pv[ID::lowpassCutoffAttackSecond]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  interpPitch.reset(float(1));

  feedback = 0;

  attackEnvelope.reset();
  decayEnvelope.reset();
  releaseEnvelope.rest();

  svf.reset();
  highpass.reset();

  firstStageLowpass.reset();
  halfBandInput.fill({});
  halfbandIir.reset();

  startup();
}

void DSPCore::startup()
{
  feedback = 0;
  phase1 = 0;
  phase2 = 0;
}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));
  SmootherCommon<float>::setSampleRate(upRate);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    constexpr auto eps = std::numeric_limits<float>::epsilon();
    if (releaseEnvelope.isResting()) {
      out0[i] = 0;
      out1[i] = 0;
      continue;
    }

    for (size_t j = 0; j < 2; ++j) {                // Halfband downsampler.
      for (size_t k = 0; k < firstStateFold; ++k) { // Stage 1 downsampler.
        auto pitch = interpPitch.process(pitchSmoothingKp);
        auto freq = interpFrequencyHz.process();
        auto osc1Freq = freq * pitch * interpOsc1FrequencyOffsetPitch.process();
        auto osc2Freq = freq * pitch * interpOsc2FrequencyOffsetPitch.process();
        auto ws1 = interpOsc1WaveShape.process();
        auto ws2 = interpOsc2WaveShape.process();
        auto oscMix = interpOscMix.process();
        auto phaseModLpToO1 = interpPhaseModFromLowpassToOsc1.process();
        auto phaseModO1ToO2 = interpPhaseModFromOsc1ToOsc2.process();
        auto phaseModO2ToO1 = interpPhaseModFromOsc2ToOsc1.process();
        auto gTarget = interpSvfG.process();
        auto kTarget = interpSvfK.process();
        auto rectMix = interpSvfRectMix.process();
        auto satMix = interpSaturationMix.process();
        auto sustain = interpSustain.process();

        // Osc1.
        phase1 += osc1Freq / upRate + phaseModLpToO1 * feedback + phaseModO2ToO1 * phase2;
        phase1 -= std::floor(phase1);
        auto p1 = ws1 - phase1;
        auto o1 = (p1 < 0 ? -p1 / (float(1) - ws1) : p1 / ws1) - float(0.5);

        // Osc2.
        phase2 += osc2Freq / upRate + phaseModO1ToO2 * o1;
        phase2 -= std::floor(phase2);
        auto p2 = ws2 - phase2;
        auto o2 = (p2 < 0 ? -p2 / (float(1) - ws2) : p2 / ws2) - float(0.5);

        // Filter.
        auto sig = svf.lowpass(
          o1 + oscMix * (o2 - o1), gTarget, kTarget, lowpassCutoffDecayKp, rectMix);
        auto sat = sig < 0 ? -std::sqrt(-sig) : std::sqrt(sig);
        sig += satMix * (sat - sig);

        // Gain.
        sig *= attackEnvelope.processKp(
          decayEnvelope.processKp(sustain, gainDecayKp) * releaseEnvelope.process(),
          gainAttackKp);

        feedback = sig;
        feedback -= std::floor(sig);

        sig = highpass.process(sig);

        firstStageLowpass.push(sig);
      }
      halfBandInput[j] = firstStageLowpass.output();
    }

    auto out = interpOutputGain.process(baseRateKp) * halfbandIir.process(halfBandInput);
    out0[i] = out;
    out1[i] = out;
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  noteNumber = calcNotePitch(info.noteNumber);

  interpPitch.push(noteNumber);

  if (pv[ID::resetPhaseAtNoteOn]->getInt()) {
    feedback = 0;
    phase1 = 0;
    phase2 = 0;
  }

  decayEnvelope.reset(float(1));
  releaseEnvelope.trigger();

  auto cutoffHz = pv[ID::lowpassCutoffHz]->getFloat();
  auto cutoffEnvAmount = pv[ID::lowpassCutoffEnvelopeAmount]->getFloat();
  auto keyFollow
    = float(1) + pv[ID::lowpassKeyFollow]->getFloat() * (noteNumber - float(1));
  svf.noteOn(
    keyFollow * cutoffHz * cutoffEnvAmount / upRate, pv[ID::lowpassQ]->getFloat());

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
    releaseEnvelope.release();
  } else {
    interpPitch.push(calcNotePitch(noteStack.back().noteNumber));
  }
}

float DSPCore::calcNotePitch(float noteNumber)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto semitone = pv[ID::tuningSemitone]->getInt() - float(semitoneOffset + 69);
  auto cent = pv[ID::tuningCent]->getFloat() / float(100);
  auto equalTemperament = pv[ID::tuningET]->getInt() + 1;
  return std::exp2((noteNumber + semitone + cent) / equalTemperament);
}
