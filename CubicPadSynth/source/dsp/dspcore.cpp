// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include "../../../lib/juce_FastMathApproximations.h"
#include "../../../lib/vcl/vectormath_exp.h"

#include <algorithm>
#include <numeric>
#include <random>

#include <iostream>

#if INSTRSET >= 10
#define PROCESSING_UNIT_NAME ProcessingUnit_AVX512
#define NOTE_NAME Note_AVX512
#define DSPCORE_NAME DSPCore_AVX512
#elif INSTRSET >= 8
#define PROCESSING_UNIT_NAME ProcessingUnit_AVX2
#define NOTE_NAME Note_AVX2
#define DSPCORE_NAME DSPCore_AVX2
#elif INSTRSET >= 7
#define PROCESSING_UNIT_NAME ProcessingUnit_AVX
#define NOTE_NAME Note_AVX
#define DSPCORE_NAME DSPCore_AVX
#else
#error Unsupported instruction set
#endif

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline Vec16f
notePitchToFrequency(Vec16f notePitch, float equalTemperament, float a4Hz = 440.0f)
{
  return a4Hz * pow(Vec16f(2.0f), (notePitch - 69.0f) / equalTemperament);
}

inline float
notePitchToFrequency(float notePitch, float equalTemperament, float a4Hz = 440.0f)
{
  return a4Hz * powf(2.0f, (notePitch - 69.0f) / equalTemperament);
}

void NOTE_NAME::setup(float sampleRate) { this->sampleRate = sampleRate; }

void NOTE_NAME::noteOn(
  int32_t noteId,
  float notePitch,
  float velocity,
  float pan,
  float phase,
  NoteProcessInfo &info,
  std::array<PROCESSING_UNIT_NAME, nUnit> &units,
  GlobalParameter &param)
{
  using ID = ParameterID::ID;

  state = NoteState::active;
  id = noteId;

  auto &unit = units[arrayIndex];
  unit.isActive = true;

  unit.velocity.insert(vecIndex, velocity);
  unit.gain.insert(vecIndex, 1.0f);

  unit.lfo.setFrequency(vecIndex, sampleRate, 1.0f);
  if (param.value[ID::lfoPhaseReset]->getInt()) unit.lfo.reset(vecIndex);

  std::uniform_real_distribution<float> dist(0.0, 1.0);
  unit.notePitch.insert(vecIndex, notePitch);
  if (param.value[ID::oscPhaseReset]->getInt()) {
    const auto phaseRnd
      = param.value[ID::oscPhaseRandom]->getInt() ? dist(info.rng) : 1.0f;
    unit.osc.setPhase(
      vecIndex, phase + phaseRnd * param.value[ID::oscInitialPhase]->getFloat());
  }

  unit.notePan.insert(vecIndex, pan);

  unit.gainEnvelope.reset(vecIndex);
  unit.lowpassEnvelope.reset(
    vecIndex, param.value[ID::tableLowpassA]->getFloat(),
    param.value[ID::tableLowpassD]->getFloat(),
    param.value[ID::tableLowpassS]->getFloat(),
    param.value[ID::tableLowpassR]->getFloat(), sampleRate);
  unit.pitchEnvelope.reset(
    vecIndex, param.value[ID::pitchA]->getFloat(), param.value[ID::pitchD]->getFloat(),
    param.value[ID::pitchS]->getFloat(), param.value[ID::pitchR]->getFloat(), sampleRate);
}

void NOTE_NAME::release(std::array<PROCESSING_UNIT_NAME, nUnit> &units)
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
  units[arrayIndex].gainEnvelope.release(vecIndex);
  units[arrayIndex].lowpassEnvelope.release(vecIndex);
  units[arrayIndex].pitchEnvelope.release(vecIndex);
}

void NOTE_NAME::release(std::array<PROCESSING_UNIT_NAME, nUnit> &units, float seconds)
{
  release(units);
  units[arrayIndex].gainEnvelope.setRelease(vecIndex, seconds);
}

void NOTE_NAME::rest()
{
  state = NoteState::rest;
  id = -1;
}

bool NOTE_NAME::isAttacking(std::array<PROCESSING_UNIT_NAME, nUnit> &units)
{
  return units[arrayIndex].gainEnvelope.isAttacking(vecIndex);
}

float NOTE_NAME::getGain(std::array<PROCESSING_UNIT_NAME, nUnit> &units)
{
  return units[arrayIndex].gain[vecIndex];
}

