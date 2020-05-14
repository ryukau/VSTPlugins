// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include "../../../lib/vcl/vectormath_exp.h"

#include <algorithm>
#include <numeric>

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

inline float midiNoteToFrequency(float pitch, float tuning, float bend)
{
  return 440.0f
    * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning + (bend - 0.5f) * 400.0f) / 1200.0f);
}

// modf for VCL types.
template<typename T, typename S> inline T vecModf(T value, S divisor)
{
  return value - divisor * floor(value / divisor);
}

template<typename Sample> void NOTE_NAME<Sample>::setup(Sample sampleRate)
{
  this->sampleRate = sampleRate;

  osc.setup(sampleRate);
}

template<typename Sample>
void NOTE_NAME<Sample>::noteOn(
  int32_t noteId,
  Sample normalizedKey,
  Sample frequency,
  Sample velocity,
  Sample pan,
  GlobalParameter &param,
  White16 &rng)
{
  using ID = ParameterID::ID;

  frequency
    *= somepow<Sample>(2, somefloor<Sample>(param.value[ID::masterOctave]->getFloat()));

  state = NoteState::active;
  id = noteId;
  this->normalizedKey = normalizedKey;
  this->frequency = frequency;
  this->velocity = velocity;
  this->pan = pan;

  float noteGain = param.value[ParameterID::unison]->getInt() ? 0.5f : 1.0f;
  noteGain *= velocity;

  gain[0] = noteGain * (1.0f - pan);
  gain[1] = noteGain * pan;

  const float nyquist = sampleRate / 2;
  const bool aliasing = param.value[ID::aliasing]->getInt();
  const bool declick = param.value[ID::declick]->getInt();
  const float randGain = param.value[ID::randomGain]->getFloat();
  const float randFreq = param.value[ID::randomFrequency]->getFloat();
  const float randAttack = param.value[ID::randomAttack]->getFloat();
  const float randDecay = param.value[ID::randomDecay]->getFloat();
  const float randPhase = param.value[ID::randomPhase]->getFloat();
  const float randSaturation = param.value[ID::randomSaturation]->getFloat();
  const float pitchMultiply = param.value[ID::pitchMultiply]->getFloat();
  const float pitchModulo = param.value[ID::pitchModulo]->getFloat();
  const float gainPow = param.value[ID::gainPower]->getFloat();
  const float attackMul = param.value[ID::attackMultiplier]->getFloat();
  const float decayMul = param.value[ID::decayMultiplier]->getFloat();

  const float satMinFreq = 100;
  const float satMaxFreq = 4000;
  float satMix = 0.0f;
  if (frequency <= satMinFreq) {
    satMix = param.value[ID::saturationMix]->getFloat();
  } else {
    satMix = 1.0f - (frequency - satMinFreq) / (satMaxFreq - satMinFreq);
    satMix *= param.value[ID::saturationMix]->getFloat();
  }

  Vec16f overtonePitch(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  overtonePitch += param.value[ID::overtoneShift]->getFloat();

  for (size_t idx = 0; idx < 64; ++idx) {
    paramAttack[idx] = param.value[ID::attack0 + idx]->getFloat();
    paramDecay[idx] = param.value[ID::decay0 + idx]->getFloat();
    paramGain[idx] = param.value[ID::overtone0 + idx]->getFloat();
    paramSaturation[idx] = param.value[ID::saturation0 + idx]->getFloat();
  }

  const float expand = param.value[ID::overtoneExpand]->getFloat();
  for (size_t idx = 0; idx < oscillatorSize; ++idx) {
    for (int jdx = 0; jdx < 16; ++jdx) {
      float index = expand * (16 * idx + jdx);
      size_t high = ceilf(index);

      if (high >= paramGain.size()) {
        osc.attack[idx].insert(jdx, 0);
        osc.decay[idx].insert(jdx, 0);
        osc.saturation[idx].insert(jdx, 0);
        osc.gain[idx].insert(jdx, 0);
        continue;
      }

      size_t low = index;
      float frac = index - low;

      osc.attack[idx].insert(
        jdx, paramAttack[low] + frac * (paramAttack[high] - paramAttack[low]));
      osc.decay[idx].insert(
        jdx, paramDecay[low] + frac * (paramDecay[high] - paramDecay[low]));
      osc.saturation[idx].insert(
        jdx,
        paramSaturation[low] + frac * (paramSaturation[high] - paramSaturation[low]));
      osc.gain[idx].insert(
        jdx, paramGain[low] + frac * (paramGain[high] - paramGain[low]));
    }
  }

  // QuadOsc diverges around nyquist frequency.
  const auto maxFreq = nyquist * 0.999f;
  const auto minFreq = nyquist * 0.001f;

  for (size_t idx = 0; idx < oscillatorSize; ++idx) {
    Vec16f rndPt = randFreq * rng.process();
    Vec16f modPt = pitchMultiply * (rndPt + overtonePitch + rndPt * overtonePitch);
    if (pitchModulo != 0) {
      modPt = vecModf(modPt, pitchModulo);
    }
    osc.frequency[idx] = frequency * abs(modPt);

    overtonePitch += 16.0f;

    osc.attack[idx] *= attackMul;
    osc.attack[idx] *= 1.0f + randAttack * (rng.process() - 1.0f); // Linear interp.

    osc.decay[idx] *= decayMul;
    Vec16f rndDec = rng.process();
    osc.decay[idx] *= 1.0f + randDecay * (rndDec * rndDec - 1.0f);

    if (declick) {
      Vec16f envFreq = abs(osc.frequency[idx]);
      envFreq = select(envFreq < 1, 1, envFreq); // Avoid negative and division by 0.
      osc.attack[idx] += 1.0f / envFreq + 0.001f;
      osc.decay[idx] += 2.0f / envFreq + 0.001f;
    }

    Vec16f rndGn = rng.process();
    osc.gain[idx] *= 1.0f + randGain * rndGn;
    osc.gain[idx] = pow(osc.gain[idx], gainPow);
    osc.gain[idx]
      = select(osc.gain[idx] > 1.0f, vecModf(osc.gain[idx], 1.0f), osc.gain[idx]);
    if (!aliasing) {
      osc.gain[idx] = select(osc.frequency[idx] >= maxFreq, 0, osc.gain[idx]);
    }

    osc.saturation[idx] *= 1.0f + randSaturation * (rng.process() - 1.0f);
    osc.satMix[idx] = satMix;
    osc.phase[idx] = randPhase * rng.process();
  }

  osc.setup(sampleRate);
}

template<typename Sample> void NOTE_NAME<Sample>::release()
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
}

