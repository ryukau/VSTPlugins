// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

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

constexpr float delayMaxTime = 1.0f;

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float calcMasterPitch(int32_t octave, int32_t semi, int32_t milli, float bend)
{
  return 12 * octave + semi + milli / 1000.0f + (bend - 0.5f) * 4.0f;
}

inline float calcDelayPitch(int32_t semi, int32_t milli, float equalTemperament)
{
  return powf(2.0f, -(semi + 0.001f * milli) / equalTemperament);
}

inline float
notePitchToFrequency(float notePitch, float equalTemperament, float a4Hz = 440.0f)
{
  return a4Hz * powf(2.0f, (notePitch - 69.0f) / equalTemperament);
}

// Approximation of `440 * powf(2, x * 10 - 5.75);`.
// x in [0.0, 1.0].
inline float mapCutoff(float x)
{
  return (float(2.358608708691953) + float(12.017595467921483) * x
          + float(12.200543970909193) * x * x + float(65.1589512791118) * x * x * x)
    / (float(0.2835017917034045) + float(-0.6282361955447577) * x
       + float(0.4872433705867005) * x * x + float(-0.13155292689641543) * x * x * x);
}

void NOTE_NAME::setup(float sampleRate) { delay.setup(sampleRate, 0.0f, delayMaxTime); }

void NOTE_NAME::noteOn(
  int32_t noteId,
  float notePitch,
  float velocity,
  float pan,
  float phase,
  float sampleRate,
  Wavetable<tableSize> &wavetable,
  NoteProcessInfo &info,
  GlobalParameter &param)
{
  using ID = ParameterID::ID;

  state = NoteState::active;
  id = noteId;

  this->velocity = velocity;
  this->pan = pan;
  gain = 1.0f;

  noteFreq = notePitchToFrequency(
    notePitch + info.masterPitch.getValue(), info.equalTemperament.getValue(),
    info.pitchA4Hz.getValue());

  wavetable.refreshTable(noteFreq, osc.table);
  osc.setFrequency(noteFreq, wavetable.tableBaseFreq);

  if (param.value[ID::oscPhaseReset]->getInt()) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    const auto phaseRnd
      = param.value[ID::oscPhaseRandom]->getInt() ? dist(info.rng) : 1.0f;
    osc.setPhase(phase + phaseRnd * param.value[ID::oscInitialPhase]->getFloat());
  }

  filter.reset();

  delay.reset();
  delaySeconds = 1.0f / noteFreq;
  while (delaySeconds > delayMaxTime) delaySeconds *= 0.5f;

  gainEnvelope.reset(
    sampleRate, param.value[ID::gainA]->getFloat(), param.value[ID::gainD]->getFloat(),
    param.value[ID::gainS]->getFloat(), param.value[ID::gainR]->getFloat(),
    param.value[ID::gainCurve]->getFloat(), noteFreq);
  filterEnvelope.reset(
    sampleRate, param.value[ID::filterA]->getFloat(),
    param.value[ID::filterD]->getFloat(), param.value[ID::filterS]->getFloat(),
    param.value[ID::filterR]->getFloat(), noteFreq);
  delayGate.reset(sampleRate, param.value[ID::delayAttack]->getFloat(), noteFreq);
}

void NOTE_NAME::release()
{
  if (state == NoteState::rest) return;
  state = NoteState::release;
  gainEnvelope.release();
  filterEnvelope.release();
}

void NOTE_NAME::rest() { state = NoteState::rest; }

bool NOTE_NAME::isAttacking() { return gainEnvelope.isAttacking(); }

float NOTE_NAME::getGain() { return gain; }

std::array<float, 2> NOTE_NAME::process(float sampleRate, NoteProcessInfo &info)
{
  gain = velocity * gainEnvelope.process();
  if (gainEnvelope.isTerminated()) state = NoteState::rest;

  const auto oscOut = osc.process();

  const auto cutAmt = info.filterAmount.getValue();
  const auto cutoff = std::clamp(
    info.filterCutoff.getValue() + info.filterKeyFollow.getValue() * noteFreq
      + mapCutoff(cutAmt * filterEnvelope.process()),
    0.0f, 22000.0f);
  const auto filterOut
    = filter.process(oscOut, sampleRate, cutoff, info.filterResonance.getValue());

  delay.setTime(sampleRate, delaySeconds * info.delayDetune.getValue() * info.lfoOut);
  const auto delayOut
    = delay.process(delayGate.process() * filterOut, info.delayFeedback.getValue());

  const auto mix = filterOut + info.delayMix.getValue() * (delayOut - filterOut);

  const auto gain1 = gain * pan;
  const auto gain0 = gain - gain1;
  return {gain0 * mix, gain1 * mix};
}

DSPCORE_NAME::DSPCORE_NAME()
{
  unisonPan.reserve(maxVoice);
  noteIndices.reserve(maxVoice);
  voiceIndices.reserve(maxVoice);

  peakInfos.resize(nOvertone);
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (auto &note : notes) note.setup(sampleRate);

  // 2 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(sampleRate * 0.01), {0.0f, 0.0f});

  startup();
  refreshTable();
  refreshLfo();
}

