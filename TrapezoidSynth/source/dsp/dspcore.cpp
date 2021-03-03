// (c) 2019-2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"
#include "../../../lib/juce_FastMathApproximations.h"

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float midiNoteToFrequency(float pitch, float tuning)
{
  return 440.0f * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning) / 1200.0f);
}

inline float paramToPitch(float semi, float cent, float bend)
{
  return powf(2.0f, (100.0f * floorf(semi) + cent + (bend - 0.5f) * 400.0f) / 1200.0f);
}

template<typename Sample> void TpzMono<Sample>::setup(Sample sampleRate)
{
  interpOctave.setSampleRate(sampleRate);
  interpOctave.setTime(0.001);
  interpOsc1Pitch.setSampleRate(sampleRate);
  interpOsc2Pitch.setSampleRate(sampleRate);
  tpzOsc1.sampleRate = 8 * sampleRate;
  tpzOsc2.sampleRate = 8 * sampleRate;
  lfo.setup(sampleRate);
  gainEnvelope.setup(sampleRate);
  filterEnvelope.setup(sampleRate);
  modEnvelope1.setup(sampleRate);
  modEnvelope2.setup(sampleRate);
  filter.setup(8 * sampleRate);
  shifter1.setup(sampleRate);
  shifter2.setup(sampleRate);
}

template<typename Sample> void TpzMono<Sample>::reset()
{
  decimationLP.reset();
  filter.reset();
  shifter1.reset();
  shifter2.reset();
  gainEnvelope.terminate();

  startup();
}

template<typename Sample> void TpzMono<Sample>::startup()
{
  feedbackBuffer = 0;
  tpzOsc2Buffer = 0;
  tpzOsc1.reset();
  tpzOsc2.reset();
  rngPitchDrift.seed = rngPitchDriftSeed;
  lfo.reset();
}

