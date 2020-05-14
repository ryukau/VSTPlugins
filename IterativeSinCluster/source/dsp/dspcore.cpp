// (c) 2019-2020 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"
#include "../../../lib/juce_FastMathApproximations.h"
#include "../../../lib/vcl/vectorclass.h"

#if INSTRSET >= 10
#define NOTE_NAME Note_AVX512
#define DSPCORE_NAME DSPCore_AVX512
#elif INSTRSET >= 8
#define NOTE_NAME Note_AVX2
#define DSPCORE_NAME DSPCore_AVX2
#elif INSTRSET >= 7
#define NOTE_NAME Note_AVX
#define DSPCORE_NAME DSPCore_AVX
#else
#error Unsupported instruction set
#endif

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float midiNoteToFrequency(float pitch, float tuning)
{
  return 440.0f * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning) / 1200.0f);
}

// Using fmod because if equalTemperament == 1, this returns 2^121 which is too large.
inline float semiToPitch(float semi, float equalTemperament)
{
  return fmodf(powf(2.0f, semi / equalTemperament), 2048);
}

inline float paramMilliToPitch(float semi, float milli, float equalTemperament)
{
  return fmodf(
    powf(2.0f, (1000.0f * floorf(semi) + milli) / (equalTemperament * 1000.0f)), 2048);
}

// https://en.wikipedia.org/wiki/Cent_(music)#Piecewise_linear_approximation
inline float centApprox(float cent) { return 1.0f + 0.0005946f * cent; }

template<typename Sample> void NOTE_NAME<Sample>::setup(Sample sampleRate)
{
  this->sampleRate = sampleRate;

  for (auto &osc : oscillator) osc.setup(sampleRate);
  chordPan.fill(0.5);

  gainEnvelope.setup(sampleRate);
}

template<typename Sample>
void NOTE_NAME<Sample>::noteOn(
  int32_t noteId,
  Sample normalizedKey,
  Sample frequency,
  Sample velocity,
  GlobalParameter &param,
  White<float> &rng)
{
  state = NoteState::active;
  id = noteId;
  this->normalizedKey = normalizedKey;
  this->frequency = frequency;
  this->velocity = velocity;

  using ID = ParameterID::ID;

  const Sample semiSign
    = param.value[ID::negativeSemi]->getInt() ? Sample(-1) : Sample(1);
  const Sample eqTemp = param.value[ID::equalTemperament]->getInt();

  const Sample nyquist = sampleRate / 2;
  const Sample randGainAmt = 3 * param.value[ID::randomGainAmount]->getFloat();
  const Sample randFreqAmt = param.value[ID::randomFrequencyAmount]->getFloat();
  const Sample pitchMultiply = param.value[ID::pitchMultiply]->getFloat();
  const Sample pitchModulo
    = semiToPitch(param.value[ID::pitchModulo]->getFloat(), eqTemp);

  frequency
    *= somepow<Sample>(2, somefloor<Sample>(param.value[ID::masterOctave]->getFloat()));
  const Sample lowShelfFreq = frequency
    * semiToPitch(semiSign * param.value[ID::lowShelfPitch]->getFloat(), eqTemp);
  const Sample lowShelfGain = param.value[ID::lowShelfGain]->getFloat();
  const Sample highShelfFreq = frequency
    * semiToPitch(semiSign * param.value[ID::highShelfPitch]->getFloat(), eqTemp);
  const Sample highShelfGain = param.value[ID::highShelfGain]->getFloat();

  const auto enableAliasing = param.value[ID::aliasing]->getInt();

  std::array<float, nPitch> notePitch;
  std::array<float, nPitch> noteGain;
  for (size_t i = 0; i < nPitch; ++i) {
    notePitch[i] = paramMilliToPitch(
      semiSign * param.value[ID::semi0 + i]->getFloat(),
      param.value[ID::milli0 + i]->getFloat(), eqTemp);
    noteGain[i] = param.value[ID::gain0 + i]->getFloat();
  }

  Vec16f overtonePitch;
  Vec16f overtoneGain;
  for (int i = 0; i < nOvertone; ++i) {
    overtonePitch.insert(i, float(i + 1));
    overtoneGain.insert(i, float(param.value[ID::overtone1 + i]->getFloat()));
  }

  for (size_t chord = 0; chord < nChord; ++chord) {
    float chordFreq = frequency
      * paramMilliToPitch(
                        semiSign * param.value[ID::chordSemi0 + chord]->getFloat(),
                        param.value[ID::chordMilli0 + chord]->getFloat(), eqTemp);
    float chordGain = param.value[ID::chordGain0 + chord]->getFloat();
    for (size_t pitch = 0; pitch < nPitch; ++pitch) {
      // Equation to calculate a sine wave frquency.
      // freq = noteFreq * (overtone + 1) * (pitch + 1)
      //      = noteFreq * (1 + pitch + overtone + pitch * overtone);
      Vec16f rndPt = randFreqAmt
        * Vec16f(rng.process(), rng.process(), rng.process(), rng.process(),
                 rng.process(), rng.process(), rng.process(), rng.process(),
                 rng.process(), rng.process(), rng.process(), rng.process(),
                 rng.process(), rng.process(), rng.process(), rng.process());
      Vec16f modPt = pitchMultiply * (rndPt + overtonePitch + rndPt * overtonePitch);
      if (pitchModulo != 1) // Modulo operation. modf isn't available in vcl.
        modPt = modPt - pitchModulo * floor(modPt / pitchModulo);

      Vec16f oscFreq = chordFreq * notePitch[pitch] * (1.0f + modPt);
      oscillator[chord].frequency[pitch] = oscFreq;

      Vec16f shelving(1.0f);
      shelving = select(oscFreq <= lowShelfFreq, shelving * lowShelfGain, shelving);
      shelving = select(oscFreq >= highShelfFreq, shelving * highShelfGain, shelving);
      Vec16f rndGn(
        rng.process(), rng.process(), rng.process(), rng.process(), rng.process(),
        rng.process(), rng.process(), rng.process(), rng.process(), rng.process(),
        rng.process(), rng.process(), rng.process(), rng.process(), rng.process(),
        rng.process());

      Vec16f oscGain(overtoneGain);
      if (!enableAliasing) oscGain = select(oscFreq >= nyquist, 0.0f, oscGain);

      oscillator[chord].gain[pitch]
        = oscGain * chordGain * noteGain[pitch] * shelving * (1.0f + randGainAmt * rndGn);
    }
  }

  for (auto &osc : oscillator) osc.setup(sampleRate);

  for (size_t i = 0; i < nChord; ++i)
    chordPan[i] = param.value[ID::chordPan0 + i]->getFloat();

  gainEnvelope.reset(
    param.value[ID::gainA]->getFloat(), param.value[ID::gainD]->getFloat(),
    param.value[ID::gainS]->getFloat(), param.value[ID::gainR]->getFloat(), frequency,
    param.value[ID::gainEnvelopeCurve]->getFloat());
  gainEnvCurve = param.value[ID::gainEnvelopeCurve]->getFloat();
}

