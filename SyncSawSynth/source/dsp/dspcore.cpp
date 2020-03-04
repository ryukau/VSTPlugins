// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float midiNoteToFrequency(float pitch, float tuning)
{
  return 440.0f * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning) / 1200.0f);
}

float paramToPitch(float semi, float cent, float bend)
{
  return powf(2.0f, (100.0f * floorf(semi) + cent + (bend - 0.5f) * 400.0f) / 1200.0f);
}

inline float tuneFixedFreq(float value, float mod)
{
  // value in [0.01, 16.0]. value^3 ~= 5000. See scaleSync in `parameter.cpp`.
  value += 1.0f;
  return 2.0f * value * value * value * (1.0f + mod);
}

template<typename Sample>
void Note<Sample>::setup(
  int32_t noteId,
  Sample normalizedKey,
  Sample frequency,
  Sample velocity,
  Steinberg::Synth::GlobalParameter &param)
{
  state = NoteState::active;
  id = noteId;
  this->normalizedKey = normalizedKey;
  this->frequency = frequency;
  this->velocity = velocity;

  if (param.value[ParameterID::osc1PhaseLock]->getInt())
    saw1.setPhase(param.value[ParameterID::osc1Phase]->getFloat());
  if (param.value[ParameterID::osc2PhaseLock]->getInt())
    saw2.setPhase(param.value[ParameterID::osc2Phase]->getFloat());

  if (!param.value[ParameterID::filterDirty]->getInt()) {
    oscBuffer.fill(0.0f);
    filter.clear();
  }

  bypassFilter = param.value[ParameterID::filterType]->getInt() == 4;
  if (!bypassFilter) {
    switch (param.value[ParameterID::filterType]->getInt()) {
      default:
      case 0:
        filter.type = BiquadType::lowpass;
        break;

      case 1:
        filter.type = BiquadType::highpass;
        break;

      case 2:
        filter.type = BiquadType::bandpass;
        break;

      case 3:
        filter.type = BiquadType::notch;
        break;
    }
    switch (param.value[ParameterID::filterShaper]->getInt()) {
      default:
      case 0:
        filter.shaper = ShaperType::hardclip;
        break;

      case 1:
        filter.shaper = ShaperType::tanh;
        break;

      case 2:
        filter.shaper = ShaperType::sinRunge;
        break;

      case 3:
        filter.shaper = ShaperType::cubicExpDecayAbs;
        break;
    }
  }

  gainEnvelope.reset(
    param.value[ParameterID::gainA]->getFloat(),
    param.value[ParameterID::gainD]->getFloat(),
    param.value[ParameterID::gainS]->getFloat(),
    param.value[ParameterID::gainR]->getFloat());
  filterEnvelope.reset(
    param.value[ParameterID::filterA]->getFloat(),
    param.value[ParameterID::filterD]->getFloat(),
    param.value[ParameterID::filterS]->getFloat(),
    param.value[ParameterID::filterR]->getFloat());
  modEnvelope.reset(
    param.value[ParameterID::modEnvelopeA]->getFloat(),
    param.value[ParameterID::modEnvelopeCurve]->getFloat());
}

template<typename Sample> void Note<Sample>::release()
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
  gainEnvelope.release();
  filterEnvelope.release();
}

template<typename Sample> void Note<Sample>::rest() { state = NoteState::rest; }

