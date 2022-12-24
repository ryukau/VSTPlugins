// (c) 2022 Takamitsu Endo
//
// This file is part of ClangSynth.
//
// ClangSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

constexpr float maxDelayTime = float(4);
constexpr float defaultTempo = float(120);

inline float calcMasterPitch(
  int_fast32_t octave,
  int_fast32_t semi,
  int_fast32_t milli,
  float bend,
  float equalTemperament)
{
  return equalTemperament * octave + semi + milli / float(1000)
    + (bend - float(0.5)) * float(4);
}

inline float
semitoneToHz(float notePitch, float equalTemperament = float(12), float a4Hz = float(440))
{
  return a4Hz * std::exp2((notePitch - float(69)) / equalTemperament);
}

DSPCore::DSPCore()
{
  unisonPan.reserve(maximumVoice);
  noteIndices.reserve(maximumVoice);
  voiceIndices.reserve(maximumVoice);
}

void Note::setup(float sampleRate) { fdn.setup(sampleRate, maxDelayTime); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  upRate = upFold * this->sampleRate;

  SmootherCommon<float>::setSampleRate(upRate);

  info.synchronizer.reset(upRate, defaultTempo, float(1));
  info.smootherKp = float(EMAFilter<double>::cutoffToP(sampleRate, 100));

  // 10 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(upRate * double(0.005)), {float(0), float(0)});

  for (auto &note : notes) note.setup(upRate);

  reset();
}

#define SET_NOTE_FILTER_CUTOFF(METHOD)                                                   \
  /* To prevent blow up, upper limit of cutoff is set to slightly below Nyquist          \
   * frequency. */                                                                       \
  auto nyquist = float(0.49998) * sampleRate;                                            \
  auto lowpassCutoffHz = pv[ID::lowpassKeyFollow]->getInt()                              \
    ? semitoneToHz(                                                                      \
      pv[ID::lowpassCutoffSemi]->getFloat() + float(69), float(12), fdnFreq)             \
    : semitoneToHz(pv[ID::lowpassCutoffSemi]->getFloat());                               \
  fdnLowpassCutoff.METHOD(std::clamp(lowpassCutoffHz, float(1), nyquist) / sampleRate);  \
                                                                                         \
  auto highpassCutoffHz = pv[ID::highpassKeyFollow]->getInt()                            \
    ? semitoneToHz(                                                                      \
      pv[ID::highpassCutoffSemi]->getFloat() + float(69), float(12), fdnFreq)            \
    : semitoneToHz(pv[ID::highpassCutoffSemi]->getFloat());                              \
  fdnHighpassCutoff.METHOD(std::clamp(highpassCutoffHz, float(1), nyquist) / sampleRate);

void Note::reset(float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  state = NoteState::rest;
  id = -1;
  gain = 0;
  releaseSwitch = float(1);

  pan.reset(float(0.5));

  envelope.reset();
  lfoPhase.offset = 0;
  modEnvelopePhase.counter = 0;
  osc.reset();

  fdn.reset();
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;
  SET_NOTE_FILTER_CUTOFF(reset);

  auto gateAttackSecond = pv[ID::gateAttackSecond]->getFloat();
  gateSmoother.setCutoff(
    sampleRate,
    gateAttackSecond < std::numeric_limits<float>::epsilon()
      ? sampleRate
      : float(1) / gateAttackSecond);
  gateSmoother.reset();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  nVoice = pv[ID::nVoice]->getInt() + 1;                                                 \
  if (nVoice > notes.size()) nVoice = notes.size();                                      \
                                                                                         \
  interpMasterGain.METHOD(pv[ID::gain]->getFloat());

void DSPCore::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.reset(param);
  info.synchronizer.reset(upRate, tempo, getTempoSyncInterval());

  ASSIGN_PARAMETER(reset);

  for (auto &note : notes) note.reset(upRate, info, param);
  for (auto &hb : halfbandIir) hb.reset();
  for (auto &frame : transitionBuffer) frame.fill({});
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;

  panCounter = 0;
}

float DSPCore::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getFloat();
  if (lfoRate > Scales::lfoRate.getMax()) return 0;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto &&upper = pv[ID::lfoTempoUpper]->getFloat() + float(1);
  auto &&lower = pv[ID::lfoTempoLower]->getFloat() + float(1);
  return float(4) * upper / lower / lfoRate;
}

void DSPCore::startup()
{
  info.synchronizer.reset(upRate, tempo, getTempoSyncInterval());
}

