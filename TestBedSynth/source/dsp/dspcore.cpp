// (c) 2023 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

inline float calcNotePitch(float notePitch, float equalTemperament = 12.0f)
{
  return std::exp2((notePitch - 69.0f) / equalTemperament);
}

void Note::setup(float sampleRate)
{
  for (auto &osc : oscillator) osc.setup(sampleRate);
}

#define ASSIGN_NOTE_PARAMETER(METHOD)                                                    \
  for (size_t idx = 0; idx < nOscillator; ++idx) {                                       \
    lfoPitch[idx].METHOD(                                                                \
      std::exp2(pv[ID::lfoKeyFollow0 + idx]->getFloat() * notePitch / info.eqTemp));     \
  }                                                                                      \
                                                                                         \
  for (size_t i0 = 0; i0 < nOscillator; ++i0) {                                          \
    for (size_t i1 = 0; i1 < nOscWavetable; ++i1) {                                      \
      size_t offset = nOscWavetable * i0 + i1;                                           \
      waveModDelay[i0][i1].setFrames(pv[ID::waveMod0Delay0 + offset]->getInt());         \
    }                                                                                    \
  }

void Note::setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (state == NoteState::rest) return;

  ASSIGN_NOTE_PARAMETER(push);

  gainEnvelope.prepare(info.gainAttackKp, info.gainDecayKp, info.gainReleaseKp);
  for (size_t idx = 0; idx < nOscillator; ++idx) {
    envelope[idx].prepare(
      info.envAttackKp[idx], info.envDecayKp[idx], info.envReleaseKp[idx]);
  }
}

void Note::noteOn(
  int_fast32_t noteId,
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

  this->notePitch = notePitch;
  this->velocity = velocity;
  this->pan = pan;

  noteHz = std::min(
    float(440) * calcNotePitch(notePitch, info.eqTemp), info.tableParam[0].baseNyquistHz);

  ASSIGN_NOTE_PARAMETER(reset);

  modulation.fill({});
  feedback.fill({});

  gainEnvelope.noteOn(size_t(sampleRate * pv[ID::gainAttackSecond]->getFloat()));
  for (size_t idx = 0; idx < nOscillator; ++idx) {
    envelope[idx].noteOn(
      size_t(sampleRate * pv[ID::envelopeAttackSecond0 + idx]->getFloat()));
    oscillator[idx].noteOn(
      noteHz, modulation, info.oscWavetable[idx].value, info.tableParam[idx]);
  }
}

void Note::noteOff(float sampleRate)
{
  if (state == NoteState::rest) return;
  state = NoteState::release;

  gainEnvelope.noteOff();
  for (size_t i = 0; i < nOscillator; ++i) envelope[i].noteOff();
}

void Note::rest()
{
  state = NoteState::rest;
  id = -1;
}

void Note::reset()
{
  rest();

  modulation.fill({});
  feedback.fill({});
  gainEnvelope.reset();
  for (auto &x : envelope) x.reset();
  for (auto &x : lfoPitch) x.reset();
  for (auto &x : lfo) x.reset();
  for (auto &ch : waveModDelay) {
    for (auto &x : ch) x.reset();
  }
  wavetable.fill({});
  for (auto &x : oscillator) x.reset();
}

bool Note::isAttacking() { return gainEnvelope.isAttacking(); }

float Note::getGain() { return velocity * gainEnvelope.value(); }

std::array<float, 2> Note::process(float sampleRate, NoteProcessInfo &info)
{
  if (state == NoteState::rest) return {0.0f, 0.0f};

  gainEnvelope.process(info.gainSustainAmplitude.getValue());

  constexpr auto eps = std::numeric_limits<float>::epsilon();
  if (gainEnvelope.isTerminated()) {
    state = NoteState::rest;
    return {0.0f, 0.0f};
  }

  for (size_t i0 = 0; i0 < nOscillator; ++i0) {
    modulation[ModID::env0 + i0]
      = envelope[i0].process(info.envelopeSustainAmplitude[i0].getValue());
    modulation[ModID::lfo0 + i0] = lfo[i0].process(
      lfoPitch[i0].process() * info.lfoPhaseDelta[i0].getValue(),
      info.lfoLowpassKp[i0].getValue(), info.lfoWavetable[i0].value);
    modulation[ModID::ext0 + i0] = info.externalInput[i0].getValue();
  }

  for (size_t i0 = 0; i0 < nOscillator; ++i0) {
    const auto &wmIn = info.waveModInput[i0];
    if (oscillator[i0].isRefreshing()) {
      const auto sum = std::accumulate(
        wmIn.begin(), wmIn.end(), 0.0f, [](float x, float y) { return x + std::abs(y); });
      const auto dot
        = std::inner_product(wmIn.begin(), wmIn.end() - 1, modulation.begin(), 0.0f);
      const auto amount = sum < std::numeric_limits<float>::epsilon()
        ? wmIn.back()
        : (wmIn.back() + dot) / sum;

      for (size_t i1 = 0; i1 < nOscWavetable; ++i1) {
        const auto &base = info.oscWavetable[i0].value[i1];
        const auto mod
          = info.oscWaveModGain[i0].value[i1] * waveModDelay[i0][i1].process(amount);
        wavetable[i0][i1] = lerp(base, mod, sum / nWaveModInput);
      }
    }

    feedback[i0] = oscillator[i0].process(
      sampleRate, noteHz * info.mainPitch.getValue(), feedback, modulation, wavetable[i0],
      info.tableParam[i0]);
  }

  float sig = gainEnvelope.value() * velocity
    * lerp(feedback[0], feedback[1], info.oscMix.getValue());
  return {(1.0f - pan) * sig, pan * sig};
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  transitionBuffer.resize(1 + size_t(this->sampleRate * 0.002), {0.0f, 0.0f});

  for (size_t idx = 0; idx < nOscillator; ++idx) {
    info.tableParam[idx].baseNyquistHz = sampleRate / float(2);
    info.tableParam[idx].upNyquistHz = upRate / float(2);
  }

  reset();
}

