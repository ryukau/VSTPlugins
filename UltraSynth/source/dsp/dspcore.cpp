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

  baseRateKp = EMAFilter<double>::secondToP(sampleRate, smoothingTimeSecond);

  SmootherCommon<double>::setSampleRate(upRate);
  SmootherCommon<double>::setTime(smoothingTimeSecond);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  lowpassCutoffDecayKp = EMAFilter<double>::secondToP(                                   \
    upRate, pv[ID::lowpassCutoffDecaySecond]->getDouble());                              \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  interpOutputGain.METHOD(pv[ID::outputGain]->getDouble());                              \
  interpLfoWaveShape.METHOD(pv[ID::lfoWaveShape]->getDouble());                          \
  interpLfoToPitch.METHOD(pv[ID::lfoToPitch]->getDouble());                              \
  interpLfoToOscMix.METHOD(pv[ID::lfoToOscMix]->getDouble());                            \
  interpLfoToCutoff.METHOD(pv[ID::lfoToCutoff]->getDouble());                            \
  interpLfoToPreSaturation.METHOD(pv[ID::lfoToPreSaturation]->getDouble());              \
  interpLfoToOsc1WaveShape.METHOD(pv[ID::lfoToOsc1WaveShape]->getDouble());              \
  interpLfoToOsc2WaveShape.METHOD(pv[ID::lfoToOsc2WaveShape]->getDouble());              \
                                                                                         \
  gainAttackKp                                                                           \
    = EMAFilter<double>::secondToP(upRate, pv[ID::gainAttackSecond]->getDouble());       \
  gainDecayKp                                                                            \
    = EMAFilter<double>::secondToP(upRate, pv[ID::gainDecaySecond]->getDouble());        \
                                                                                         \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());   \
  interpFrequencyHz.METHOD(pitchBend *(pv[ID::tuningA4Hz]->getInt() + a4HzOffset));      \
                                                                                         \
  interpOsc1FrequencyOffsetPitch.METHOD(calcOscillatorPitch(                             \
    pv[ID::osc1Octave]->getDouble(), pv[ID::osc1FineTuneCent]->getDouble()));            \
  interpOsc2FrequencyOffsetPitch.METHOD(calcOscillatorPitch(                             \
    pv[ID::osc2Octave]->getDouble(), pv[ID::osc2FineTuneCent]->getDouble()));            \
  interpOsc1WaveShape.METHOD(pv[ID::osc1WaveShape]->getDouble());                        \
  interpOsc2WaveShape.METHOD(pv[ID::osc2WaveShape]->getDouble());                        \
  interpOsc1SawPulse.METHOD(pv[ID::osc1SawPulseMix]->getDouble());                       \
  interpOsc2SawPulse.METHOD(pv[ID::osc2SawPulseMix]->getDouble());                       \
  interpPhaseModFromLowpassToOsc1.METHOD(                                                \
    pv[ID::phaseModFromLowpassToOsc1]->getDouble());                                     \
  interpPmPhase1ToPhase2.METHOD(pv[ID::pmPhase1ToPhase2]->getDouble());                  \
  interpPmPhase2ToPhase1.METHOD(pv[ID::pmPhase2ToPhase1]->getDouble());                  \
  interpPmOsc1ToPhase2.METHOD(pv[ID::pmOsc1ToPhase2]->getDouble());                      \
  interpPmOsc2ToPhase1.METHOD(pv[ID::pmOsc2ToPhase1]->getDouble());                      \
  interpOscMix.METHOD(pv[ID::oscMix]->getDouble());                                      \
                                                                                         \
  auto keyFollow                                                                         \
    = double(1) + pv[ID::lowpassKeyFollow]->getDouble() * (notePitch - double(1));       \
  auto cutoff = keyFollow * pv[ID::lowpassCutoffHz]->getDouble() / upRate;               \
  interpSvfG.METHOD(SVFTool::freqToG(cutoff));                                           \
  interpSvfK.METHOD(SVFTool::qToK(pv[ID::lowpassQ]->getDouble()));                       \
                                                                                         \
  interpRectificationMix.METHOD(pv[ID::rectificationMix]->getDouble());                  \
  interpSaturationMix.METHOD(pv[ID::saturationMix]->getDouble());                        \
  interpSustain.METHOD(pv[ID::gainSustainAmplitude]->getDouble());                       \
                                                                                         \
  releaseEnvelope.prepare(upRate, pv[ID::gainReleaseSecond]->getDouble());               \
  svf.setSmootherSecond(upRate, pv[ID::lowpassCutoffAttackSecond]->getDouble());