void Note::setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  gate.prepare(sampleRate, pv[ID::gateReleaseSecond]->getFloat());

  auto gateAttackSecond = pv[ID::gateAttackSecond]->getFloat();
  gateSmoother.setCutoff(
    sampleRate,
    gateAttackSecond < std::numeric_limits<float>::epsilon()
      ? sampleRate
      : float(1) / gateAttackSecond);

  fdn.delay.rate = pv[ID::fdnInterpRate]->getFloat();
  auto fdnInterpLowpassSecond = pv[ID::fdnInterpLowpassSecond]->getFloat();
  fdn.delay.kp = fdnInterpLowpassSecond == 0
    ? float(1)
    : float(EMAFilter<double>::cutoffToP(sampleRate, double(1) / fdnInterpLowpassSecond));
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;
  SET_NOTE_FILTER_CUTOFF(push);
}

void DSPCore::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.setParameters(param);

  ASSIGN_PARAMETER(push);

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
}

inline float alignModValue(float amount, float alignment, float value)
{
  if (alignment == 0) return amount * value;
  return alignment * std::floor(value * amount / alignment + float(0.5));
}

std::array<float, 2> Note::process(float sampleRate, NoteProcessInfo &info)
{
  constexpr auto eps = std::numeric_limits<float>::epsilon();

  if (state == NoteState::rest) return {float(0), float(0)};

  auto modenv = info.envelope.process(modEnvelopePhase.process());
  auto modenvToFdnLp
    = noteToPitch(modenv * info.modEnvelopeToFdnLowpassCutoff.getValue(), info.eqTemp);
  auto modenvToFdnHp
    = noteToPitch(modenv * info.modEnvelopeToFdnHighpassCutoff.getValue(), info.eqTemp);
  auto modenvToOsc = modenv * info.modEnvelopeToOscPitch.getValue();
  auto modenvToFdn = modenv * info.modEnvelopeToFdnPitch.getValue();
  auto modEnvelopeToFdnOvertoneAdd = modenv * info.modEnvelopeToFdnOvertoneAdd.getValue();

  auto lfoValue = info.lfo.process(lfoPhase.process(info.lfoPhase));
  auto lfoToOsc = alignModValue(
    info.lfoToOscPitchAmount.getValue(), info.lfoToOscPitchAlignment, lfoValue);
  auto lfoToFdn = alignModValue(
    info.lfoToFdnPitchAmount.getValue(), info.lfoToFdnPitchAlignment, lfoValue);

  auto oscPitchMod = (modenvToOsc + lfoToOsc) * float(12) / info.eqTemp;
  auto fdnPitchMod = noteToPitch(modenvToFdn + lfoToFdn, info.eqTemp);

  float sig = impulse;
  impulse = 0;

  auto oscGain = envelope.process() * velocity;
  if (oscGain >= eps) {
    auto nt = oscPitchMod + oscNote + info.oscNoteOffset.getValue();
    sig += oscGain * osc.process(sampleRate, nt, info.wavetable);
  }

  if (info.fdnEnable) {
    auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch * fdnPitchMod;
    float overtone = float(1);
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      fdn.delay.setDelayTimeAt(
        idx, sampleRate,
        info.fdnOvertoneOffset.getValue()
          + (float(1) + overtoneRandomness[idx]) * overtone,
        fdnFreq);
      auto ot = overtone * info.fdnOvertoneMul.getValue() + info.fdnOvertoneAdd.getValue()
        + modEnvelopeToFdnOvertoneAdd;
      if (info.fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
        // Almost same operation as `std::fmod()`.
        ot /= float(1) + info.fdnOvertoneModulo.getValue();
        ot -= std::floor(ot);
        ot *= float(1) + info.fdnOvertoneModulo.getValue();
      }
      overtone = ot;
    }

    constexpr auto minCutoff = float(0.00001);
    constexpr auto nyquist = float(0.49998);
    fdn.lowpass.setCutoff(
      std::clamp(
        modenvToFdnLp * fdnLowpassCutoff.process(info.smootherKp), minCutoff, nyquist),
      info.fdnLowpassQ.getValue());
    fdn.highpass.setCutoff(
      std::clamp(
        modenvToFdnHp * fdnHighpassCutoff.process(info.smootherKp), minCutoff, nyquist),
      info.fdnHighpassQ.getValue());

    // TODO: FDN gain.
    sig = float(0.01 * pi) * fdn.process(sig, info.fdnFeedback.getValue());
  }

  auto gateGain = gate.process();
  gain = gateGain * releaseSwitch;
  auto outputGain = gateSmoother.process(gateGain);
  sig *= outputGain;

  if (state == NoteState::release && outputGain <= eps) state = NoteState::rest;

  auto panGain = pan.process(info.smootherKp);
  return {(float(1) - panGain) * sig, panGain * sig};
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setTime(pv[ID::smoothingTimeSecond]->getFloat());
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

  id = noteId;

  this->velocity = velocity;
  gain = float(1);
  releaseSwitch = float(1);

  gate.reset();

  modEnvelopePhase.noteOn(sampleRate, pv[ID::modEnvelopeTime]->getFloat());

  // Pitch.
  const auto eqTemp = pv[ID::equalTemperament]->getFloat() + 1;

  fdnPitch = calcNotePitch(notePitch, eqTemp);
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;

  oscNote = float(12) / eqTemp * (notePitch - 69) + 69;

  // Oscillator.
  impulse = pv[ID::impulseGain]->getFloat();
  envelope.noteOn(
    pv[ID::oscGain]->getFloat(), sampleRate * pv[ID::oscAttack]->getFloat(),
    sampleRate * pv[ID::oscDecay]->getFloat());

  // FDN matrix.
  std::uniform_int_distribution<unsigned> seedDist{
    0, std::numeric_limits<unsigned>::max()};

  fdn.randomOrthogonal(
    seedDist(info.fdnRng), pv[ID::fdnMatrixIdentityAmount]->getFloat(),
    pv[ID::fdnRandomizeRatio]->getFloat(), info.fdnMatrixRandomBase);

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

  // Resetting.
  bool resetAtNoteOn = pv[ID::resetAtNoteOn]->getInt();

  if (resetAtNoteOn || state == NoteState::rest) {
    this->pan.reset(pan);
    gateSmoother.reset();

    osc.reset();
    lfoPhase.offset = pv[ID::lfoRetrigger]->getInt() ? -info.synchronizer.getPhase() : 0;

    fdn.reset();

    float overtone = 1.0f;
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      fdn.delay.resetDelayTimeAt(
        idx, sampleRate,
        info.fdnOvertoneOffset.getValue()
          + (float(1) + overtoneRandomness[idx]) * overtone,
        fdnFreq);
      auto ot
        = overtone * info.fdnOvertoneMul.getValue() + info.fdnOvertoneAdd.getValue();
      if (info.fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
        ot /= float(1) + info.fdnOvertoneModulo.getValue();
        ot -= std::floor(ot);
        ot *= float(1) + info.fdnOvertoneModulo.getValue();
      }
      overtone = ot;
    }

    SET_NOTE_FILTER_CUTOFF(reset);
  } else {
    this->pan.push(pan);
    SET_NOTE_FILTER_CUTOFF(push);
  }

  state = NoteState::active;
}