template<typename Sample>
void TpzMono<Sample>::setParameters(Sample tempo, PlugParameter &param)
{
  interpOctave.push(getOctave(param));
  interpOsc1Pitch.setTime(param.value[ParameterID::pitchSlide]->getFloat());
  interpOsc1Pitch.push(getOsc1Pitch(param));
  interpOsc2Pitch.setTime(
    param.value[ParameterID::pitchSlide]->getFloat()
    * param.value[ParameterID::pitchSlideOffset]->getFloat());
  interpOsc2Pitch.push(getOsc2Pitch(param));

  interpOsc1Slope.push(param.value[ParameterID::osc1Slope]->getFloat());
  interpOsc1PulseWidth.push(param.value[ParameterID::osc1PulseWidth]->getFloat());
  interpOsc2Slope.push(param.value[ParameterID::osc2Slope]->getFloat());
  interpOsc2PulseWidth.push(param.value[ParameterID::osc2PulseWidth]->getFloat());
  interpOscMix.push(param.value[ParameterID::oscMix]->getFloat());
  interpPitchDrift.push(param.value[ParameterID::osc1PitchDrift]->getFloat());
  interpPhaseMod.push(param.value[ParameterID::pmOsc2ToOsc1]->getFloat());
  interpFeedback.push(param.value[ParameterID::osc1Feedback]->getFloat());
  interpFilterCutoff.push(param.value[ParameterID::filterCutoff]->getFloat());
  interpFilterFeedback.push(param.value[ParameterID::filterFeedback]->getFloat());
  interpFilterSaturation.push(param.value[ParameterID::filterSaturation]->getFloat());
  interpFilterEnvToCutoff.push(param.value[ParameterID::filterEnvToCutoff]->getFloat());
  interpFilterKeyToCutoff.push(param.value[ParameterID::filterKeyToCutoff]->getFloat());
  interpOscMixToFilterCutoff.push(
    param.value[ParameterID::oscMixToFilterCutoff]->getFloat());
  interpMod1EnvToPhaseMod.push(param.value[ParameterID::modEnv1ToPhaseMod]->getFloat());
  interpMod2EnvToFeedback.push(param.value[ParameterID::modEnv2ToFeedback]->getFloat());
  interpMod2EnvToLFOFrequency.push(
    param.value[ParameterID::modEnv2ToLFOFrequency]->getFloat());
  interpModEnv2ToOsc2Slope.push(param.value[ParameterID::modEnv2ToOsc2Slope]->getFloat());
  interpMod2EnvToShifter1.push(param.value[ParameterID::modEnv2ToShifter1]->getFloat());
  interpLFOShape.push(param.value[ParameterID::lfoShape]->getFloat());
  interpLFOToPitch.push(param.value[ParameterID::lfoToPitch]->getFloat());
  interpLFOToSlope.push(param.value[ParameterID::lfoToSlope]->getFloat());
  interpLFOToPulseWidth.push(param.value[ParameterID::lfoToPulseWidth]->getFloat());
  interpLFOToCutoff.push(param.value[ParameterID::lfoToCutoff]->getFloat());

  // shiftHz = freq * shifterPitch - freq.
  interpShifter1Pitch.push(
    paramToPitch(
      param.value[ParameterID::shifter1Semi]->getFloat(),
      param.value[ParameterID::shifter1Cent]->getFloat(), 0.5f)
    - Sample(1));
  interpShifter1Gain.push(param.value[ParameterID::shifter1Gain]->getFloat());
  interpShifter2Pitch.push(
    paramToPitch(
      param.value[ParameterID::shifter2Semi]->getFloat(),
      param.value[ParameterID::shifter2Cent]->getFloat(), 0.5f)
    - Sample(1));
  interpShifter2Gain.push(param.value[ParameterID::shifter2Gain]->getFloat());

  float lfoFreq = param.value[ParameterID::lfoFrequency]->getFloat();
  // tempo / 60 is Hz for a 1/4 beat.
  if (param.value[ParameterID::lfoTempoSync]->getInt()) {
    lfoFreq = lfoFreq * tempo / 240.0f;
  }
  interpLFOFrequency.push(lfoFreq);

  filter.setOrder(param.value[ParameterID::filterOrder]->getInt());

  switch (param.value[ParameterID::lfoType]->getInt()) {
    default:
    case 0: // sin
      lfo.type = LFOType::sin;
      break;

    case 1: // saw
      lfo.type = LFOType::saw;
      break;

    case 2: // pulse
      lfo.type = LFOType::pulse;
      break;

    case 3: // noise
      lfo.type = LFOType::noise;
      break;
  }

  gainEnvelope.set(
    param.value[ParameterID::gainA]->getFloat(),
    param.value[ParameterID::gainD]->getFloat(),
    param.value[ParameterID::gainS]->getFloat(),
    param.value[ParameterID::gainR]->getFloat(),
    param.value[ParameterID::gainCurve]->getFloat());
  filterEnvelope.set(
    param.value[ParameterID::filterA]->getFloat(),
    param.value[ParameterID::filterD]->getFloat(),
    param.value[ParameterID::filterS]->getFloat(),
    param.value[ParameterID::filterR]->getFloat(),
    param.value[ParameterID::filterCurve]->getFloat());
}

