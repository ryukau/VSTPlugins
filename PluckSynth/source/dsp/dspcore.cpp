// (c) 2022 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

#ifdef USE_VECTORCLASS
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
#else
  #define NOTE_NAME Note_Plain
  #define DSPCORE_NAME DSPCore_Plain
#endif

constexpr float maxDelayTime = 4.0f;
constexpr float defaultTempo = 120.0f;

inline float calcMasterPitch(
  int_fast32_t octave,
  int_fast32_t semi,
  int_fast32_t milli,
  float bend,
  float equalTemperament)
{
  return equalTemperament * octave + semi + milli / 1000.0f + (bend - 0.5f) * 4.0f;
}

inline float
semitoneToHz(float notePitch, float equalTemperament = 12.0f, float a4Hz = 440.0f)
{
  return a4Hz * std::exp2((notePitch - 69.0f) / equalTemperament);
}

DSPCORE_NAME::DSPCORE_NAME()
{
  unisonPan.reserve(maxVoice);
  noteIndices.reserve(maxVoice);
  voiceIndices.reserve(maxVoice);
}

void NOTE_NAME::setup(float sampleRate) { fdn.setup(sampleRate, maxDelayTime); }

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  upRate = upFold * this->sampleRate;

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(0.01f);

  info.synchronizer.reset(upRate, defaultTempo, 1.0f);

  // 10 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(upRate * 0.005), {0.0f, 0.0f});

  for (auto &note : notes) note.setup(upRate);

  reset();
}

void NOTE_NAME::reset()
{
  rest();
  id = -1;
  gain = 0;
  releaseSwitch = 1.0f;
  fdn.reset();
}

void DSPCORE_NAME::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.reset(param);

  for (auto &note : notes) note.reset();

  interpMasterGain.reset(pv[ID::gain]->getFloat());

  for (auto &hb : halfbandIir) hb.reset();

  for (auto &frame : transitionBuffer) frame.fill(0.0f);
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;

  panCounter = 0;
}

float DSPCORE_NAME::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getFloat();
  if (lfoRate > Scales::lfoRate.getMax()) return 0.0f;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto &&upper = pv[ID::lfoTempoUpper]->getFloat() + float(1);
  auto &&lower = pv[ID::lfoTempoLower]->getFloat() + float(1);
  return float(4) * upper / lower / lfoRate;
}

void DSPCORE_NAME::startup()
{
  info.synchronizer.reset(upRate, tempo, getTempoSyncInterval());
}

#define SET_NOTE_FILTER_CUTOFF(METHOD)                                                   \
  /* To prevent blow up, upper limit of cutoff is set to slightly below Nyquist          \
   * frequency. */                                                                       \
  auto nyquist = float(0.49998) * sampleRate;                                            \
  auto lowpassCutoffHz = pv[ID::lowpassKeyFollow]->getInt()                              \
    ? semitoneToHz(                                                                      \
      pv[ID::lowpassCutoffSemi]->getFloat() + float(69), float(12), fdnFreq)             \
    : semitoneToHz(pv[ID::lowpassCutoffSemi]->getFloat());                               \
  fdnLowpassCutoff.METHOD(std::clamp(lowpassCutoffHz, 1.0f, nyquist) / sampleRate);      \
                                                                                         \
  auto highpassCutoffHz = pv[ID::highpassKeyFollow]->getInt()                            \
    ? semitoneToHz(                                                                      \
      pv[ID::highpassCutoffSemi]->getFloat() + float(69), float(12), fdnFreq)            \
    : semitoneToHz(pv[ID::highpassCutoffSemi]->getFloat());                              \
  fdnHighpassCutoff.METHOD(std::clamp(highpassCutoffHz, 1.0f, nyquist) / sampleRate);

