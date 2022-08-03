// (c) 2022 Takamitsu Endo
//
// This file is part of ClangCymbal.
//
// ClangCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

constexpr float maxDelayTime = float(4);

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

inline float semitoneToNormalizedFreq(
  float sampleRate, float noteFreq, float semitone, float keyFollow)
{
  auto staticHz = semitoneToHz(semitone);
  auto followHz = semitoneToHz(semitone + float(69), float(12), noteFreq);
  auto hz = staticHz + keyFollow * (followHz - staticHz);
  return hz / sampleRate;
}

DSPCore::DSPCore() {}

void Note::setup(float sampleRate)
{
  fdn.setup(sampleRate, maxDelayTime);
  tremolo.setup(sampleRate, Scales::tremoloDelayTime.getMax());
}

void DSPCore::setup(double sampleRate)
{
  isInitialized = true;

  this->sampleRate = float(sampleRate);
  upRate = upFold * this->sampleRate;

  SmootherCommon<float>::setSampleRate(upRate);

  info.smootherKp = float(EMAFilter<double>::cutoffToP(sampleRate, 100));

  // 10 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(upRate * double(0.005)), {float(0), float(0)});

  note.setup(upRate);

  reset();
}

#define SET_NOTE_FILTER_CUTOFF(METHOD)                                                   \
  auto oscDensityPitch                                                                   \
    = float(1) + pv[ID::oscDensityKeyFollow]->getFloat() * (fdnPitch - float(1));        \
  oscDensity.METHOD(oscDensityPitch *pv[ID::oscDensityHz]->getFloat() / sampleRate);     \
                                                                                         \
  tremoloMix.METHOD(pv[ID::tremoloMix]->getFloat());                                     \
  tremoloDepth.METHOD(pv[ID::tremoloDepth]->getFloat());                                 \
  tremoloDelayTime.METHOD(sampleRate *pv[ID::tremoloDelayTime]->getFloat());             \
  tremoloModToDelayTimeOffset.METHOD(                                                    \
    pv[ID::tremoloModulationToDelayTimeOffset]->getFloat());                             \
  tremoloModSmoothingKp.METHOD(float(EMAFilter<double>::cutoffToP(                       \
    sampleRate, pv[ID::tremoloModulationSmoothingHz]->getFloat())));                     \
                                                                                         \
  auto oscLpCutoff = semitoneToNormalizedFreq(                                           \
    sampleRate, fdnFreq, pv[ID::oscLowpassCutoffSemi]->getFloat(),                       \
    pv[ID::oscLowpassKeyFollow]->getFloat());                                            \
  oscLowpass.METHOD##Cutoff(oscLpCutoff, pv[ID::oscLowpassQ]->getFloat());               \
                                                                                         \
  auto modenv = info.envelope.process(modEnvelopePhase.process());                       \
  auto modenvToFdnLp = noteToPitch(                                                      \
    modenv * pv[ID::modEnvelopeToFdnLowpassCutoff]->getFloat(), info.eqTemp);            \
  auto modenvToFdnHp = noteToPitch(                                                      \
    modenv * pv[ID::modEnvelopeToFdnHighpassCutoff]->getFloat(), info.eqTemp);           \
                                                                                         \
  auto fdnLowpassKeyFollow = pv[ID::fdnLowpassKeyFollow]->getInt();                      \
  auto fdnHighpassKeyFollow = pv[ID::fdnHighpassKeyFollow]->getInt();                    \
  auto fdnLpCutBase = pv[ID::fdnLowpassCutoffSemi]->getFloat();                          \
  auto fdnHpCutBase = pv[ID::fdnHighpassCutoffSemi]->getFloat();                         \
  auto fdnLpCutSlope = pv[ID::fdnLowpassCutoffSlope]->getFloat();                        \
  auto fdnHpCutSlope = pv[ID::fdnHighpassCutoffSlope]->getFloat();                       \
  auto fdnLpQBase = pv[ID::fdnLowpassQ]->getFloat();                                     \
  auto fdnHpQBase = pv[ID::fdnHighpassQ]->getFloat();                                    \
  auto fdnLpQSlope = pv[ID::fdnLowpassQSlope]->getFloat();                               \
  auto fdnHpQSlope = pv[ID::fdnHighpassQSlope]->getFloat();                              \
  for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {                                     \
    auto lpCut = semitoneToNormalizedFreq(                                               \
      sampleRate, fdnFreq,                                                               \
      idx * fdnLpCutSlope + fdnLpCutBase                                                 \
        + pv[ID::fdnLowpassCutoffSemiOffset0 + idx]->getFloat(),                         \
      fdnLowpassKeyFollow);                                                              \
    auto hpCut = semitoneToNormalizedFreq(                                               \
      sampleRate, fdnFreq,                                                               \
      idx * fdnHpCutSlope + fdnHpCutBase                                                 \
        + pv[ID::fdnHighpassCutoffSemiOffset0 + idx]->getFloat(),                        \
      fdnHighpassKeyFollow);                                                             \
    auto lpQ                                                                             \
      = fdnLpQBase + idx * fdnLpQSlope + pv[ID::fdnLowpassQOffset0 + idx]->getFloat();   \
    auto hpQ                                                                             \
      = fdnHpQBase + idx * fdnHpQSlope + pv[ID::fdnHighpassQOffset0 + idx]->getFloat();  \
    fdn.lowpass.METHOD##CutoffAt(                                                        \
      idx, modenvToFdnLp *lpCut, std::clamp(lpQ, float(0.01), float(halfSqrt2)));        \
    fdn.highpass.METHOD##CutoffAt(                                                       \
      idx, modenvToFdnHp *hpCut, std::clamp(hpQ, float(0.01), float(halfSqrt2)));        \
  }