template<typename Sample> void NOTE_NAME<Sample>::rest() { state = NoteState::rest; }

template<typename Sample> std::array<Sample, 2> NOTE_NAME<Sample>::process()
{
  if (state == NoteState::rest) return {0.0f, 0.0f};

  float sig = osc.process();
  if (osc.isTerminated()) rest();

  std::array<Sample, 2> frame;
  frame[0] = gain[0] * sig;
  frame[1] = gain[1] * sig;

  return frame;
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  interpPhaserPhase.setRange(float(twopi));

  for (auto &note : notes) note.setup(sampleRate);

  // 2 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(sampleRate * 0.005), {0.0f, 0.0f});

  for (auto &note : notes) note.rest();
  midiNotes.resize(0);

  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &note : notes) note.rest();
  lastNoteFreq = 1.0f;

  for (auto &ph : phaser) ph.reset();

  startup();
}

void DSPCORE_NAME::startup()
{
  rng.setSeed(param.value[ParameterID::seed]->getInt());

  for (size_t i = 0; i < phaser.size(); ++i) {
    phaser[i].phase = float(i) / phaser.size();
  }
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpMasterGain.push(
    param.value[ID::gain]->getFloat() * param.value[ID::gainBoost]->getFloat());

  interpPhaserMix.push(param.value[ID::phaserMix]->getFloat());
  interpPhaserFrequency.push(
    param.value[ID::phaserFrequency]->getFloat() * twopi / sampleRate);
  interpPhaserFeedback.push(param.value[ID::phaserFeedback]->getFloat());

  const float phaserRange = param.value[ID::phaserRange]->getFloat();
  interpPhaserRange.push(phaserRange);
  interpPhaserMin.push(
    Thiran2Phaser16::getOffset(phaserRange, param.value[ID::phaserMin]->getFloat()));

  interpPhaserPhase.push(param.value[ID::phaserPhase]->getFloat());
  interpPhaserOffset.push(param.value[ID::phaserOffset]->getFloat());

  auto phaserStage = param.value[ID::phaserStage]->getInt();
  phaser[0].setStage(phaserStage);
  phaser[1].setStage(phaserStage);

  nVoice = size_t(1) << param.value[ID::nVoice]->getInt();
  if (nVoice > notes.size()) nVoice = notes.size();
}