void DSPCore::reset()
{
  noteNumber = 69.0;
  velocity = 0;

  ASSIGN_PARAMETER(reset);

  lfoSmootherB.reset();
  lfoSmootherP.reset();

  attackEnvelope.reset();
  decayEnvelope.reset();
  releaseEnvelope.rest();

  svf.reset(interpSvfG.getValue(), interpSvfK.getValue());

  firstStageLowpass.reset();
  halfBandInput.fill({});
  halfbandIir.reset();

  startup();
}

void DSPCore::startup()
{
  lfoPhase.offset = 0;
  synchronizer.reset(sampleRate, tempo, getTempoSyncInterval());

  resetBuffer();
}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

template<typename Sample> inline Sample processOsc(Sample phase, Sample shape, Sample mix)
{
  return Sample(-0.5)
    + (phase < 0 ? lerp(-phase / (Sample(1) - shape), Sample(0), mix)
                 : lerp(phase / shape, Sample(1), mix));
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    sampleRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(),
    beatsElapsed, !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    constexpr auto eps = std::numeric_limits<double>::epsilon();
    // if (releaseEnvelope.isResting()) {
    //   out0[i] = 0;
    //   out1[i] = 0;
    //   continue;
    // }

    auto lfoShape = interpLfoWaveShape.process(baseRateKp);
    auto lfoPhi = lfoPhase.process(synchronizer.process());
    auto lfoBipolar
      = std::clamp(lfoShape * std::sin(double(twopi) * lfoPhi), double(-1), double(1));
    auto lfoPositive = double(0.25) + double(0.25) * lfoBipolar;
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
          = (double(1) + lfoToPitch * lfoB) * interpPitch.process(pitchSmoothingKp);
        auto freq = interpFrequencyHz.process();
        auto osc1Freq = freq * pitch * interpOsc1FrequencyOffsetPitch.process();
        auto osc2Freq = freq * pitch * interpOsc2FrequencyOffsetPitch.process();

        auto ws1 = std::clamp<double>(
          interpOsc1WaveShape.process() + lfoToOsc1WaveShape * lfoB, eps, 1 - eps);
        auto ws2 = std::clamp<double>(
          interpOsc2WaveShape.process() + lfoToOsc2WaveShape * lfoB, eps, 1 - eps);

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
        oscMix = std::clamp(oscMix + lfoToOscMix * double(lfoB), double(0), double(1));
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
          decayEnvelope.processKp(velocity * sustain, gainDecayKp)
            * releaseEnvelope.process(),
          gainAttackKp);

        feedback = sig;
        feedback -= std::floor(sig);

        firstStageLowpass.push(sig);
      }
      halfBandInput[j] = firstStageLowpass.output();
    }

    auto out
      = float(interpOutputGain.process(baseRateKp) * halfbandIir.process(halfBandInput));
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

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  interpPitch.push(notePitch);

  velocity = velocityMap.map(info.velocity);

  decayEnvelope.reset(velocity);
  releaseEnvelope.trigger();

  auto cutoffHz = pv[ID::lowpassCutoffHz]->getDouble();
  auto cutoffEnvAmount = pv[ID::lowpassCutoffEnvelopeAmount]->getDouble();
  auto keyFollow
    = double(1) + pv[ID::lowpassKeyFollow]->getDouble() * (notePitch - double(1));
  svf.noteOn(
    keyFollow * cutoffHz * cutoffEnvAmount / upRate, pv[ID::lowpassQ]->getDouble());

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
    noteNumber = noteStack.back().noteNumber;
    interpPitch.push(calcNotePitch(noteNumber));
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto semitone = pv[ID::tuningSemitone]->getInt() - double(semitoneOffset + 69);
  auto cent = pv[ID::tuningCent]->getDouble() / double(100);
  auto equalTemperament = pv[ID::tuningET]->getInt() + 1;
  return std::exp2((note + semitone + cent) / equalTemperament);
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

void DSPCore::resetBuffer()
{
  feedback = 0;
  phase1 = 0;
  phase2 = 0;
  o1 = 0;
  o2 = 0;
}