DSPCore::DSPCore()
{
  unisonPan.reserve(maxVoice);
  noteIndices.reserve(maxVoice);
  voiceIndices.reserve(maxVoice);
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  nVoice = pv[ID::nVoice]->getInt() + 1;                                                 \
                                                                                         \
  interpMasterGain.METHOD(pv[ID::gain]->getFloat());                                     \
  dcHighpassCutoffKp.METHOD(float(                                                       \
    EMAFilter<double>::cutoffToP(upRate, pv[ID::dcHighpassCutoffHz]->getFloat())));      \
  dcHighpassEnable = pv[ID::dcHighpassEnable]->getInt();

void DSPCore::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  updateSampleRate(true);

  info.reset(upRate, param);

  for (auto &note : notes) note.reset();

  ASSIGN_PARAMETER(reset);

  for (auto &x : dcHighpass) x.reset();
  for (auto &x : downSampler) x.reset();

  for (auto &frame : transitionBuffer) frame.fill(0.0f);
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  updateSampleRate();

  info.setParameters(upRate, param);

  for (auto &note : notes) note.setParameters(upRate, info, param);

  ASSIGN_PARAMETER(push);
}

void DSPCore::processSample(std::array<float, 2> &frame)
{
  frame.fill({});

  for (auto &note : notes) {
    if (note.state == NoteState::rest) continue;
    auto sig = note.process(upRate, info);
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

  const auto dcHighpassKp = dcHighpassCutoffKp.process();
  if (dcHighpassEnable) {
    frame[0] = dcHighpass[0].process(frame[0], dcHighpassKp);
    frame[1] = dcHighpass[1].process(frame[1], dcHighpassKp);
  }

  const auto masterGain = interpMasterGain.process();
  frame[0] *= masterGain;
  frame[1] *= masterGain;
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));

  size_t oversampling = pv[ID::oversampling]->getInt();

  std::array<float, 2> frame{};
  for (uint_fast32_t i = 0; i < length; ++i) {
    processMidiNote(i);

    info.process();

    if (oversampling == 2) { // 16x.
      for (size_t j = 0; j < downSampler[0].fold; ++j) {
        processSample(frame);
        downSampler[0].inputBuffer[j] = frame[0];
        downSampler[1].inputBuffer[j] = frame[1];
      }
      out0[i] = downSampler[0].process();
      out1[i] = downSampler[1].process();
    } else if (oversampling == 1) { // 2x.
      for (size_t j = 0; j < 2; ++j) {
        processSample(frame);
        downSampler[0].inputBuffer[j] = frame[0];
        downSampler[1].inputBuffer[j] = frame[1];
      }
      out0[i] = downSampler[0].process2x();
      out1[i] = downSampler[1].process2x();
    } else {
      processSample(frame);
      out0[i] = frame[0];
      out1[i] = frame[1];
    }
  }
}

void DSPCore::noteOn(
  int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  const size_t nUnison = 1;

  noteIndices.resize(0);

  // Pick up note from resting one.
  for (size_t index = 0; index < nVoice; ++index) {
    if (notes[index].id == noteId) noteIndices.push_back(index);
    if (notes[index].state == NoteState::rest) noteIndices.push_back(index);
    if (noteIndices.size() >= nUnison) break;
  }

  // If there aren't enough resting note, pick up from most quiet one.
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

  if (nUnison <= 1) {
    notes[noteIndices[0]].noteOn(
      noteId, float(pitch) + tuning, velocity, 0.5f, upRate, info, param);
    return;
  }
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  for (size_t i = 0; i < notes.size(); ++i)
    if (notes[i].id == noteId) notes[i].noteOff(upRate);
}

inline void DSPCore::fillTransitionBuffer(size_t noteIndex)
{
  auto &note = notes[noteIndex];
  if (note.state == NoteState::rest) return;

  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    auto oscOut = note.process(upRate, info);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = 1.0f - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += oscOut[0] * interp;
    transitionBuffer[idx][1] += oscOut[1] * interp;
  }
}

inline void DSPCore::updateSampleRate(bool reset)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto previousRate = upRate;
  auto oversampling = std::min<size_t>(pv[ID::oversampling]->getInt(), fold.size() - 1);
  upRate = float(fold[oversampling] * sampleRate);

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(pv[ID::parameterSmoothingSecond]->getFloat());

  if (!reset && previousRate == upRate) return;

  for (auto &note : notes) note.setup(upRate);
}