DSPCORE_NAME::DSPCORE_NAME()
{
  unisonPan.reserve(maxVoice);
  noteIndices.reserve(maxVoice);
  voiceIndices.reserve(maxVoice);

  midiNotes.reserve(maxVoice);

  for (int i = 0; i < notes.size(); ++i) {
    notes[i].vecIndex = i % 16;
    notes[i].arrayIndex = i / 16;
  }
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  midiNotes.resize(0);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (size_t idx = 0; idx < nUnit; ++idx) {
    units[idx].gainEnvelope.setup(
      this->sampleRate, param.value[ParameterID::gainS]->getFloat());
  }

  for (auto &note : notes) note.setup(this->sampleRate);

  // 2 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(this->sampleRate * 0.01), {0.0f, 0.0f});

  startup();
  prepareRefresh = true;
}

void PROCESSING_UNIT_NAME::reset(GlobalParameter &param)
{
  notePitch = 0;
  pitch = 0;
  lowpassPitch = 0;
  notePan = 0.5f;
  frequency = 1;
  gain = 0;
  gain0 = 0;
  gain1 = 0;
  velocity = 0;

  isActive = false;

  osc.reset();
  lfo.reset();
  lfoSmoother.reset();
  gainEnvelope.terminate();
  gainEnvelope.resetSustain(param.value[ParameterID::gainS]->getFloat());
  pitchEnvelope.resetSustain(param.value[ParameterID::pitchS]->getFloat());
  lowpassEnvelope.resetSustain(param.value[ParameterID::tableLowpassS]->getFloat());
}

void DSPCORE_NAME::reset()
{
  for (auto &note : notes) note.rest();
  for (auto &unit : units) unit.reset(param);
  info.reset(param);

  panCounter = 0;

  interpMasterGain.reset(param.value[ParameterID::gain]->getFloat());

  for (auto &buf : transitionBuffer) buf.fill(0);
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;

  startup();
}

void DSPCORE_NAME::startup()
{
  info.rng.seed(0); // TODO: provide seed.

  for (auto &unit : units) {
    unit.osc.setPhase(param.value[ParameterID::oscInitialPhase]->getFloat());
    unit.lfo.reset();
    unit.lfoSmoother.reset();
  }
}

void PROCESSING_UNIT_NAME::setParameters(
  float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;

  gainEnvelope.set(
    param.value[ID::gainA]->getFloat(), param.value[ID::gainD]->getFloat(),
    param.value[ID::gainS]->getFloat(), param.value[ID::gainR]->getFloat(),
    notePitchToFrequency(
      notePitch + info.masterPitch.getValue(), info.equalTemperament.getValue(),
      info.pitchA4Hz.getValue()));
  lowpassEnvelope.set(
    param.value[ID::tableLowpassA]->getFloat(),
    param.value[ID::tableLowpassD]->getFloat(),
    param.value[ID::tableLowpassS]->getFloat(),
    param.value[ID::tableLowpassR]->getFloat(), sampleRate);
  pitchEnvelope.set(
    param.value[ID::pitchA]->getFloat(), param.value[ID::pitchD]->getFloat(),
    param.value[ID::pitchS]->getFloat(), param.value[ID::pitchR]->getFloat(), sampleRate);
}

void DSPCORE_NAME::setParameters(float tempo)
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpMasterGain.push(param.value[ID::gain]->getFloat());

  info.masterPitch.push(getMasterPitch(param));
  info.equalTemperament.push(param.value[ID::equalTemperament]->getFloat() + 1);
  info.pitchA4Hz.push(param.value[ID::pitchA4Hz]->getFloat() + 100);
  info.tableLowpass.push(
    float(Scales::tableLowpass.getMax()) - param.value[ID::tableLowpass]->getFloat());
  info.tableLowpassKeyFollow.push(param.value[ID::tableLowpassKeyFollow]->getFloat());
  info.tableLowpassEnvelopeAmount.push(
    param.value[ID::tableLowpassEnvelopeAmount]->getFloat());
  info.pitchEnvelopeAmount.push(
    param.value[ID::pitchEnvelopeAmount]->getFloat()
    * (param.value[ID::pitchEnvelopeAmountNegative]->getInt() ? -1 : 1));

  const float beat = float(param.value[ID::lfoTempoNumerator]->getInt() + 1)
    / float(param.value[ID::lfoTempoDenominator]->getInt() + 1);
  info.lfoFrequency.push(
    param.value[ID::lfoFrequencyMultiplier]->getFloat() * tempo / 240.0f / beat);
  info.lfoPitchAmount.push(param.value[ID::lfoPitchAmount]->getFloat());
  info.lfoLowpass.push(param.value[ID::lfoLowpass]->getFloat());

  for (auto &unit : units) unit.setParameters(sampleRate, info, param);

  nVoice = 16 * (param.value[ID::nVoice]->getInt() + 1);
  if (nVoice > notes.size()) nVoice = notes.size();

  if (prepareRefresh || (!isLFORefreshed && param.value[ID::refreshLFO]->getInt()))
    refreshLfo();
  isLFORefreshed = param.value[ID::refreshLFO]->getInt();

  if (prepareRefresh || (!isTableRefeshed && param.value[ID::refreshTable]->getInt()))
    refreshTable();
  isTableRefeshed = param.value[ID::refreshTable]->getInt();

  prepareRefresh = false;
}