void NOTE_NAME::setParameters(
  float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;
  gate.prepare(sampleRate, pv[ID::gateRelease]->getFloat());

  fdn.delay.rate = pv[ID::fdnInterpRate]->getFloat();
  auto fdnInterpLowpassSecond = pv[ID::fdnInterpLowpassSecond]->getFloat();
  fdn.delay.kp = fdnInterpLowpassSecond == 0
    ? float(1)
    : float(EMAFilter<double>::cutoffToP(sampleRate, 1.0 / fdnInterpLowpassSecond));
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;
  SET_NOTE_FILTER_CUTOFF(push);
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.setParameters(param);

  for (auto &note : notes) {
    if (note.state == NoteState::rest) continue;
    note.setParameters(upRate, info, param);
  }

  if (prepareRefresh || (!isWavetableRefeshed && pv[ID::refreshWavetable]->getInt())) {
    info.wavetable.param.denominatorSlope
      = pv[ID::oscSpectrumDenominatorSlope]->getFloat();
    info.wavetable.param.rotationSlope = pv[ID::oscSpectrumRotationSlope]->getFloat();
    info.wavetable.param.rotationOffset = pv[ID::oscSpectrumRotationOffset]->getFloat();
    info.wavetable.param.interval = 1 + pv[ID::oscSpectrumInterval]->getInt();
    info.wavetable.param.highpassIndex = pv[ID::oscSpectrumHighpass]->getInt();
    info.wavetable.param.blur = pv[ID::oscSpectrumBlur]->getFloat();
    for (size_t idx = 0; idx < oscOvertoneSize; ++idx) {
      info.wavetable.param.overtoneAmp[idx] = std::polar(
        pv[ID::oscOvertone0 + idx]->getFloat(),
        float(pi) * pv[ID::oscRotation0 + idx]->getFloat());
    }
    info.wavetable.fillTable(upRate);
  }
  isWavetableRefeshed = pv[ID::refreshWavetable]->getInt();
  prepareRefresh = false;

  interpMasterGain.push(pv[ID::gain]->getFloat());
}

inline float alignModValue(float amount, float alignment, float value)
{
  if (alignment == 0) return amount * value;
  return alignment * std::floor(value * amount / alignment + 0.5f);
}

std::array<float, 2> NOTE_NAME::process(float sampleRate, NoteProcessInfo &info)
{
  constexpr auto eps = std::numeric_limits<float>::epsilon();

  if (state == NoteState::release) {
    if (gain <= eps) state = NoteState::rest;
  }
  if (state == NoteState::rest) return {float(0), float(0)};

  fdnLowpassCutoff.process();
  fdnHighpassCutoff.process();

  auto modenv = info.envelope.process(modEnvelopePhase.process());
  auto modenvToOsc = modenv * info.modEnvelopeToOscPitch.getValue();
  auto modenvToFdn = modenv * info.modEnvelopeToFdnPitch.getValue();
  auto modenvToOscLfo
    = float(1) + (modenv - float(1)) * info.modEnvelopeToLfoToPOscPitch.getValue();
  auto modenvToFdnLfo
    = float(1) + (modenv - float(1)) * info.modEnvelopeToLfoToPFdnPitch.getValue();
  auto modenvToFdnLp
    = (modenv + float(1)) * info.modEnvelopeToFdnLowpassCutoff.getValue();
  auto modenvToFdnHp
    = (modenv + float(1)) * info.modEnvelopeToFdnHighpassCutoff.getValue();

  auto lfoValue = info.lfo.process(lfoPhase.process(info.lfoPhase));
  auto lfoToOsc = alignModValue(
    info.lfoToOscPitchAmount.getValue(), info.lfoToOscPitchAlignment,
    modenvToOscLfo * lfoValue);
  auto lfoToFdn = alignModValue(
    info.lfoToFdnPitchAmount.getValue(), info.lfoToFdnPitchAlignment,
    modenvToFdnLfo * lfoValue);

  auto oscPitchMod = (modenvToOsc + lfoToOsc) * float(12) / info.eqTemp;
  auto fdnPitchMod = noteToPitch(modenvToFdn + lfoToFdn, info.eqTemp);

  float sig = 0;

  auto oscGain = envelope.process();
  if (oscGain >= eps) {
    auto nt = oscPitchMod + oscNote + info.oscNoteOffset.getValue();
    sig = oscGain * osc.process(sampleRate, nt, info.wavetable);
  }

  if (info.fdnEnable) {
    auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch * fdnPitchMod;
    float overtone = 1.0f;
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      fdn.delay.setDelayTimeAt(
        idx, sampleRate,
        info.fdnOvertoneOffset.getValue()
          + (float(1) + overtoneRandomness[idx]) * overtone,
        fdnFreq);
      auto ot
        = overtone * info.fdnOvertoneMul.getValue() + info.fdnOvertoneAdd.getValue();
      if (info.fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
        // Almost same operation as `std::fmod()`.
        ot /= info.fdnOvertoneModulo.getValue();
        ot -= std::floor(ot);
        ot *= info.fdnOvertoneModulo.getValue();
      }
      overtone = ot;
    }

    constexpr auto minCutoff = float(0.00001);
    constexpr auto nyquist = float(0.49998);
    fdn.lowpass.setCutoff(
      std::clamp(modenvToFdnLp * fdnLowpassCutoff.getValue(), minCutoff, nyquist),
      info.fdnLowpassQ.getValue());
    fdn.highpass.setCutoff(
      std::clamp(modenvToFdnHp * fdnHighpassCutoff.getValue(), minCutoff, nyquist),
      info.fdnHighpassQ.getValue());

    // TODO: FDN gain.
    sig = float(0.01 * pi) * fdn.process(sig, info.fdnFeedback.getValue());
  }

  auto gateGain = gate.process();
  sig *= gateGain * velocity;
  gain = velocity * releaseSwitch * gateGain;

  return {(float(1) - pan) * sig, pan * sig};
}

