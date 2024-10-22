// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

inline float calcMasterPitch(
  int32_t octave, int32_t semi, int32_t milli, float bend, float equalTemperament)
{
  return equalTemperament * octave + semi + milli / 1000.0f + (bend - 0.5f) * 4.0f;
}

inline float
notePitchToFrequency(float notePitch, float equalTemperament = 12.0f, float a4Hz = 440.0f)
{
  return a4Hz * powf(2.0f, (notePitch - 69.0f) / equalTemperament);
}

inline float calcNotePitch(float notePitch, float equalTemperament = 12.0f)
{
  return powf(2.0f, (notePitch - 69.0f) / equalTemperament);
}

// Fast approximation of EMAFilter::cutoffToP().
// x is normalized frequency. Range is [0, 0.5).
template<typename T> inline T cutoffToPApprox(T x)
{
  return (T(-0.0004930424721520979) + T(2.9650003823571467) * x
          + T(1.8250079928630534) * x * x)
    / (T(0.4649282844668299) + T(1.8754712250208077) * x + T(3.7307819672604023) * x * x)
    + T(0.0010604699447733293);
}

// Fast approximation of OnePoleHighpass::setCutoff().
// x is normalized frequency. Range is [0, 0.5).
template<typename T> T onepoleHighpassPoleApprox(T x)
{
  return (T(5.476984559402437) + T(-13.572160512877103) * x
          + T(9.553503352240815) * x * x)
    / (T(5.479174921068828) + T(20.63587495820437) * x + T(36.02138404173517) * x * x);
}

void Note::setup(float sampleRate)
{
  cymbalLowpassEnvelope.setup(sampleRate);
  cymbal.setup(sampleRate);
  for (auto &cmb : comb) cmb.reset();
}

void Note::noteOn(
  int32_t noteId,
  float notePitch,
  float velocity,
  float pan,
  float sampleRate,
  NoteProcessInfo &info,
  GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  state = NoteState::active;
  id = noteId;

  this->velocity = velocity;
  this->pan = pan;
  gain = 1.0f;

  const float eqTemp = pv[ID::equalTemperament]->getFloat() + 1;
  const auto semitone = int32_t(pv[ID::semitone]->getInt()) - 120;
  const auto octave = eqTemp * (int32_t(pv[ID::octave]->getInt()) - 12);
  const auto milli = 0.001f * (int32_t(pv[ID::milli]->getInt()) - 1000);
  const float a4Hz = pv[ID::pitchA4Hz]->getFloat() + 100;
  const auto pitch = calcNotePitch(octave + semitone + milli + notePitch, eqTemp);
  const auto frequency = a4Hz * pitch;

  noise.reset(
    sampleRate, pv[ID::exciterAttack]->getFloat(), pv[ID::exciterDecay]->getFloat(),
    frequency, pv[ID::exciterNoiseMix]->getFloat());
  exciterLowpass.reset();
  exciterLowpass.setCutoff(sampleRate, pv[ID::exciterLowpassCutoff]->getFloat());
  gate.reset(sampleRate, pv[ID::exciterAttack]->getFloat());

  releaseLength = 0.01f * sampleRate;
  releaseCounter = int32_t(releaseLength);

  for (size_t idx = 0; idx < nComb; ++idx) {
    const auto combTime = pv[ID::combTime0 + idx]->getFloat();
    const auto spread = combTime * pv[ID::randomComb]->getFloat();
    std::uniform_real_distribution<float> distCombTime(
      combTime - spread, combTime + spread);
    comb[idx].setTime(sampleRate, distCombTime(info.rngComb));
  }

  for (size_t idx = 0; idx < nDelay; ++idx) {
    const auto freq = pitch * pv[ID::frequency0 + idx]->getFloat();
    const auto spread
      = (freq - float(Scales::frequency.getMin())) * pv[ID::randomFrequency]->getFloat();

    auto distLower = freq - spread;
    auto distUpper = freq + spread;
    if (distLower > distUpper) std::swap(distLower, distUpper);
    std::uniform_real_distribution<float> distFreq(0.0f, 1.0f);
    auto freqValue = distLower + (distUpper - distLower) * distFreq(info.rngString);
    cymbal.string[idx].delay.setTime(sampleRate, 1.0f / freqValue);
  }
  cymbal.trigger(pv[ID::distance]->getFloat(), pv[ID::connection]->getInt());

  cymbalLowpassEnvelope.prepare(
    sampleRate, pv[ID::lowpassA]->getFloat(), pv[ID::lowpassD]->getFloat(),
    pv[ID::lowpassS]->getFloat(), pv[ID::lowpassR]->getFloat());

  dcKiller.reset();

  isCompressorOn = pv[ID::compressor]->getInt();
  compressor.prepare(
    sampleRate, pv[ID::compressorTime]->getFloat(),
    pv[ID::compressorThreshold]->getFloat());
  compressor.reset();
}