template<typename Sample>
void TpzMono<Sample>::noteOn(
  bool wasResting, Sample frequency, Sample normalizedKey, PlugParameter &param)
{
  noteFreq = frequency;

  int32_t pitchSlideType
    = wasResting ? param.value[ParameterID::pitchSlideType]->getInt() : -1;
  switch (pitchSlideType) {
    case 1: { // Sustain
      interpOctave.reset(getOctave(param));
      interpOsc1Pitch.reset(getOsc1Pitch(param));
      interpOsc2Pitch.reset(getOsc2Pitch(param));
    } break;

    case 2: // Reset to 0
      interpOsc1Pitch.reset(0);
      interpOsc2Pitch.reset(0);
      break;

    default:
      break;
  }

  this->normalizedKey = normalizedKey;
  if (
    param.value[ParameterID::gainEnvRetrigger]->getInt() || gainEnvelope.isTerminated()
    || gainEnvelope.isReleasing()) {
    gainEnvelope.reset(
      param.value[ParameterID::gainA]->getFloat(),
      param.value[ParameterID::gainD]->getFloat(),
      param.value[ParameterID::gainS]->getFloat(),
      param.value[ParameterID::gainR]->getFloat(),
      param.value[ParameterID::gainCurve]->getFloat());
  }

  if (
    param.value[ParameterID::filterEnvRetrigger]->getInt()
    || filterEnvelope.isTerminated() || filterEnvelope.isReleasing()) {
    filterEnvelope.reset(
      param.value[ParameterID::filterA]->getFloat(),
      param.value[ParameterID::filterD]->getFloat(),
      param.value[ParameterID::filterS]->getFloat(),
      param.value[ParameterID::filterR]->getFloat(),
      param.value[ParameterID::filterCurve]->getFloat());
  }

  if (param.value[ParameterID::modEnv1Retrigger]->getInt() || wasResting)
    modEnvelope1.reset(
      param.value[ParameterID::modEnv1Attack]->getFloat(),
      param.value[ParameterID::modEnv1Curve]->getFloat());

  if (param.value[ParameterID::modEnv2Retrigger]->getInt() || wasResting)
    modEnvelope2.reset(
      param.value[ParameterID::modEnv2Attack]->getFloat(),
      param.value[ParameterID::modEnv2Curve]->getFloat());
}

template<typename Sample> void TpzMono<Sample>::noteOff(Sample frequency)
{
  noteFreq = frequency;
}

template<typename Sample> void TpzMono<Sample>::release(bool resetPitch)
{
  if (resetPitch) noteFreq = 0;

  gainEnvelope.release();
  filterEnvelope.release();
}

template<typename Sample> Sample TpzMono<Sample>::process(const size_t bufferSize)
{
  if (gainEnvelope.isTerminated()) return 0;

  const auto modEnv2Sig = modEnvelope2.process();
  lfo.setFreq(
    interpLFOFrequency.process() + modEnv2Sig * interpMod2EnvToLFOFrequency.process());
  lfo.pw = interpLFOShape.process();
  const auto lfoSig = lfo.process();

  const auto filterEnv = filterEnvelope.process()
    + interpOscMixToFilterCutoff.process() * (1.0f + feedbackBuffer);
  const auto cutoff = interpFilterCutoff.process()
    + interpFilterKeyToCutoff.process() * noteFreq
    + Sample(19800)
      * (interpLFOToCutoff.process() * lfoSig + interpFilterEnvToCutoff.process() * filterEnv);
  filter.setCutoff(clamp(cutoff, Sample(20), Sample(8 * 20000)));
  filter.feedback = interpFilterFeedback.process();
  filter.saturation = interpFilterSaturation.process();

  const auto octave = interpOctave.process();
  tpzOsc1.setFreq(
    octave
    * interpOsc1Pitch.process()
    * (1.0f
      + lfoSig * interpLFOToPitch.process()
      + interpPitchDrift.process() * rngPitchDrift.process()));
  tpzOsc1.setSlope(interpOsc1Slope.process() + lfoSig * interpLFOToSlope.process());
  tpzOsc1.setPulseWidth(
    interpOsc1PulseWidth.process() + lfoSig * interpLFOToPulseWidth.process());

  const auto osc2Freq = octave * interpOsc2Pitch.process();
  tpzOsc2.setFreq(osc2Freq);
  tpzOsc2.setSlope(
    interpOsc2Slope.process() + +modEnv2Sig * interpModEnv2ToOsc2Slope.process());
  tpzOsc2.setPulseWidth(interpOsc2PulseWidth.process());

  const auto oscMix = interpOscMix.process();
  const auto modEnv1Sig = modEnvelope1.process();
  tpzOsc1.addPhase(
    feedbackBuffer
      * (interpMod2EnvToFeedback.process() * modEnv2Sig + interpFeedback.process())
    + tpzOsc2Buffer
      * (interpMod1EnvToPhaseMod.process() * modEnv1Sig + interpPhaseMod.process()));
  for (size_t i = 0; i < 8; ++i) {
    tpzOsc2Buffer = tpzOsc2.process();
    const float osc1Sig = tpzOsc1.process();
    feedbackBuffer = decimationLP.process(
      filter.process(osc1Sig + oscMix * (tpzOsc2Buffer - osc1Sig)));
  }

  shifter1.setShift(
    osc2Freq * interpShifter1Pitch.process()
    + modEnv2Sig * interpMod2EnvToShifter1.process());
  shifter2.setShift(osc2Freq * interpShifter2Pitch.process());

  return gainEnvelope.process()
    * (feedbackBuffer + interpShifter1Gain.process() * shifter1.process(feedbackBuffer)
       + interpShifter2Gain.process() * shifter2.process(feedbackBuffer));
}