template<typename Sample> void NOTE_NAME<Sample>::release()
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
  gainEnvelope.release();
}

template<typename Sample> void NOTE_NAME<Sample>::rest() { state = NoteState::rest; }

template<typename Sample> std::array<Sample, 2> NOTE_NAME<Sample>::process()
{
  if (state == NoteState::rest) return {0, 0};

  std::array<Sample, 2> out{};
  for (size_t i = 0; i < nChord; ++i) {
    auto sig = oscillator[i].process();
    out[0] += sig * (Sample(1) - chordPan[i]);
    out[1] += sig * chordPan[i];
  }

  const auto gainEnv = gainEnvelope.process();
  if (gainEnvelope.isTerminated()) rest();

  gain = velocity
    * (gainEnv
       + gainEnvCurve
         * (juce::dsp::FastMathApproximations::tanh(2.0f * gainEnvCurve * gainEnv) - gainEnv));
  out[0] *= gain;
  out[1] *= gain;

  return out;
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (auto &note : notes) note.setup(sampleRate);

  for (auto &chrs : chorus)
    chrs.setup(
      sampleRate, 0,
      Scales::chorusDelayTimeRange.getMax() + Scales::chorusMinDelayTime.getMax());

  // 2 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(sampleRate * 0.005), {0, 0});

  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &note : notes) note.rest();
  lastNoteFreq = 1.0f;

  for (auto &chrs : chorus) chrs.reset();

  startup();
}