void DSPCORE_NAME::reset()
{
  for (auto &note : notes) note.rest();
  info.reset();
  startup();
}

void DSPCORE_NAME::startup() { info.rng.seed(param.value[ParameterID::seed]->getInt()); }

void DSPCORE_NAME::setParameters(float tempo)
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpMasterGain.push(param.value[ID::gain]->getFloat());

  info.masterPitch.push(calcMasterPitch(
    int32_t(param.value[ID::oscOctave]->getInt()) - 12,
    param.value[ID::oscSemi]->getInt() - 120, param.value[ID::oscMilli]->getInt() - 1000,
    param.value[ID::pitchBend]->getFloat()));

  auto equalTemperament = param.value[ID::equalTemperament]->getFloat() + 1;
  info.equalTemperament.push(equalTemperament);
  info.pitchA4Hz.push(param.value[ID::pitchA4Hz]->getFloat() + 100);

  info.filterCutoff.push(param.value[ID::filterCutoff]->getFloat());
  info.filterResonance.push(param.value[ID::filterResonance]->getFloat());
  info.filterAmount.push(param.value[ID::filterAmount]->getFloat());
  info.filterKeyFollow.push(param.value[ID::filterKeyFollow]->getFloat());

  info.delayMix.push(param.value[ID::delayMix]->getFloat());
  info.delayDetune.push(calcDelayPitch(
    param.value[ID::delayDetuneSemi]->getInt() - 120,
    param.value[ID::delayDetuneMilli]->getInt() - 1000, equalTemperament));
  info.delayFeedback.push(param.value[ID::delayFeedback]->getFloat());

  const float beat = float(param.value[ID::lfoTempoNumerator]->getInt() + 1)
    / float(param.value[ID::lfoTempoDenominator]->getInt() + 1);
  info.lfoFrequency.push(
    param.value[ID::lfoFrequencyMultiplier]->getFloat() * tempo / 240.0f / beat);
  info.lfoAmount.push(param.value[ID::lfoDelayAmount]->getFloat());
  info.lfoLowpass.push(
    PController<float>::cutoffToP(sampleRate, param.value[ID::lfoLowpass]->getFloat()));

  nVoice = 16 * (param.value[ID::nVoice]->getInt() + 1);
  if (nVoice > notes.size()) nVoice = notes.size();
}

void DSPCORE_NAME::process(const size_t length, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> frame{};
  for (uint32_t i = 0; i < length; ++i) {
    SmootherCommon<float>::setBufferIndex(i);
    processMidiNote(i);

    info.process(sampleRate, lfoWavetable);

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

void DSPCORE_NAME::setUnisonPan(size_t nUnison)
{
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

  using ID = ParameterID::ID;

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

  if (nUnison <= 1) {
    notes[noteIndices[0]].noteOn(
      identifier, float(pitch) + tuning, velocity, 0.5f, 0.0f, sampleRate, wavetable,
      info, param);
    return;
  }

  setUnisonPan(nUnison);

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
      sampleRate, wavetable, info, param);
  }
}

void DSPCORE_NAME::fillTransitionBuffer(size_t noteIndex)
{
  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  auto &note = notes[noteIndex];

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    if (note.state == NoteState::rest) {
      trStop = trIndex + bufIdx;
      if (trStop >= transitionBuffer.size()) trStop -= transitionBuffer.size();
      break;
    }

    auto oscOut = note.process(sampleRate, info);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += oscOut[0] * interp;
    transitionBuffer[idx][1] += oscOut[1] * interp;
  }
}

void DSPCORE_NAME::noteOff(int32_t noteId)
{
  for (size_t i = 0; i < notes.size(); ++i)
    if (notes[i].id == noteId) notes[i].release();
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

  for (size_t idx = 0; idx < peakInfos.size(); ++idx) {
    peakInfos[idx].frequency = (pitchMultiplier * idx + 1.0f) * tableBaseFreq
      * param.value[ID::overtonePitch0 + idx]->getFloat();
    if (pitchModulo != 0) {
      peakInfos[idx].frequency = fmodf(
        peakInfos[idx].frequency, notePitchToFrequency(pitchModulo, 12.0f, 440.0f));
    }

    peakInfos[idx].gain = powf(param.value[ID::overtoneGain0 + idx]->getFloat(), gainPow);
    peakInfos[idx].bandWidth
      = widthMul * param.value[ID::overtoneWidth0 + idx]->getFloat();
    peakInfos[idx].phase = param.value[ID::overtonePhase0 + idx]->getFloat();
  }

  wavetable.padsynth(
    sampleRate, tableBaseFreq, peakInfos, param.value[ID::padSynthSeed]->getInt(),
    param.value[ID::spectrumExpand]->getFloat(),
    int32_t(param.value[ID::spectrumShift]->getInt()) - spectrumSize,
    param.value[ID::profileComb]->getInt() + 1, param.value[ID::profileShape]->getFloat(),
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