void Note::release(float sampleRate)
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
  cymbalLowpassEnvelope.release(sampleRate);
}

void Note::rest() { state = NoteState::rest; }

bool Note::isAttacking() { return cymbalLowpassEnvelope.isAttacking(); }

float Note::getGain() { return gain; }

std::array<float, 2> Note::process(float sampleRate, NoteProcessInfo &info)
{
  float sig = noise.isTerminated
    ? 0
    : info.noiseGain.getValue() * exciterLowpass.process(noise.process(info.rngNoise));

  for (auto &cmb : comb) sig -= cmb.process(sig);
  sig *= gate.process();

  float lpEnv = cymbalLowpassEnvelope.process(sampleRate);
  gain = velocity * lpEnv; // Used to determin most quiet note.

  cymbal.kp = cutoffToPApprox(lpEnv * info.lowpassCutoff.getValue() / sampleRate);
  cymbal.b1 = onepoleHighpassPoleApprox(info.highpassCutoff.getValue() / sampleRate);
  sig = dcKiller.process(cymbal.process(sig, info.propagation.getValue()));

  if (isCompressorOn) sig = compressor.process(sig);

  if (cymbalLowpassEnvelope.isTerminated()) {
    --releaseCounter;
    sig *= releaseCounter / releaseLength;
    if (releaseCounter <= 0) state = NoteState::rest;
  }

  sig *= velocity;
  return {(1.0f - pan) * sig, pan * sig};
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.01f);

  // 10 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(this->sampleRate * 0.005), {0.0f, 0.0f});

  for (auto &note : notes) note.setup(this->sampleRate);

  reset();
}

DSPCore::DSPCore()
{
  unisonPan.reserve(maxVoice);
  noteIndices.reserve(maxVoice);
  voiceIndices.reserve(maxVoice);
}

void DSPCore::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.reset(param);

  for (auto &note : notes) {
    note.rest();
    note.id = -1;
    note.gain = 0;
    note.noise.resetPhase();
    for (auto &cmb : note.comb) cmb.reset();
    note.cymbal.reset();
    note.cymbalLowpassEnvelope.reset();
  }

  interpMasterGain.reset(pv[ID::gain]->getFloat() * pv[ID::boost]->getFloat());

  for (auto &frame : transitionBuffer) frame.fill(0.0f);
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;
}

void DSPCore::startup() {}

void DSPCore::setParameters(float /* tempo */)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.setParameters(param);

  nVoice = pv[ID::nVoice]->getInt() + 1;

  interpMasterGain.push(pv[ID::gain]->getFloat() * pv[ID::boost]->getFloat());

  for (auto &note : notes) {
    if (note.state == NoteState::rest) continue;
    note.cymbalLowpassEnvelope.set(
      sampleRate, pv[ID::lowpassA]->getFloat(), pv[ID::lowpassD]->getFloat(),
      pv[ID::lowpassS]->getFloat(), pv[ID::lowpassR]->getFloat());
  }
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  std::array<float, 2> frame{};
  for (uint32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    info.process();

    frame.fill(0.0f);

    for (auto &note : notes) {
      if (note.state == NoteState::rest) continue;
      auto sig = note.process(sampleRate, info);
      frame[0] += sig[0];
      frame[1] += sig[1];
    }

    if (isTransitioning) {
      frame[0] += transitionBuffer[trIndex][0];
      frame[1] += transitionBuffer[trIndex][1];
      transitionBuffer[trIndex].fill(0.0f);
      trIndex = (trIndex + 1) % transitionBuffer.size();
      if (trIndex == trStop) isTransitioning = false;
    }

    const auto masterGain = interpMasterGain.process();
    out0[i] = masterGain * frame[0];
    out1[i] = masterGain * frame[1];
  }
}