std::array<float, 2> PROCESSING_UNIT_NAME::process(
  float sampleRate,
  WaveTable<tableSize, nOvertone> &wavetable,
  LfoWaveTable<lfoTableSize> &lfoWaveTable,
  NoteProcessInfo &info)
{
  lfo.setFrequency(sampleRate, info.lfoFrequency.getValue());
  Vec16f lfoSig = info.lfoPitchAmount.getValue() * lfo.process(lfoWaveTable.table);
  lfoSmoother.setP(info.lfoLowpass.getValue());
  lfoSig = lfoSmoother.process(lfoSig);

  pitch = lfoSig + notePitch + info.masterPitch.getValue()
    + info.pitchEnvelopeAmount.getValue() * pitchEnvelope.process();
  osc.setFrequency(
    notePitchToFrequency(
      pitch, info.equalTemperament.getValue(), info.pitchA4Hz.getValue()),
    wavetable.tableBaseFreq);

  float lpKey = info.tableLowpassKeyFollow.getValue();
  float lpCutoff = info.tableLowpass.getValue();
  float lpPt = lpCutoff * 128.0f; // 128 comes from midi note number range + 1.
  lowpassPitch = (lpPt + lpKey * (lpCutoff * (float(nTable) - pitch) - lpPt))
    - lowpassEnvelope.process() * info.tableLowpassEnvelopeAmount.getValue();
  lowpassPitch = select(lowpassPitch < 0.0f, 0.0f, lowpassPitch);
  Vec16f sig = osc.processCubic(lowpassPitch + pitch, wavetable.table);

  gain = velocity * gainEnvelope.process();
  isActive = horizontal_add(gain) != 0;

  gain1 = gain * notePan;
  gain0 = gain - gain1;

  std::array<float, 2> frame;
  frame[0] = horizontal_add(gain0 * sig);
  frame[1] = horizontal_add(gain1 * sig);
  return frame;
}