template<typename Sample> Sample Note<Sample>::process(NoteProcessInfo<Sample> &info)
{
  if (state == NoteState::rest) return 0;

  const float modEnv = modEnvelope.process();

  saw1.setOrder(info.osc1PTROrder);
  switch (info.osc1SyncType) {
    default:
    case 0: // Off
      saw1.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq1 * modEnv * modEnv + info.modLFOToFreq1 * info.modLFO)
        * info.osc1Pitch);
      saw1.setSyncFreq(0.0f);
      break;
    case 1: { // Ratio
      saw1.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToSync1 * modEnv * modEnv + info.modLFOToSync1 * info.modLFO)
        * info.osc1Pitch * info.osc1Sync);
      saw1.setSyncFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq1 * modEnv * modEnv + info.modLFOToFreq1 * info.modLFO)
        * info.osc1Pitch);
    } break;
    case 2: // Fixed-Master
      saw1.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq1 * modEnv * modEnv + info.modLFOToFreq1 * info.modLFO)
        * info.osc1Pitch);
      saw1.setSyncFreq(tuneFixedFreq(
        info.osc1Sync,
        info.modEnvelopeToSync1 + 0.5f + 0.5f * info.modEnvelopeToSync1 * info.modLFO));
      break;
    case 3: // Fixed-Slave
      saw1.setOscFreq(tuneFixedFreq(
        info.osc1Sync,
        info.modEnvelopeToFreq1 + 0.5f + 0.5f * info.modEnvelopeToFreq1 * info.modLFO));
      saw1.setSyncFreq(
        frequency
        * (1.0f + info.modEnvelopeToSync1 * modEnv * modEnv + info.modLFOToSync1 * info.modLFO)
        * info.osc1Pitch);
      break;
  }

  saw2.setOrder(info.osc2PTROrder);
  switch (info.osc2SyncType) {
    default:
    case 0: // Off
      saw2.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq2 * modEnv * modEnv + info.modLFOToFreq2 * info.modLFO)
        * info.osc2Pitch);
      saw2.setSyncFreq(0.0f);
      break;
    case 1: // Ratio
      saw2.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToSync2 * modEnv * modEnv + info.modLFOToSync2 * info.modLFO)
        * info.osc2Pitch * info.osc2Sync);
      saw2.setSyncFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq2 * modEnv * modEnv + info.modLFOToFreq2 * info.modLFO)
        * info.osc2Pitch);
      break;
    case 2: // Fixed-Master
      saw2.setOscFreq(
        frequency
        * (1.0f + info.modEnvelopeToFreq2 * modEnv * modEnv + info.modLFOToFreq2 * info.modLFO)
        * info.osc2Pitch);
      saw2.setSyncFreq(tuneFixedFreq(
        info.osc2Sync,
        info.modEnvelopeToSync2 + 0.5f + 0.5f * info.modEnvelopeToSync2 * info.modLFO));
      break;
    case 3: // Fixed-Slave
      saw2.setOscFreq(tuneFixedFreq(
        info.osc2Sync,
        info.modEnvelopeToFreq2 + 0.5f + 0.5f * info.modEnvelopeToFreq2 * info.modLFO));
      saw2.setSyncFreq(
        frequency
        * (1.0f + info.modEnvelopeToSync2 * modEnv * modEnv + info.modLFOToSync2 * info.modLFO)
        * info.osc2Pitch);
      break;
  }

  auto toSync1 = info.fmOsc1ToSync1 * oscBuffer[0] + info.fmOsc2ToSync1 * oscBuffer[1];
  auto outSaw1 = saw1.process(0.0f, toSync1);
  auto toFreq2 = info.fmOsc1ToFreq2 * oscBuffer[0];
  auto outSaw2 = saw2.process(toFreq2, 0.0f);
  oscBuffer[0] = outSaw1;
  oscBuffer[1] = outSaw2;

  const auto gainEnv = gainEnvelope.process();
  if (gainEnvelope.isTerminated()) rest();
  gain = velocity
    * (gainEnv
       + info.gainEnvelopeCurve
         * (juce::dsp::FastMathApproximations::tanh(3.0f * info.gainEnvelopeCurve * gainEnv) - gainEnv));

  if (bypassFilter) return gain * (info.osc1Gain * outSaw1 + info.osc2Gain * outSaw2);

  filterEnv = filterEnvelope.process();
  filter.setCutoffQ(
    info.filterCutoff
      * powf(
        2.0f,
        8.0f * info.filterCutoffAmount * filterEnv
          + info.filterKeyToCutoff * normalizedKey),
    info.filterResonance + info.filterResonanceAmount * filterEnv * filterEnv);
  filter.feedback = clamp(
    info.filterFeedback + 2.0f * info.filterKeyToFeedback * normalizedKey, 0.0f, 1.0f);
  filter.saturation = info.filterSaturation;
  return gain * filter.process(info.osc1Gain * outSaw1 + info.osc2Gain * outSaw2);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (auto &note : notes) {
    for (auto &nt : note) nt = std::make_unique<Note<float>>(sampleRate);
  }

  // 2 msec + 1 sample transition time.
  transitionBuffer.resize(1 + int(sampleRate * 0.005), 0.0);

  startup();
}

void DSPCore::free() {}

void DSPCore::reset()
{
  for (auto &note : notes) {
    for (auto &nt : note) nt->rest();
  }
  startup();
}

void DSPCore::startup() { lfoPhase = 0.0f; }