void DSPCORE_NAME::startup() { rng.seed = param.value[ParameterID::seed]->getInt(); }

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpTremoloMix.push(param.value[ID::chorusMix]->getFloat());
  interpMasterGain.push(
    param.value[ID::gain]->getFloat() * param.value[ID::gainBoost]->getFloat());

  nVoice = size_t(1) << param.value[ID::nVoice]->getInt();
  if (nVoice > notes.size()) nVoice = notes.size();

  for (auto &note : notes) {
    if (note.state == NoteState::rest) continue;
    note.gainEnvelope.set(
      param.value[ID::gainA]->getFloat(), param.value[ID::gainD]->getFloat(),
      param.value[ID::gainS]->getFloat(), param.value[ID::gainR]->getFloat(),
      note.frequency);
  }

  for (size_t i = 0; i < chorus.size(); ++i) {
    chorus[i].setParam(
      param.value[ID::chorusFrequency]->getFloat(),
      param.value[ID::chorusPhase]->getFloat()
        + i * param.value[ID::chorusOffset]->getFloat(),
      param.value[ID::chorusFeedback]->getFloat(),
      param.value[ID::chorusDepth]->getFloat(),
      param.value[ID::chorusDelayTimeRange0 + i]->getFloat(),
      param.value[ID::chorusKeyFollow]->getInt()
        ? 200.0f * param.value[ID::chorusMinDelayTime0 + i]->getFloat() / lastNoteFreq
        : param.value[ID::chorusMinDelayTime0 + i]->getFloat());
  }
}

void DSPCORE_NAME::process(const size_t length, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> frame{};
  std::array<float, 2> chorusOut{};
  for (uint32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    frame.fill(0.0f);

    for (auto &note : notes) {
      if (note.state == NoteState::rest) continue;
      auto noteSig = note.process();
      frame[0] += noteSig[0];
      frame[1] += noteSig[1];
    }

    if (isTransitioning) {
      frame[0] += transitionBuffer[mptIndex][0];
      frame[1] += transitionBuffer[mptIndex][1];
      transitionBuffer[mptIndex].fill(0.0f);
      mptIndex = (mptIndex + 1) % transitionBuffer.size();
      if (mptIndex == mptStop) isTransitioning = false;
    }

    const auto chorusIn = frame[0] + frame[1];
    chorusOut.fill(0.0f);
    for (auto &chrs : chorus) {
      const auto out = chrs.process(chorusIn);
      chorusOut[0] += out[0];
      chorusOut[1] += out[1];
    }
    chorusOut[0] /= chorus.size();
    chorusOut[1] /= chorus.size();

    const auto chorusMix = interpTremoloMix.process();
    const auto masterGain = interpMasterGain.process();
    out0[i] = masterGain * (frame[0] + chorusMix * (chorusOut[0] - frame[0]));
    out1[i] = masterGain * (frame[1] + chorusMix * (chorusOut[1] - frame[1]));
  }
}

void DSPCORE_NAME::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  size_t noteIdx = 0;
  size_t mostSilent = 0;
  float gain = 1.0f;
  for (; noteIdx < nVoice; ++noteIdx) {
    if (notes[noteIdx].id == noteId) break;
    if (notes[noteIdx].state == NoteState::rest) break;
    if (!notes[noteIdx].gainEnvelope.isAttacking() && notes[noteIdx].gain < gain) {
      gain = notes[noteIdx].gain;
      mostSilent = noteIdx;
    }
  }
  if (noteIdx >= nVoice) {
    isTransitioning = true;

    noteIdx = mostSilent;

    // Beware the negative overflow. mptStop is size_t.
    mptStop = mptIndex - 1;
    if (mptStop >= transitionBuffer.size()) mptStop += transitionBuffer.size();

    for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
      if (notes[noteIdx].state == NoteState::rest) {
        mptStop = mptIndex + bufIdx;
        if (mptStop >= transitionBuffer.size()) mptStop -= transitionBuffer.size();
        break;
      }

      auto frame = notes[noteIdx].process();
      auto idx = (mptIndex + bufIdx) % transitionBuffer.size();
      auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

      transitionBuffer[idx][0] += frame[0] * interp;
      transitionBuffer[idx][1] += frame[1] * interp;
    }
  }

  if (param.value[ParameterID::randomRetrigger]->getInt())
    rng.seed = param.value[ParameterID::seed]->getInt();

  auto normalizedKey = float(pitch) / 127.0f;
  lastNoteFreq = midiNoteToFrequency(pitch, tuning);
  notes[noteIdx].noteOn(noteId, normalizedKey, lastNoteFreq, velocity, param, rng);
}

void DSPCORE_NAME::noteOff(int32_t noteId)
{
  size_t i = 0;
  for (; i < notes.size(); ++i) {
    if (notes[i].id == noteId) break;
  }
  if (i >= notes.size()) return;

  notes[i].release();
}