void Note::release(float sampleRate)
{
  if (state == NoteState::rest) return;
  state = NoteState::release;

  gate.release();

  // Prevent tie-break at note stealing when feedback is 1.0.
  releaseSwitch *= float(1) - std::numeric_limits<float>::epsilon();
}

void Note::rest() { state = NoteState::rest; }

bool Note::isAttacking() { return false; }

float Note::getGain() { return gain; }

void DSPCore::noteOn(
  int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

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
      if (pv[ID::resetAtNoteOn]->getInt()) fillTransitionBuffer(index);
      noteIndices.push_back(index);
      if (noteIndices.size() >= nUnison) break;
    }
  }

  // Parameters must be set after transition buffer is filled.
  velocity = velocityMap.map(velocity);

  if (nUnison <= 1) {
    notes[noteIndices[0]].noteOn(
      noteId, float(pitch) + tuning, velocity, float(0.5), upRate, info, param);
    return;
  }

  unisonPan.resize(nUnison);
  const auto unisonPanRange = param.value[ID::unisonPan]->getFloat();
  const float panRange = unisonPanRange / float(nUnison - 1);
  const float panOffset = float(0.5) - float(0.5) * unisonPanRange;
  if (++panCounter > unisonPan.size()) panCounter = 0;
  for (size_t idx = 0; idx < unisonPan.size(); ++idx) {
    auto pan = panRange * ((idx + panCounter) % unisonPan.size()) + panOffset;
    unisonPan[idx] = std::clamp(pan, float(0), float(1));
  }

  std::array<float, 4> intervals{
    pv[ID::unisonIntervalSemitone0 + 0]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 1]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 2]->getFloat(),
    pv[ID::unisonIntervalSemitone0 + 3]->getFloat(),
  };
  const size_t cycleAt = pv[ID::unisonIntervalCycleAt]->getInt() + 1;
  const auto eqTemp = pv[ID::unisonEqualTemperament]->getFloat() + float(1);
  const auto pitchMul = param.value[ID::unisonPitchMul]->getFloat();
  float sumInterval = 0;
  for (size_t unison = 0; unison < nUnison; ++unison) {
    if (noteIndices.size() <= unison) break;

    auto notePitch = float(pitch) + tuning + sumInterval * pitchMul * float(12) / eqTemp;
    sumInterval += intervals[unison % cycleAt];

    notes[noteIndices[unison]].noteOn(
      noteId, notePitch, velocity / nUnison, unisonPan[unison], upRate, info, param);
  }
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  for (auto &note : notes)
    if (note.id == noteId) note.release(upRate);
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
    auto oscOut = note.process(upRate, info);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = float(1) - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx][0] += oscOut[0] * interp;
    transitionBuffer[idx][1] += oscOut[1] * interp;
  }
}