void DSPCore::setParameters()
{
  interpMasterGain.push(param.value[ParameterID::gain]->getFloat());

  interpOsc1Gain.push(param.value[ParameterID::osc1Gain]->getFloat());
  interpOsc1Pitch.push(paramToPitch(
    param.value[ParameterID::osc1Semi]->getFloat(),
    param.value[ParameterID::osc1Cent]->getFloat(),
    param.value[ParameterID::pitchBend]->getFloat()));
  interpOsc1Sync.push(param.value[ParameterID::osc1Sync]->getFloat());

  interpOsc2Gain.push(
    (param.value[ParameterID::osc2Invert]->getInt() ? -1.0f : 1.0f)
    * param.value[ParameterID::osc2Gain]->getFloat());
  interpOsc2Pitch.push(paramToPitch(
    param.value[ParameterID::osc2Semi]->getFloat(),
    param.value[ParameterID::osc2Cent]->getFloat(),
    param.value[ParameterID::pitchBend]->getFloat()));
  interpOsc2Sync.push(param.value[ParameterID::osc2Sync]->getFloat());

  interpFMOsc1ToSync1.push(param.value[ParameterID::fmOsc1ToSync1]->getFloat());
  interpFMOsc1ToFreq2.push(param.value[ParameterID::fmOsc1ToFreq2]->getFloat());
  interpFMOsc2ToSync1.push(param.value[ParameterID::fmOsc2ToSync1]->getFloat());

  interpModEnvelopeToFreq1.push(param.value[ParameterID::modEnvelopeToFreq1]->getFloat());
  interpModEnvelopeToSync1.push(param.value[ParameterID::modEnvelopeToSync1]->getFloat());
  interpModEnvelopeToFreq2.push(param.value[ParameterID::modEnvelopeToFreq2]->getFloat());
  interpModEnvelopeToSync2.push(param.value[ParameterID::modEnvelopeToSync2]->getFloat());
  interpModLFOFrequency.push(param.value[ParameterID::modLFOFrequency]->getFloat());
  interpModLFONoiseMix.push(param.value[ParameterID::modLFONoiseMix]->getFloat());
  interpModLFOToFreq1.push(param.value[ParameterID::modLFOToFreq1]->getFloat());
  interpModLFOToSync1.push(param.value[ParameterID::modLFOToSync1]->getFloat());
  interpModLFOToFreq2.push(param.value[ParameterID::modLFOToFreq2]->getFloat());
  interpModLFOToSync2.push(param.value[ParameterID::modLFOToSync2]->getFloat());

  interpGainEnvelopeCurve.push(param.value[ParameterID::gainEnvelopeCurve]->getFloat());

  interpFilterCutoff.push(param.value[ParameterID::filterCutoff]->getFloat());
  interpFilterResonance.push(param.value[ParameterID::filterResonance]->getFloat());
  interpFilterFeedback.push(param.value[ParameterID::filterFeedback]->getFloat());
  interpFilterSaturation.push(param.value[ParameterID::filterSaturation]->getFloat());
  interpFilterCutoffAmount.push(param.value[ParameterID::filterCutoffAmount]->getFloat());
  interpFilterResonanceAmount.push(
    param.value[ParameterID::filterResonanceAmount]->getFloat());
  interpFilterKeyToCutoff.push(param.value[ParameterID::filterKeyToCutoff]->getFloat());
  interpFilterKeyToFeedback.push(
    param.value[ParameterID::filterKeyToFeedback]->getFloat());

  switch (param.value[ParameterID::nVoice]->getInt()) {
    case 0:
      nVoice = 1;
      break;

    case 1:
      nVoice = 2;
      break;

    case 2:
      nVoice = 4;
      break;

    case 3:
      nVoice = 8;
      break;

    case 4:
      nVoice = 16;
      break;

    default:
    case 5:
      nVoice = 32;
      break;
  }
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  bool unison = param.value[ParameterID::unison]->getFloat();
  for (auto &note : notes) {
    if (note[0]->state == NoteState::rest) continue;
    note[0]->gainEnvelope.set(
      param.value[ParameterID::gainA]->getFloat(),
      param.value[ParameterID::gainD]->getFloat(),
      param.value[ParameterID::gainS]->getFloat(),
      param.value[ParameterID::gainR]->getFloat());
    if (unison) {
      if (note[1]->state == NoteState::rest) continue;
      note[1]->gainEnvelope.set(
        param.value[ParameterID::gainA]->getFloat(),
        param.value[ParameterID::gainD]->getFloat(),
        param.value[ParameterID::gainS]->getFloat(),
        param.value[ParameterID::gainR]->getFloat());
    }
  }

  noteInfo.osc1SyncType = param.value[ParameterID::osc1SyncType]->getInt();
  noteInfo.osc1PTROrder = param.value[ParameterID::osc1PTROrder]->getInt();
  noteInfo.osc2SyncType = param.value[ParameterID::osc2SyncType]->getInt();
  noteInfo.osc2PTROrder = param.value[ParameterID::osc2PTROrder]->getInt();
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    noteInfo.osc1Gain = interpOsc1Gain.process();
    noteInfo.osc1Pitch = interpOsc1Pitch.process();
    noteInfo.osc1Sync = interpOsc1Sync.process();
    noteInfo.osc2Gain = interpOsc2Gain.process();
    noteInfo.osc2Pitch = interpOsc2Pitch.process();
    noteInfo.osc2Sync = interpOsc2Sync.process();
    noteInfo.fmOsc1ToSync1 = interpFMOsc1ToSync1.process();
    noteInfo.fmOsc1ToFreq2 = interpFMOsc1ToFreq2.process();
    noteInfo.fmOsc2ToSync1 = interpFMOsc2ToSync1.process();
    noteInfo.modEnvelopeToFreq1 = interpModEnvelopeToFreq1.process();
    noteInfo.modEnvelopeToSync1 = interpModEnvelopeToSync1.process();
    noteInfo.modEnvelopeToFreq2 = interpModEnvelopeToFreq2.process();
    noteInfo.modEnvelopeToSync2 = interpModEnvelopeToSync2.process();

    lfoPhase += 2.0 * float(pi) * interpModLFOFrequency.process() / sampleRate;
    if (lfoPhase >= float(pi)) lfoPhase -= float(pi);
    lfoValue = sinf(lfoPhase);
    // lfoValue = (lfoValue + 1.0f) * 0.5f;
    const float noiseSig = clamp(noise.process(), -1.0f, 1.0f) / 16.0f;
    noteInfo.modLFO = clamp(
      lfoValue + interpModLFONoiseMix.process() * (noiseSig - lfoValue), -1.0f, 1.0f);

    noteInfo.modLFOToFreq1 = interpModLFOToFreq1.process();
    noteInfo.modLFOToSync1 = interpModLFOToSync1.process();
    noteInfo.modLFOToFreq2 = interpModLFOToFreq2.process();
    noteInfo.modLFOToSync2 = interpModLFOToSync2.process();
    noteInfo.gainEnvelopeCurve = interpGainEnvelopeCurve.process();
    noteInfo.filterCutoff = interpFilterCutoff.process();
    noteInfo.filterResonance = interpFilterResonance.process();
    noteInfo.filterFeedback = interpFilterFeedback.process();
    noteInfo.filterSaturation = interpFilterSaturation.process();
    noteInfo.filterCutoffAmount = interpFilterCutoffAmount.process();
    noteInfo.filterResonanceAmount = interpFilterResonanceAmount.process();
    noteInfo.filterKeyToCutoff = interpFilterKeyToCutoff.process();
    noteInfo.filterKeyToFeedback = interpFilterKeyToFeedback.process();

    float sample = 0.0f;
    for (auto &note : notes) {
      if (note[0]->state == NoteState::rest) continue;
      sample += note[0]->process(noteInfo);
      if (unison) {
        if (note[1]->state == NoteState::rest) continue;
        sample += note[1]->process(noteInfo);
      }
    }

    if (isTransitioning) {
      sample += transitionBuffer[mptIndex];
      transitionBuffer[mptIndex] = 0.0f;
      mptIndex = (mptIndex + 1) % transitionBuffer.size();
      if (mptIndex == mptStop) isTransitioning = false;
    }

    const float masterGain = interpMasterGain.process();
    out0[i] = masterGain * sample;
    out1[i] = masterGain * sample;
  }
}