template<typename Sample> Sample TpzMono<Sample>::getOctave(PlugParameter &param)
{
  int32_t index = 4
    + int32_t(std::floor(
      param.value[ParameterID::octave]->getFloat()
      + filterEnvelope.getValue()
        * param.value[ParameterID::filterEnvToOctave]->getFloat()));
  if (index < 0) index = 0;
  if (size_t(index) >= octaveTable.size()) index = int32_t(octaveTable.size()) - 1;
  return octaveTable[index];
}

template<typename Sample> Sample TpzMono<Sample>::getOsc1Pitch(PlugParameter &param)
{
  return noteFreq
    * paramToPitch(
           param.value[ParameterID::osc1Semi]->getFloat(),
           param.value[ParameterID::osc1Cent]->getFloat(),
           param.value[ParameterID::pitchBend]->getFloat());
}

template<typename Sample> Sample TpzMono<Sample>::getOsc2Pitch(PlugParameter &param)
{
  return noteFreq * param.value[ParameterID::osc2Overtone]->getInt()
    * paramToPitch(
           param.value[ParameterID::osc2Semi]->getFloat(),
           param.value[ParameterID::osc2Cent]->getFloat(),
           param.value[ParameterID::pitchBend]->getFloat());
}

DSPCore::DSPCore() { midiNotes.reserve(128); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  midiNotes.resize(0);

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.01f);

  noteStack.reserve(128);
  noteStack.resize(0);

  tpz1.setup(sampleRate);

  interpMasterGain.reset(param.value[ParameterID::gain]->getFloat());

  reset();
  startup();
}

void DSPCore::free() {}

void DSPCore::reset()
{
  tpz1.reset();
  startup();
}

void DSPCore::startup() { tpz1.startup(); }

void DSPCore::setParameters(double tempo)
{
  SmootherCommon<float>::setTime(param.value[ParameterID::smoothness]->getFloat());

  interpMasterGain.push(velocity * param.value[ParameterID::gain]->getFloat());

  tpz1.setParameters(tempo, param);
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  float sample = 1.0;
  for (uint32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    sample = tpz1.process(length);
    const float masterGain = interpMasterGain.process();
    out0[i] = masterGain * sample;
    out1[i] = masterGain * sample;
  }
}

void DSPCore::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  NoteInfo info;
  info.id = noteId;
  info.frequency = midiNoteToFrequency(pitch, tuning);
  noteStack.push_back(info);

  this->velocity = velocity;

  tpz1.noteOn(
    noteStack.size() == 1, noteStack.back().frequency, float(pitch) / 127.0f, param);
}

void DSPCore::noteOff(int32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    if (param.value[ParameterID::pitchSlideType]->getInt() == 2) {
      tpz1.release(true);
    } else {
      tpz1.release(false);
    }
  } else {
    tpz1.noteOff(noteStack.back().frequency);
  }
}