void DSPCORE_NAME::process(const size_t length, float *out0, float *out1)
{
  if (wavetable.isRefreshing) {
    for (int i = 0; i < length; ++i) {
      processMidiNote(i);
      out0[i] = 0;
      out1[i] = 0;
    }
    return;
  }

  SmootherCommon<float>::setBufferSize(float(length));

  std::array<float, 2> frame{};
  for (uint32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    info.masterPitch.process();
    info.equalTemperament.process();
    info.pitchA4Hz.process();
    info.tableLowpass.process();
    info.tableLowpassKeyFollow.process();
    info.tableLowpassEnvelopeAmount.process();
    info.pitchEnvelopeAmount.process();
    info.lfoFrequency.process();
    info.lfoPitchAmount.process();
    info.lfoLowpass.process();

    frame.fill(0.0f);

    for (auto &unit : units) {
      if (!unit.isActive) continue;
      auto sig = unit.process(sampleRate, wavetable, lfoWavetable, info);
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

enum UnisonPanType {
  unisonPanAlternateLR,
  unisonPanAlternateMS,
  unisonPanAscendLR,
  unisonPanAscendRL,
  unisonPanHighOnMid,
  unisonPanHighOnSide,
  unisonPanRandom,
  unisonPanRotateL,
  unisonPanRotateR,
  unisonPanShuffle
};

void DSPCORE_NAME::sortVoiceIndicesByGain()
{
  voiceIndices.resize(nVoice);
  std::iota(voiceIndices.begin(), voiceIndices.end(), 0);
  std::sort(voiceIndices.begin(), voiceIndices.end(), [&](size_t left, size_t right) {
    return !notes[left].isAttacking(units)
      && (notes[left].getGain(units) < notes[right].getGain(units));
  });
}

void DSPCORE_NAME::terminateNotes(size_t nNote)
{
  if (param.value[ParameterID::voicePool]->getInt()) {
    sortVoiceIndicesByGain();
    if (nNote > voiceIndices.size()) nNote = voiceIndices.size();
    for (size_t idx = 0; idx < nNote; ++idx) {
      notes[voiceIndices[idx]].release(units, 0.02f);
    }
  }
}

void DSPCORE_NAME::noteOn(int32_t identifier, int16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;

  const size_t nUnison = 1 + param.value[ID::nUnison]->getInt();

  noteIndices.resize(0);

  // Pick up note from resting one.
  for (size_t index = 0; index < nVoice; ++index) {
    if (notes[index].id == identifier) noteIndices.push_back(index);
    if (notes[index].state == NoteState::rest) noteIndices.push_back(index);
    if (noteIndices.size() >= nUnison) break;
  }

  // If there aren't enought resting note, pick up from most quiet one.
  if (noteIndices.size() < nUnison) {
    sortVoiceIndicesByGain();
    for (auto &index : voiceIndices) {
      fillTransitionBuffer(index);
      noteIndices.push_back(index);
      if (noteIndices.size() >= nUnison) break;
    }
  }

  if (nUnison <= 1) {
    notes[noteIndices[0]].noteOn(
      identifier, float(pitch) + tuning, velocity, 0.5f, 0.0f, info, units, param);
    terminateNotes(nUnison);
    return;
  }

  unisonPan.resize(nUnison);
  const auto unisonPanRange = param.value[ID::unisonPan]->getFloat();
  const float panRange = unisonPanRange / float(nUnison - 1);
  const float panOffset = 0.5f - 0.5f * unisonPanRange;
  switch (param.value[ID::unisonPanType]->getInt()) {
    case unisonPanAlternateLR: {
      panCounter = !panCounter;
      if (panCounter)
        goto ASCEND_LR;
      else
        goto ASCEND_RL;
    } break;

    case unisonPanAlternateMS: {
      panCounter = !panCounter;
      if (panCounter)
        goto HIGH_ON_MID;
      else
        goto HIGH_ON_SIDE;
    } break;

    case unisonPanAscendLR: {
    ASCEND_LR:
      for (size_t idx = 0; idx < unisonPan.size(); ++idx)
        unisonPan[idx] = panRange * idx + panOffset;
    } break;

    case unisonPanAscendRL: {
    ASCEND_RL:
      size_t inc = 0;
      size_t dec = unisonPan.size() - 1;
      while (inc < unisonPan.size()) {
        unisonPan[dec] = panRange * inc + panOffset;
        ++inc, --dec;
      }
    } break;

    case unisonPanHighOnMid: {
    HIGH_ON_MID:
      size_t panIdx = unisonPan.size() / 2;
      size_t sign = 1;
      size_t inc = 0;
      size_t dec = unisonPan.size() - 1;
      while (inc < unisonPan.size()) {
        panIdx += sign * inc;
        unisonPan[dec] = panRange * panIdx + panOffset;
        sign *= -1;
        ++inc, --dec;
      }
    } break;

    case unisonPanHighOnSide: {
    HIGH_ON_SIDE:
      size_t panIdx = unisonPan.size() / 2;
      size_t sign = 1;
      for (size_t idx = 0; idx < unisonPan.size(); ++idx) {
        panIdx += sign * idx;
        unisonPan[idx] = panRange * panIdx + panOffset;
        sign *= -1;
      }
    } break;

    case unisonPanRandom: {
      const auto halfRange = 0.5f * panRange;
      std::uniform_real_distribution<float> dist(0.5f - halfRange, 0.5f + halfRange);
      for (size_t idx = 0; idx < unisonPan.size(); ++idx) unisonPan[idx] = dist(info.rng);
    } break;

    case unisonPanRotateL: {
      panCounter = (panCounter + 1) % unisonPan.size();
      for (size_t idx = 0; idx < unisonPan.size(); ++idx) {
        unisonPan[idx] = panRange * ((idx + panCounter) % unisonPan.size()) + panOffset;
      }
    } break;

    case unisonPanRotateR: {
      panCounter = (panCounter + 1) % unisonPan.size();
      size_t inc = 0;
      size_t dec = unisonPan.size() - 1;
      while (inc < unisonPan.size()) {
        unisonPan[dec] = panRange * ((inc + panCounter) % unisonPan.size()) + panOffset;
        ++inc, --dec;
      }
    } break;

    case unisonPanShuffle: // Shuffle.
    default: {
      for (size_t idx = 0; idx < unisonPan.size(); ++idx)
        unisonPan[idx] = panRange * idx + panOffset;
      std::shuffle(unisonPan.begin(), unisonPan.end(), info.rng);
    } break;
  }

  const auto unisonDetune = param.value[ID::unisonDetune]->getFloat();
  const auto unisonPhase = param.value[ID::unisonPhase]->getFloat();
  const auto unisonGainRandom = param.value[ID::unisonGainRandom]->getFloat();
  const bool randomizeDetune = param.value[ID::unisonDetuneRandom]->getInt();
  std::uniform_real_distribution<float> distDetune(0.0f, 1.0f);
  std::uniform_real_distribution<float> distGain(1.0f - unisonGainRandom, 1.0f);
  for (size_t unison = 0; unison < nUnison; ++unison) {
    if (noteIndices.size() <= unison) break;
    auto detune = unison * unisonDetune * (randomizeDetune ? distDetune(info.rng) : 1.0f);
    auto notePitch = (float(pitch) + tuning) * (1.0f + detune);
    auto phase = unisonPhase * unison / float(nUnison);
    notes[noteIndices[unison]].noteOn(
      identifier, notePitch, distGain(info.rng) * velocity, unisonPan[unison], phase,
      info, units, param);
  }

  terminateNotes(nUnison);
}

void DSPCORE_NAME::fillTransitionBuffer(size_t noteIndex)
{
  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  auto &unit = units[notes[noteIndex].arrayIndex];
  auto vecIndex = notes[noteIndex].vecIndex;

  float gain0 = unit.gain0[vecIndex];
  float gain1 = unit.gain1[vecIndex];
  float pitch = unit.lowpassPitch[vecIndex] + unit.pitch[vecIndex];
  trOsc.phase = unit.osc.phase.extract(vecIndex);
  trOsc.tick = unit.osc.tick.extract(vecIndex);

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    if (notes[noteIndex].state == NoteState::rest) {
      trStop = trIndex + bufIdx;
      if (trStop >= transitionBuffer.size()) trStop -= transitionBuffer.size();
      break;
    }

    float oscOut = trOsc.process(pitch, wavetable.table);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += oscOut * interp * gain0;
    transitionBuffer[idx][1] += oscOut * interp * gain1;
  }
}

void DSPCORE_NAME::noteOff(int32_t noteId)
{
  for (size_t i = 0; i < notes.size(); ++i)
    if (notes[i].id == noteId) notes[i].release(units);
}

void DSPCORE_NAME::refreshTable()
{
  using ID = ParameterID::ID;

  reset();

  const float tableBaseFreq = param.value[ID::tableBaseFrequency]->getFloat();
  const float pitchMultiplier = param.value[ID::overtonePitchMultiply]->getFloat();
  const float pitchModulo = param.value[ID::overtonePitchModulo]->getFloat();
  const float gainPow = param.value[ID::overtoneGainPower]->getFloat();
  const float widthMul = param.value[ID::overtoneWidthMultiply]->getFloat();

  for (size_t idx = 0; idx < nOvertone; ++idx) {
    otFrequency[idx] = (pitchMultiplier * idx + 1.0f) * tableBaseFreq
      * param.value[ID::overtonePitch0 + idx]->getFloat();
    if (pitchModulo != 0)
      otFrequency[idx]
        = fmodf(otFrequency[idx], notePitchToFrequency(pitchModulo, 12.0f, 440.0f));
    otGain[idx] = powf(param.value[ID::overtoneGain0 + idx]->getFloat(), gainPow);
    otBandWidth[idx] = widthMul * param.value[ID::overtoneWidth0 + idx]->getFloat();
    otPhase[idx] = param.value[ID::overtonePhase0 + idx]->getFloat();
  }

  wavetable.padsynth(
    sampleRate, tableBaseFreq, otFrequency, otGain, otPhase, otBandWidth,
    param.value[ID::padSynthSeed]->getInt(), param.value[ID::spectrumExpand]->getFloat(),
    int32_t(param.value[ID::spectrumShift]->getInt()) - spectrumSize,
    param.value[ID::profileComb]->getInt() + 1, param.value[ID::profileShape]->getFloat(),
    param.value[ID::overtonePitchRandom]->getInt(),
    param.value[ID::spectrumInvert]->getInt(),
    param.value[ID::uniformPhaseProfile]->getInt());
}

void DSPCORE_NAME::refreshLfo()
{
  using ID = ParameterID::ID;

  reset();

  std::vector<float> table(nLFOWavetable);
  for (size_t idx = 0; idx < nLFOWavetable; ++idx)
    table[idx] = param.value[ID::lfoWavetable0 + idx]->getFloat();

  lfoWavetable.refreshTable(table, param.value[ID::lfoWavetableType]->getInt());
}