void DSPCore::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  size_t i = 0;
  size_t mostSilent = 0;
  float gain = 1.0f;
  for (; i < nVoice; ++i) {
    if (notes[i][0]->id == noteId) break;
    if (notes[i][0]->state == NoteState::rest) break;
    if (!notes[i][0]->gainEnvelope.isAttacking() && notes[i][0]->gain < gain) {
      gain = notes[i][0]->gain;
      mostSilent = i;
    }
  }
  if (i >= nVoice) {
    isTransitioning = true;

    i = mostSilent;

    noteInfo.osc1Gain = interpOsc1Gain.getValue();
    noteInfo.osc1Pitch = interpOsc1Pitch.getValue();
    noteInfo.osc1Sync = interpOsc1Sync.getValue();
    noteInfo.osc1SyncType = param.value[ParameterID::osc1SyncType]->getInt();
    noteInfo.osc1PTROrder = param.value[ParameterID::osc1PTROrder]->getInt();
    noteInfo.osc2Gain = interpOsc2Gain.getValue();
    noteInfo.osc2Pitch = interpOsc2Pitch.getValue();
    noteInfo.osc2Sync = interpOsc2Sync.getValue();
    noteInfo.osc2SyncType = param.value[ParameterID::osc2SyncType]->getInt();
    noteInfo.osc2PTROrder = param.value[ParameterID::osc2PTROrder]->getInt();
    noteInfo.fmOsc1ToSync1 = interpFMOsc1ToSync1.getValue();
    noteInfo.fmOsc1ToFreq2 = interpFMOsc1ToFreq2.getValue();
    noteInfo.fmOsc2ToSync1 = interpFMOsc2ToSync1.getValue();
    noteInfo.modEnvelopeToFreq1 = interpModEnvelopeToFreq1.getValue();
    noteInfo.modEnvelopeToSync1 = interpModEnvelopeToSync1.getValue();
    noteInfo.modEnvelopeToFreq2 = interpModEnvelopeToFreq2.getValue();
    noteInfo.modEnvelopeToSync2 = interpModEnvelopeToSync2.getValue();
    noteInfo.modLFO = lfoValue;
    noteInfo.modLFOToFreq1 = interpModLFOToFreq1.getValue();
    noteInfo.modLFOToSync1 = interpModLFOToSync1.getValue();
    noteInfo.modLFOToFreq2 = interpModLFOToFreq2.getValue();
    noteInfo.modLFOToSync2 = interpModLFOToSync2.getValue();
    noteInfo.gainEnvelopeCurve = interpGainEnvelopeCurve.getValue();
    noteInfo.filterCutoff = interpFilterCutoff.getValue();
    noteInfo.filterResonance = interpFilterResonance.getValue();
    noteInfo.filterFeedback = interpFilterFeedback.getValue();
    noteInfo.filterSaturation = interpFilterSaturation.getValue();
    noteInfo.filterCutoffAmount = interpFilterCutoffAmount.getValue();
    noteInfo.filterResonanceAmount = interpFilterResonanceAmount.getValue();
    noteInfo.filterKeyToCutoff = interpFilterKeyToCutoff.getValue();
    noteInfo.filterKeyToFeedback = interpFilterKeyToFeedback.getValue();

    // Beware the negative overflow. mptStop is size_t.
    mptStop = mptIndex - 1;
    if (mptStop >= transitionBuffer.size()) mptStop += transitionBuffer.size();

    for (size_t j = 0; j < transitionBuffer.size(); ++j) {
      if (notes[i][0]->state == NoteState::rest) {
        mptStop = mptIndex + j;
        if (mptStop >= transitionBuffer.size()) mptStop -= transitionBuffer.size();
        break;
      }

      float sample = notes[i][0]->process(noteInfo);
      if (
        param.value[ParameterID::unison]->getFloat()
        && notes[i][1]->state != NoteState::rest) {
        sample += notes[i][1]->process(noteInfo);
      }
      transitionBuffer[(mptIndex + j) % transitionBuffer.size()]
        += sample * (0.5 + 0.5 * cosf(pi * (float)j / transitionBuffer.size()));
    }
  }

  auto normalizedKey = float(pitch) / 127.0f;
  auto frequency = midiNoteToFrequency(pitch, tuning);
  notes[i][0]->setup(noteId, normalizedKey, frequency, velocity, param);
  if (param.value[ParameterID::unison]->getFloat()) {
    notes[i][1]->setup(noteId, normalizedKey, frequency, velocity, param);
    notes[i][1]->saw1.addPhase(0.1777);
    notes[i][1]->saw2.addPhase(0.6883f);
  } else {
    notes[i][1]->release();
  }
}

void DSPCore::noteOff(int32_t noteId)
{
  size_t i = 0;
  for (; i < notes.size(); ++i) {
    if (notes[i][0]->id == noteId) break;
  }
  if (i >= notes.size()) return;

  notes[i][0]->release();
  notes[i][1]->release();
}