void Note::reset(float sampleRate, NoteProcessInfo &info, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  state = NoteState::rest;
  id = -1;
  gain = 0;
  releaseSwitch = float(1);

  pulsar.reset();
  oscLowpass.reset();
  envelope.reset();
  modEnvelopePhase.counter = 0;

  fdn.reset();
  auto fdnFreq = info.fdnFreqOffset.getValue() * fdnPitch;

  SET_NOTE_FILTER_CUTOFF(reset);

  oscEnvelopeSmoother.setCutoff(sampleRate, float(4000));
  oscEnvelopeSmoother.reset();

  tremolo.reset();
  tremoloSmoother.reset();

  auto gateAttackSecond = pv[ID::gateAttackSecond]->getFloat();
  gateSmoother.setCutoff(
    sampleRate,
    gateAttackSecond < std::numeric_limits<float>::epsilon()
      ? sampleRate
      : float(1) / gateAttackSecond);
  gateSmoother.reset();
}

#define ASSIGN_PARAMETER(METHOD) interpMasterGain.METHOD(pv[ID::gain]->getFloat());

void DSPCore::reset()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  info.reset(param);

  ASSIGN_PARAMETER(reset);

  note.reset(upRate, info, param);
  for (auto &hb : halfbandIir) hb.reset();
  for (auto &frame : transitionBuffer) frame.fill({});
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;
}

void DSPCore::startup() {}

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

  pulsar.setDecay(sampleRate * pv[ID::oscNoiseDecay]->getFloat());

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

  if (note.state != NoteState::rest) note.setParameters(upRate, info, param);
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
  auto modenvToFdn = modenv * info.modEnvelopeToFdnPitch.getValue();
  auto modEnvelopeToFdnOvertoneAdd = modenv * info.modEnvelopeToFdnOvertoneAdd.getValue();

  auto fdnPitchMod = noteToPitch(modenvToFdn, info.eqTemp);

  float sig = impulse;
  impulse = 0;

  auto oscGain = oscEnvelopeSmoother.process(envelope.process() * velocity);
  if (oscGain >= eps) {
    auto densityEnvelope = float(1)
      + info.oscBounce.getValue()
        * (std::pow(oscGain, info.oscBounceCurve.getValue()) - float(1));
    auto density = oscDensity.process();
    auto noise = pulsar.process(
      info.oscJitter.getValue(), density, densityEnvelope,
      info.oscPulseGainRandomness.getValue(), info.fdnRng);
    auto pulse = pulsar.processBlit(density);
    sig += oscGain * (noise + info.oscNoisePulseRatio.getValue() * (pulse - noise));
  }
  sig = oscLowpass.process(sig);
  auto oscOut = sig;

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

    // TODO: FDN gain.
    sig = float(0.01 * pi) * fdn.process(sig, info.fdnFeedback.getValue());
    sig = tremolo.process(
      sig, tremoloSmoother.processKp(oscOut, tremoloModSmoothingKp.process()),
      tremoloModToDelayTimeOffset.process(), tremoloDelayTime.process(),
      tremoloDepth.process(), tremoloMix.process());
  }

  auto gateGain = gate.process();
  gain = gateGain * releaseSwitch;
  auto outputGain = gateSmoother.process(gateGain);
  sig *= outputGain;

  if (state == NoteState::release && outputGain <= eps) state = NoteState::rest;

  return {sig, sig};
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setTime(pv[ID::smoothingTimeSecond]->getFloat());
  SmootherCommon<float>::setBufferSize(float(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    halfIn.fill({});

    for (size_t j = 0; j < upFold; ++j) {
      info.process();

      if (note.state != NoteState::rest) {
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
  pulsar.reset();
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
    oscLowpass.reset();

    oscEnvelopeSmoother.reset();
    tremoloSmoother.reset();
    gateSmoother.reset();

    fdn.reset();
    tremolo.reset();

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

  note.noteOn(noteId, float(pitch) + tuning, velocity, float(0.5), upRate, info, param);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  if (note.id == noteId) note.release(upRate);
}

void DSPCore::fillTransitionBuffer(size_t noteIndex)
{
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