void DSPCORE_NAME::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  info.synchronizer.prepare(
    upRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(), beatsElapsed,
    !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    halfIn.fill({});

    for (size_t j = 0; j < upFold; ++j) {
      info.process();

      for (auto &note : notes) {
        if (note.state == NoteState::rest) continue;
        auto sig = note.process(upRate, info);
        halfIn[0][j] += sig[0];
        halfIn[1][j] += sig[1];
      }

      if (isTransitioning) {
        halfIn[0][j] += transitionBuffer[trIndex][0];
        halfIn[1][j] += transitionBuffer[trIndex][1];
        transitionBuffer[trIndex].fill(0.0f);
        trIndex = (trIndex + 1) % transitionBuffer.size();
        if (trIndex == trStop) isTransitioning = false;
      }

      const auto masterGain = interpMasterGain.process();
      halfIn[0][j] *= masterGain;
      halfIn[1][j] *= masterGain;
    }

    out0[i] = halfbandIir[0].process(halfIn[0]);
    out1[i] = halfbandIir[1].process(halfIn[1]);
  }
}

void NOTE_NAME::noteOn(
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

  this->pan = pan;

  this->velocity = velocity;
  gain = 1.0f;
  releaseSwitch = 1.0f;
  gate.reset();

  lfoPhase.offset = pv[ID::lfoRetrigger]->getInt() ? -info.synchronizer.getPhase() : 0;
  modEnvelopePhase.noteOn(sampleRate, pv[ID::modEnvelopeTime]->getFloat());

  // Pitch.
  const auto eqTemp = pv[ID::equalTemperament]->getFloat() + 1;

  fdnPitch = calcNotePitch(notePitch, eqTemp);
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;

  oscNote = float(12) / eqTemp * (notePitch - 69) + 69;

  // Oscillator.
  osc.reset();
  envelope.noteOn(
    pv[ID::oscGain]->getFloat(), sampleRate * pv[ID::oscAttack]->getFloat(),
    sampleRate * pv[ID::oscDecay]->getFloat());

  // FDN matrix.
  std::uniform_int_distribution<unsigned> seedDist{
    0, std::numeric_limits<unsigned>::max()};
  fdn.reset();
  fdn.randomOrthogonal(
    seedDist(info.fdnRng), pv[ID::fdnMatrixIdentityAmount]->getFloat(),
    pv[ID::fdnRandomizeRatio]->getFloat(), info.fdnMatrixRandomBase);
  fdn.process(pv[ID::impulseGain]->getFloat(), 1.0f);

  // FDN delay.
  fdn.delay.rate = pv[ID::fdnInterpRate]->getFloat();
  auto fdnInterpLowpassSecond = pv[ID::fdnInterpLowpassSecond]->getFloat();
  fdn.delay.kp = fdnInterpLowpassSecond == 0
    ? float(1)
    : float(EMAFilter<double>::cutoffToP(sampleRate, 1.0 / fdnInterpLowpassSecond));

  std::uniform_real_distribution<float> overtoneDist(-1.0, 1.0);
  for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
    overtoneRandomness[idx]
      = overtoneDist(info.fdnRng) * pv[ID::fdnOvertoneRandomness]->getFloat();
  }

  float overtone = 1.0f;
  for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
    fdn.delay.resetDelayTimeAt(
      idx, sampleRate,
      info.fdnOvertoneOffset.getValue() + (float(1) + overtoneRandomness[idx]) * overtone,
      fdnFreq);
    auto ot = overtone * info.fdnOvertoneMul.getValue() + info.fdnOvertoneAdd.getValue();
    if (info.fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
      ot /= info.fdnOvertoneModulo.getValue();
      ot -= std::floor(ot);
      ot *= info.fdnOvertoneModulo.getValue();
    }
    overtone = ot;
  }

  // FDN feedback filters.
  SET_NOTE_FILTER_CUTOFF(reset);
  fdn.lowpass.setCutoff(fdnLowpassCutoff.getValue(), info.fdnLowpassQ.getValue());
  fdn.highpass.setCutoff(fdnHighpassCutoff.getValue(), info.fdnHighpassQ.getValue());
}