void DSPCore::setUnisonPan(size_t nUnison)
{
  using ID = ParameterID::ID;

  unisonPan.resize(nUnison);

  const float unisonSpread = param.value[ID::unisonSpread]->getFloat();
  const float panRange = unisonSpread / float(nUnison - 1);
  const float panOffset = 0.5f - 0.5f * unisonSpread;

  for (size_t idx = 0; idx < unisonPan.size(); ++idx)
    unisonPan[idx] = panRange * idx + panOffset;
}

void DSPCore::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  const size_t nUnison = 1 + pv[ID::nUnison]->getInt();

  noteIndices.resize(0);

  // Pick up note from resting one.
  for (uint32_t index = 0; index < nVoice; ++index) {
    if (notes[index].id == noteId) noteIndices.push_back(index);
    if (notes[index].state == NoteState::rest) noteIndices.push_back(index);
    if (noteIndices.size() >= nUnison) break;
  }

  // If there aren't enought resting note, pick up from most quiet one.
  if (noteIndices.size() < nUnison) {
    voiceIndices.resize(nVoice);
    std::iota(voiceIndices.begin(), voiceIndices.end(), 0);
    std::sort(voiceIndices.begin(), voiceIndices.end(), [&](size_t lhs, size_t rhs) {
      return !notes[lhs].isAttacking() && (notes[lhs].getGain() < notes[rhs].getGain());
    });

    for (auto &index : voiceIndices) {
      fillTransitionBuffer(index);
      noteIndices.push_back(index);
      if (noteIndices.size() >= nUnison) break;
    }
  }

  // Parameters must be set after transition buffer is filled.
  velocity = velocityMap.map(velocity);

  if (pv[ID::retriggerNoise]->getInt()) info.rngNoise.seed(pv[ID::seedNoise]->getInt());
  if (pv[ID::retriggerComb]->getInt()) info.rngComb.seed(pv[ID::seedComb]->getInt());
  if (pv[ID::retriggerString]->getInt())
    info.rngString.seed(pv[ID::seedString]->getInt());
  if (pv[ID::retriggerUnison]->getInt())
    info.rngUnison.seed(pv[ID::seedUnison]->getInt());

  if (nUnison <= 1) {
    notes[noteIndices[0]].noteOn(
      noteId, float(pitch) + tuning, velocity, 0.5f, sampleRate, info, param);
    return;
  }

  setUnisonPan(nUnison);

  const float unisonDetune = param.value[ID::unisonDetune]->getFloat();
  const float unisonGainRandom = param.value[ID::unisonGainRandom]->getFloat();
  const bool randomizeDetune = param.value[ID::unisonDetuneRandom]->getInt();
  std::uniform_real_distribution<float> distDetune(randomizeDetune ? 0.0f : 1.0f, 1.0f);
  std::uniform_real_distribution<float> distGain(1.0f - unisonGainRandom, 1.0f);
  for (size_t unison = 0; unison < nUnison; ++unison) {
    if (noteIndices.size() <= unison) break;
    auto detune = unison * unisonDetune * distDetune(info.rngUnison);
    auto notePitch = (float(pitch) + tuning) * (1.0f + detune);
    auto vel = distGain(info.rngUnison) * velocity;
    notes[noteIndices[unison]].noteOn(
      noteId, notePitch, vel, unisonPan[unison], sampleRate, info, param);
  }
}

void DSPCore::noteOff(int32_t noteId)
{
  for (size_t i = 0; i < notes.size(); ++i)
    if (notes[i].id == noteId) notes[i].release(sampleRate);
}

void DSPCore::fillTransitionBuffer(size_t noteIndex)
{
  auto &note = notes[noteIndex];
  if (note.state == NoteState::rest) return;

  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    auto oscOut = note.process(sampleRate, info);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += oscOut[0] * interp;
    transitionBuffer[idx][1] += oscOut[1] * interp;
  }
}
