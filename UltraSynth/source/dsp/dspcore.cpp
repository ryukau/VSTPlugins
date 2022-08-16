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

constexpr float defaultTempo = float(120);

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

inline float calcOscillatorPitch(int octave, float cent)
{
  return std::exp2(octave - octaveOffset + cent / 1200.0f);
}

inline float calcPitch(float semitone, float equalTemperament = 12.0f)
{
  return std::exp2(semitone / equalTemperament);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  auto upRate = float(sampleRate) * upFold;

  synchronizer.reset(sampleRate, defaultTempo, float(1));

  constexpr auto smoothingTimeSecond = 0.2f;

  baseRateKp = float(EMAFilter<double>::secondToP(sampleRate, smoothingTimeSecond));

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(smoothingTimeSecond);

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
  interpLfoToPitch.METHOD(pv[ID::lfoToPitch]->getFloat());                               \
  interpLfoToOscMix.METHOD(pv[ID::lfoToOscMix]->getFloat());                             \
  interpLfoToCutoff.METHOD(pv[ID::lfoToCutoff]->getFloat());                             \
  interpLfoToPreSaturation.METHOD(pv[ID::lfoToPreSaturation]->getFloat());               \
  interpLfoToOsc1WaveShape.METHOD(pv[ID::lfoToOsc1WaveShape]->getFloat());               \
  interpLfoToOsc2WaveShape.METHOD(pv[ID::lfoToOsc2WaveShape]->getFloat());               \
                                                                                         \
  interpOsc1FrequencyOffsetPitch.METHOD(calcOscillatorPitch(                             \
    pv[ID::osc1Octave]->getFloat(), pv[ID::osc1FineTuneCent]->getFloat()));              \
  interpOsc2FrequencyOffsetPitch.METHOD(calcOscillatorPitch(                             \
    pv[ID::osc2Octave]->getFloat(), pv[ID::osc2FineTuneCent]->getFloat()));              \
  interpOsc1WaveShape.METHOD(pv[ID::osc1WaveShape]->getFloat());                         \
  interpOsc2WaveShape.METHOD(pv[ID::osc2WaveShape]->getFloat());                         \
  interpOsc1SawPulse.METHOD(pv[ID::osc1SawPulseMix]->getFloat());                        \
  interpOsc2SawPulse.METHOD(pv[ID::osc2SawPulseMix]->getFloat());                        \
  interpPhaseModFromLowpassToOsc1.METHOD(pv[ID::phaseModFromLowpassToOsc1]->getFloat()); \
  interpPmPhase1ToPhase2.METHOD(pv[ID::pmPhase1ToPhase2]->getFloat());                   \
  interpPmPhase2ToPhase1.METHOD(pv[ID::pmPhase2ToPhase1]->getFloat());                   \
  interpPmOsc1ToPhase2.METHOD(pv[ID::pmOsc1ToPhase2]->getFloat());                       \
  interpPmOsc2ToPhase1.METHOD(pv[ID::pmOsc2ToPhase1]->getFloat());                       \
  interpOscMix.METHOD(pv[ID::oscMix]->getFloat());                                       \
                                                                                         \
  auto keyFollow                                                                         \
    = float(1) + pv[ID::lowpassKeyFollow]->getFloat() * (noteNumber - float(1));         \
  auto cutoff = keyFollow * pv[ID::lowpassCutoffHz]->getFloat() / upRate;                \
  interpSvfG.METHOD(SVFTool::freqToG(cutoff));                                           \
  interpSvfK.METHOD(SVFTool::qToK(pv[ID::lowpassQ]->getFloat()));                        \
                                                                                         \
  interpRectificationMix.METHOD(pv[ID::rectificationMix]->getFloat());                   \
  interpSaturationMix.METHOD(pv[ID::saturationMix]->getFloat());                         \
  interpSustain.METHOD(pv[ID::gainSustainAmplitude]->getFloat());                        \
                                                                                         \
  releaseEnvelope.prepare(upRate, pv[ID::gainReleaseSecond]->getFloat());                \
  svf.setSmootherSecond(upRate, pv[ID::lowpassCutoffAttackSecond]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  interpPitch.reset(float(1));

  attackEnvelope.reset();
  decayEnvelope.reset();
  releaseEnvelope.rest();

  svf.reset();

  firstStageLowpass.reset();
  halfBandInput.fill({});
  halfbandIir.reset();

  startup();
}

void DSPCore::startup()
{
  synchronizer.reset(sampleRate, tempo, getTempoSyncInterval());

  resetBuffer();
}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

template<typename Sample> inline Sample processOsc(Sample phase, Sample shape, Sample mix)
{
  return float(-0.5)
    + (phase < 0 ? lerp(-phase / (Sample(1) - shape), Sample(0), mix)
                 : lerp(phase / shape, Sample(1), mix));
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));
  SmootherCommon<float>::setSampleRate(upRate);

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    sampleRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(),
    beatsElapsed, !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    constexpr auto eps = std::numeric_limits<float>::epsilon();
    // if (releaseEnvelope.isResting()) {
    //   out0[i] = 0;
    //   out1[i] = 0;
    //   continue;
    // }

    auto lfoPhi = lfoPhase.process(synchronizer.process());
    auto lfoBipolar = std::sin(float(twopi) * lfoPhi);
    auto lfoPositive = float(0.25) + float(0.25) * lfoBipolar;
    lfoPositive *= lfoPositive;
    lfoPositive *= lfoPositive;

    auto lfoToPitch = interpLfoToPitch.process(baseRateKp);
    auto lfoToOscMix = interpLfoToOscMix.process(baseRateKp);
    auto lfoToCutoff = interpLfoToCutoff.process(baseRateKp);
    auto lfoToPreSaturation = interpLfoToPreSaturation.process(baseRateKp);
    auto lfoToOsc1WaveShape = interpLfoToOsc1WaveShape.process(baseRateKp);
    auto lfoToOsc2WaveShape = interpLfoToOsc2WaveShape.process(baseRateKp);

    for (size_t j = 0; j < 2; ++j) {                // Halfband downsampler.
      for (size_t k = 0; k < firstStateFold; ++k) { // Stage 1 downsampler.
        auto lfoB = lfoSmootherB.processKp(lfoBipolar, 0.1);
        auto lfoP = lfoSmootherP.processKp(lfoPositive, 0.1);

        auto pitch
          = (float(1) + lfoToPitch * lfoB) * interpPitch.process(pitchSmoothingKp);
        auto freq = interpFrequencyHz.process();
        auto osc1Freq = freq * pitch * interpOsc1FrequencyOffsetPitch.process();
        auto osc2Freq = freq * pitch * interpOsc2FrequencyOffsetPitch.process();

        auto wsLfo = lfoToOsc1WaveShape * lfoB;
        auto ws1
          = std::clamp<double>(interpOsc1WaveShape.process() + wsLfo, eps, 1 - eps);
        auto ws2
          = std::clamp<double>(interpOsc2WaveShape.process() + wsLfo, eps, 1 - eps);

        auto spMix1 = interpOsc1SawPulse.process();
        auto spMix2 = interpOsc2SawPulse.process();
        auto pmLpToO1 = interpPhaseModFromLowpassToOsc1.process();
        auto pmP1ToP2 = interpPmPhase1ToPhase2.process();
        auto pmP2ToP1 = interpPmPhase2ToPhase1.process();
        auto pmO1ToP2 = interpPmOsc1ToPhase2.process();
        auto pmO2ToP1 = interpPmOsc2ToPhase1.process();
        auto oscMix = interpOscMix.process();
        auto gTarget = interpSvfG.process();
        auto kTarget = interpSvfK.process();
        auto rectMix = interpRectificationMix.process();
        auto satMix = interpSaturationMix.process();
        auto sustain = interpSustain.process();

        // Osc1.
        phase1
          += osc1Freq / upRate + pmLpToO1 * feedback + pmP1ToP2 * phase2 + pmO1ToP2 * o2;
        phase1 -= std::floor(phase1);
        o1 = processOsc<double>(ws1 - phase1, ws1, spMix1);

        // Osc2.
        phase2 += osc2Freq / upRate + pmP2ToP1 * phase1 + pmO2ToP1 * o1;
        phase2 -= std::floor(phase2);
        o2 = processOsc<double>(ws2 - phase2, ws2, spMix2);

        // Saturation.
        oscMix = std::clamp(oscMix + lfoToOscMix * float(lfoB), float(0), float(1));
        auto sig = o1 + oscMix * (o2 - o1);
        sig *= lerp<double>(1.0, lfoB, lfoToPreSaturation);
        sig = lerp<double>(sig, std::abs(sig), rectMix);
        auto sat = sig < 0 ? -std::sqrt(-sig) : std::sqrt(sig);
        sig = lerp<double>(sig, sat, satMix);

        // Filter.
        gTarget = std::clamp<double>(
          gTarget + lfoToCutoff * lfoP, SVFTool::minCutoff, SVFTool::nyquist);
        sig = svf.lowpass(sig, gTarget, kTarget, lowpassCutoffDecayKp);

        // Gain.
        sig *= attackEnvelope.processKp(
          decayEnvelope.processKp(sustain, gainDecayKp) * releaseEnvelope.process(),
          gainAttackKp);

        feedback = sig;
        feedback -= std::floor(sig);

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

  if (releaseEnvelope.isResting() && pv[ID::resetPhaseAtNoteOn]->getInt()) {
    lfoPhase.offset = pv[ID::lfoRetrigger]->getInt() ? -synchronizer.getPhase() : 0;

    resetBuffer();
  }

  noteNumber = calcNotePitch(info.noteNumber);
  interpPitch.push(noteNumber);

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

float DSPCore::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getFloat();
  if (lfoRate > Scales::lfoRate.getMax()) return 0;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto upper = pv[ID::lfoTempoUpper]->getFloat() + float(1);
  auto lower = pv[ID::lfoTempoLower]->getFloat() + float(1);
  return float(4) * upper / lower / lfoRate;
}

void DSPCore::resetBuffer()
{
  feedback = 0;
  phase1 = 0;
  phase2 = 0;
  o1 = 0;
  o2 = 0;
}