void DSPCORE_NAME::process(const size_t length, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> frame{};
  for (uint32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    frame.fill(0.0f);

    for (auto &note : notes) {
      if (note.state == NoteState::rest) continue;
      auto noteOut = note.process();
      frame[0] += noteOut[0];
      frame[1] += noteOut[1];
    }

    if (isTransitioning) {
      frame[0] += transitionBuffer[trIndex][0];
      frame[1] += transitionBuffer[trIndex][1];
      transitionBuffer[trIndex].fill(0.0f);
      trIndex = (trIndex + 1) % transitionBuffer.size();
      if (trIndex == trStop) isTransitioning = false;
    }

    const auto phaserFreq = interpPhaserFrequency.process();
    const auto phaserFeedback = interpPhaserFeedback.process();
    const auto phaserRange = interpPhaserRange.process();
    const auto phaserMin = interpPhaserMin.process();
    const auto phaserPhase = interpPhaserPhase.process();
    const auto phaserOffset = interpPhaserOffset.process();
    phaser[0].setup(phaserPhase, phaserFreq, phaserFeedback, phaserRange, phaserMin);
    phaser[1].setup(
      phaserPhase + phaserOffset, phaserFreq, phaserFeedback, phaserRange, phaserMin);

    const auto phaserMix = interpPhaserMix.process();
    frame[0] += phaserMix * (phaser[0].process(frame[0]) - frame[0]);
    frame[1] += phaserMix * (phaser[1].process(frame[1]) - frame[1]);

    const auto masterGain = interpMasterGain.process();
    out0[i] = masterGain * frame[0];
    out1[i] = masterGain * frame[1];
  }
}

void DSPCORE_NAME::noteOn(int32_t identifier, int16_t pitch, float tuning, float velocity)
{
  if (param.value[ParameterID::randomRetrigger]->getInt())
    rng.setSeed(param.value[ParameterID::seed]->getInt());

  size_t nUnison = param.value[ParameterID::unison]->getInt() ? 2 : 1;

  std::vector<size_t> indices(nVoice);
  std::iota(indices.begin(), indices.end(), 0);

  std::vector<size_t> noteIndices;

  size_t index = 0;
  while (index < indices.size() && noteIndices.size() < nUnison) {
    const auto ndx = indices[index];
    if (notes[ndx].id == identifier || notes[ndx].state == NoteState::rest) {
      noteIndices.push_back(ndx);
      indices.erase(indices.begin() + index);
    } else {
      index += 1;
    }
  }

  if (noteIndices.size() < nUnison) {
    std::sort(indices.begin(), indices.end(), [&](size_t left, size_t right) {
      return notes[left].osc.getDecayGain() < notes[right].osc.getDecayGain();
    });

    for (size_t ndx = 0; ndx < indices.size(); ++ndx) {
      fillTransitionBuffer(indices[ndx]);
      noteIndices.push_back(indices[ndx]);
      if (noteIndices.size() >= nUnison) break;
    }
  }

  for (size_t unison = 0; unison < nUnison; ++unison) {
    if (noteIndices.size() <= unison) break;
    const size_t ndx = noteIndices[unison];

    auto normalizedKey = float(pitch) / 127.0f;
    lastNoteFreq = midiNoteToFrequency(
      pitch, tuning, param.value[ParameterID::pitchBend]->getFloat());
    auto pan = nUnison == 1 ? 0.5 : unison / float(nUnison - 1);
    notes[ndx].noteOn(identifier, normalizedKey, lastNoteFreq, velocity, pan, param, rng);

    // Band-aid solution. Hope this won't overlap the id provided by DAW.
    identifier += 1024;
  }
}

void DSPCORE_NAME::fillTransitionBuffer(size_t noteIndex)
{
  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    if (notes[noteIndex].state == NoteState::rest) {
      trStop = trIndex + bufIdx;
      if (trStop >= transitionBuffer.size()) trStop -= transitionBuffer.size();
      break;
    }

    auto frame = notes[noteIndex].process();
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += frame[0] * interp;
    transitionBuffer[idx][1] += frame[1] * interp;
  }
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