void NOTE_NAME::release(float sampleRate)
{
  if (state == NoteState::rest) return;
  state = NoteState::release;

  gate.release();

  // Prevent tie-break at note stealing when feedback is 1.0.
  releaseSwitch *= float(1) - std::numeric_limits<float>::epsilon();
}

void NOTE_NAME::rest() { state = NoteState::rest; }

bool NOTE_NAME::isAttacking() { return envelope.isAttacking(); }

float NOTE_NAME::getGain() { return gain; }

void DSPCORE_NAME::noteOn(
  int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  // // Round robin note stealing for debug.
  // auto &targetNote = notes[noteOnIndex];
  // if (targetNote.state != NoteState::rest) fillTransitionBuffer(noteOnIndex);
  // targetNote.noteOn(noteId, float(pitch) + tuning, velocity, 0.5f, upRate, info,
  // param); if (++noteOnIndex >= maxVoice) noteOnIndex = 0;

  const size_t nUnison = 1 + pv[ID::nUnison]->getInt();

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

  unisonPan.resize(nUnison);
  const auto unisonPanRange = param.value[ID::unisonPan]->getFloat();
  const float panRange = unisonPanRange / float(nUnison - 1);
  const float panOffset = 0.5f - 0.5f * unisonPanRange;
  panCounter = (panCounter + 1) % unisonPan.size();
  for (size_t idx = 0; idx < unisonPan.size(); ++idx) {
    auto pan = panRange * ((idx + panCounter) % unisonPan.size()) + panOffset;
    unisonPan[idx] = std::clamp(pan, 0.0f, 1.0f);
  }

  std::array<float, 4> intervals{
    pv[ID::unisonIntervalSemitone0 + 0]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 1]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 2]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 3]->getFloat(),
  };
  const size_t cycleAt = pv[ID::unisonIntervalCycleAt]->getInt();
  const auto eqTemp = pv[ID::unisonEqualTemperament]->getFloat() + float(1);
  const auto pitchMul = param.value[ID::unisonPitchMul]->getFloat();
  size_t intervalIndex = 0;
  float sumInterval = 0;
  for (size_t unison = 0; unison < nUnison; ++unison) {
    if (noteIndices.size() <= unison) break;

    auto notePitch
      = (float(pitch) + tuning + sumInterval * pitchMul * float(12) / eqTemp);
    sumInterval += intervals[intervalIndex];
    if (++intervalIndex > cycleAt) intervalIndex = 0;

    notes[noteIndices[unison]].noteOn(
      noteId, notePitch, velocity / nUnison, unisonPan[unison], upRate, info, param);
  }
}

void DSPCORE_NAME::noteOff(int_fast32_t noteId)
{
  for (auto &note : notes)
    if (note.id == noteId) note.release(upRate);
}

void DSPCORE_NAME::fillTransitionBuffer(size_t noteIndex)
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
