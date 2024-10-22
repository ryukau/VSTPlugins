// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <numeric>
#include <random>

constexpr float maxDelayTime = float(4);

inline float calcNotePitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2((note - 69.0f) / equalTemperament);
}

inline float noteToPitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2(note / equalTemperament);
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

Note::Note()
{
  fdnMatrixRandomBase.resize(fdnMatrixSize);
  for (size_t i = 0; i < fdnMatrixRandomBase.size(); ++i) {
    fdnMatrixRandomBase[i].resize(fdnMatrixSize - i);
  }
}

void Note::setup(float sampleRate)
{
  oscEnvelopeSmoother.setCutoff(sampleRate, float(4000));
  fdn.setup(sampleRate, maxDelayTime);
  tremolo.setup(sampleRate, float(Scales::tremoloDelayTime.getMax()));
}

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = float(sampleRate);
  upRate = upFold * this->sampleRate;

  SmootherCommon<float>::setSampleRate(upRate);

  // 10 msec + 1 sample transition time.
  transitionBuffer.resize(1 + size_t(upRate * double(0.005)), float(0));

  note.setup(upRate);

  reset();

  isInitialized = true;
}

#define ASSIGN_NOTE_PARAMETER(METHOD)                                                    \
  envelopeTable.interpType = pv[ID::modEnvelopeInterpolation]->getInt();                 \
  for (size_t idx = 0; idx < nModEnvelopeWavetable; ++idx) {                             \
    envelopeTable.source[idx + 1] = pv[ID::modEnvelopeWavetable0 + idx]->getFloat();     \
  }                                                                                      \
  envelopeTable.source[nModEnvelopeWavetable] = 0;                                       \
                                                                                         \
  fdnEnable = pv[ID::fdnEnable]->getInt();

#define ASSIGN_FILTER_PARAMETER(METHOD)                                                  \
  eqTemp = pv[ID::equalTemperament]->getFloat() + float(1);                              \
  auto semitone = int_fast32_t(pv[ID::semitone]->getInt()) - 120;                        \
  auto octave = int_fast32_t(pv[ID::octave]->getInt()) - 12;                             \
  auto milli = float(0.001) * (int_fast32_t(pv[ID::milli]->getInt()) - 1000);            \
  auto a4Hz = pv[ID::pitchA4Hz]->getFloat() + float(100);                                \
  auto pitchBend = pv[ID::pitchBendRange]->getFloat() * pv[ID::pitchBend]->getFloat();   \
  fdnFreqOffset.METHOD(a4Hz *calcNotePitch(                                              \
    eqTemp *octave + semitone + milli + pitchBend + float(69), eqTemp));                 \
                                                                                         \
  fdnOvertoneOffset.METHOD(pv[ID::fdnOvertoneOffset]->getFloat());                       \
  fdnOvertoneMul.METHOD(pv[ID::fdnOvertoneMul]->getFloat());                             \
  fdnOvertoneAdd.METHOD(pv[ID::fdnOvertoneAdd]->getFloat());                             \
  fdnOvertoneModulo.METHOD(pv[ID::fdnOvertoneModulo]->getFloat());                       \
  fdnFeedback.METHOD(pv[ID::fdnFeedback]->getFloat());                                   \
                                                                                         \
  modEnvelopeToFdnPitch.METHOD(pv[ID::modEnvelopeToFdnPitch]->getFloat());               \
  modEnvelopeToFdnOvertoneAdd.METHOD(pv[ID::modEnvelopeToFdnOvertoneAdd]->getFloat());   \
  modEnvelopeToOscJitter.METHOD(pv[ID::modEnvelopeToOscJitter]->getFloat());             \
  modEnvelopeToOscNoisePulseRatio.METHOD(                                                \
    pv[ID::modEnvelopeToOscNoisePulseRatio]->getFloat());                                \
                                                                                         \
  oscBounce.METHOD(pv[ID::oscBounce]->getFloat());                                       \
  oscBounceCurve.METHOD(pv[ID::oscBounceCurve]->getFloat());                             \
  oscPulseAmpRandomness.METHOD(pv[ID::oscPulseAmpRandomness]->getFloat());               \
  auto oscDensityPitch = float(1)                                                        \
    + pv[ID::oscDensityKeyFollow]->getFloat() * (fdnPitch.getValue() - float(1));        \
  oscDensity.METHOD(oscDensityPitch *pv[ID::oscDensityHz]->getFloat() / sampleRate);     \
  oscJitter.METHOD(pv[ID::oscJitter]->getFloat());                                       \
  oscNoisePulseRatio.METHOD(pv[ID::oscNoisePulseRatio]->getFloat());                     \
                                                                                         \
  tremoloMix.METHOD(pv[ID::tremoloMix]->getFloat());                                     \
  tremoloDepth.METHOD(pv[ID::tremoloDepth]->getFloat());                                 \
  tremoloDelayTime.METHOD(sampleRate *pv[ID::tremoloDelayTime]->getFloat());             \
  tremoloModToDelayTimeOffset.METHOD(                                                    \
    pv[ID::tremoloModulationToDelayTimeOffset]->getFloat());                             \
  tremoloModDeltaPhase.METHOD(pv[ID::tremoloModulationRateHz]->getFloat() / sampleRate); \
                                                                                         \
  auto fdnFreq = fdnFreqOffset.getValue() * fdnPitch.getValue();                         \
                                                                                         \
  auto oscLpCutoff = semitoneToNormalizedFreq(                                           \
    sampleRate, fdnFreq, pv[ID::oscLowpassCutoffSemi]->getFloat(),                       \
    pv[ID::oscLowpassKeyFollow]->getFloat());                                            \
  oscLowpass.METHOD##Cutoff(oscLpCutoff, pv[ID::oscLowpassQ]->getFloat());               \
                                                                                         \
  auto modenv = envelopeTable.process(modEnvelopePhase.process());                       \
  auto modenvToFdnLp                                                                     \
    = noteToPitch(modenv * pv[ID::modEnvelopeToFdnLowpassCutoff]->getFloat(), eqTemp);   \
  auto modenvToFdnHp                                                                     \
    = noteToPitch(modenv * pv[ID::modEnvelopeToFdnHighpassCutoff]->getFloat(), eqTemp);  \
                                                                                         \
  auto fdnLowpassKeyFollow = float(pv[ID::fdnLowpassKeyFollow]->getInt());               \
  auto fdnHighpassKeyFollow = float(pv[ID::fdnHighpassKeyFollow]->getInt());             \
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

void Note::reset(float sampleRate, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  state = NoteState::rest;
  id = -1;

  previousSeed = pv[ID::fdnSeed]->getInt();
  rng.seed(previousSeed);

  std::normal_distribution<float> dist{}; // mean 0, stddev 1.
  for (auto &row : fdnMatrixRandomBase) {
    for (auto &value : row) value = dist(rng);
  }

  ASSIGN_NOTE_PARAMETER(reset);
  ASSIGN_FILTER_PARAMETER(reset);

  envelopeTable.reset();
  modEnvelopePhase.counter = 0;

  impulse = 0;
  oscEnvelope.reset();
  oscEnvelopeSmoother.reset();
  pulsar.reset();
  oscLowpass.reset();

  fdnPitch.reset(float(1));
  pitchSlideKp = float(
    EMAFilter<double>::secondToP(sampleRate, pv[ID::slideTimeSecond]->getDouble()));
  fdn.reset();

  tremolo.reset(pv[ID::tremoloModulationToDelayTimeOffset]->getFloat());
  tremoloPhase = 0;

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

  ASSIGN_PARAMETER(reset);

  note.reset(upRate, param);

  halfbandIir.reset();

  std::fill(transitionBuffer.begin(), transitionBuffer.end(), float(0));
  isTransitioning = false;
  trIndex = 0;
  trStop = 0;
}

void DSPCore::startup() {}

void Note::setParameters(float sampleRate, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto seed = pv[ID::fdnSeed]->getInt();
  if (previousSeed != seed) {
    previousSeed = seed;
    rng.seed(previousSeed);

    std::normal_distribution<float> dist{};
    for (auto &row : fdnMatrixRandomBase) {
      for (auto &value : row) value = dist(rng);
    }
  }

  ASSIGN_NOTE_PARAMETER(push);
  ASSIGN_FILTER_PARAMETER(push);

  pulsar.setDecay(sampleRate * pv[ID::oscNoiseDecay]->getFloat());

  if (state == NoteState::release && pv[ID::slideType]->getInt() == 2) {
    // `Reset to 0` case.
    pitchSlideKp = float(
      EMAFilter<double>::secondToP(sampleRate, pv[ID::gateReleaseSecond]->getDouble()));
  } else {
    pitchSlideKp = float(
      EMAFilter<double>::secondToP(sampleRate, pv[ID::slideTimeSecond]->getDouble()));
  }

  fdn.delay.rate = pv[ID::fdnInterpRate]->getFloat();
  fdn.delay.kp = float(EMAFilter<double>::secondToP(
    sampleRate, pv[ID::fdnInterpLowpassSecond]->getDouble()));

  gate.prepare(sampleRate, pv[ID::gateReleaseSecond]->getFloat());
  auto gateAttackSecond = pv[ID::gateAttackSecond]->getFloat();
  gateSmoother.setCutoff(
    sampleRate,
    gateAttackSecond < std::numeric_limits<float>::epsilon()
      ? sampleRate
      : float(1) / gateAttackSecond);
}

void DSPCore::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  ASSIGN_PARAMETER(push);

  if (note.state != NoteState::rest) note.setParameters(upRate, param);
}

inline float alignModValue(float amount, float alignment, float value)
{
  if (alignment == 0) return amount * value;
  return alignment * std::floor(value * amount / alignment + float(0.5));
}

float Note::process(float sampleRate)
{
  constexpr auto eps = std::numeric_limits<float>::epsilon();

  modEnvelopeToFdnPitch.process();
  modEnvelopeToFdnOvertoneAdd.process();
  modEnvelopeToOscJitter.process();
  modEnvelopeToOscNoisePulseRatio.process();
  oscBounce.process();
  oscBounceCurve.process();
  oscJitter.process();
  oscDensity.process();
  oscPulseAmpRandomness.process();
  oscNoisePulseRatio.process();
  fdnFreqOffset.process();
  fdnOvertoneOffset.process();
  fdnOvertoneMul.process();
  fdnOvertoneAdd.process();
  fdnOvertoneModulo.process();
  fdnFeedback.process();
  tremoloMix.process();
  tremoloDepth.process();
  tremoloDelayTime.process();
  tremoloModToDelayTimeOffset.process();
  tremoloModDeltaPhase.process();

  fdnPitch.process(pitchSlideKp);

  envelopeTable.processRefresh();

  if (state == NoteState::rest) return 0;

  auto modenv = envelopeTable.process(modEnvelopePhase.process());
  auto modenvToFdnPitch = modenv * modEnvelopeToFdnPitch.getValue();
  auto modenvToFdnOvertoneAdd = modenv * modEnvelopeToFdnOvertoneAdd.getValue();
  auto modenvToOscJitter = modenv * modEnvelopeToOscJitter.getValue();
  auto modenvToOscNoisePulseRatio = modenv * modEnvelopeToOscNoisePulseRatio.getValue();

  float sig = impulse;
  impulse = 0;

  auto oscGain = oscEnvelopeSmoother.process(oscEnvelope.process() * velocity);
  if (oscGain >= eps) {
    auto densityEnvelope = float(1)
      + oscBounce.getValue() * (std::pow(oscGain, oscBounceCurve.getValue()) - float(1));

    auto oscOut = pulsar.process(
      oscJitter.getValue() + modenvToOscJitter, oscDensity.getValue(), densityEnvelope,
      oscPulseAmpRandomness.getValue(),
      oscNoisePulseRatio.getValue() + modenvToOscNoisePulseRatio, rng);

    sig += oscGain * oscOut;
  }
  sig = oscLowpass.process(sig);
  auto oscOut = sig;

  if (fdnEnable) {
    auto fdnFreq = fdnFreqOffset.getValue() * fdnPitch.getValue()
      * noteToPitch(modenvToFdnPitch, eqTemp);
    float overtone = float(1);
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      fdn.delay.setDelayTimeAt(
        idx, sampleRate,
        fdnOvertoneOffset.getValue() + (float(1) + overtoneRandomness[idx]) * overtone,
        fdnFreq);
      auto ot = overtone * fdnOvertoneMul.getValue() + fdnOvertoneAdd.getValue()
        + modenvToFdnOvertoneAdd;
      if (fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
        // Almost same operation as `std::fmod()`.
        ot /= float(1) + fdnOvertoneModulo.getValue();
        ot -= std::floor(ot);
        ot *= float(1) + fdnOvertoneModulo.getValue();
      }
      overtone = ot;
    }

    sig = float(0.01 * pi) * fdn.process(sig, fdnFeedback.getValue());
  }

  tremoloPhase += tremoloModDeltaPhase.getValue();
  tremoloPhase -= std::floor(tremoloPhase);
  sig = tremolo.process(
    sig, std::sin(float(twopi) * tremoloPhase), tremoloModToDelayTimeOffset.getValue(),
    tremoloDelayTime.getValue(), tremoloDepth.getValue(), tremoloMix.getValue());

  auto outputGain = gateSmoother.process(gate.process());
  sig *= outputGain;

  if (state == NoteState::release && outputGain <= eps) state = NoteState::rest;

  return sig;
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(pv[ID::commonSmoothingTimeSecond]->getFloat());
  SmootherCommon<float>::setBufferSize(float(length));

  bool overSampling = pv[ID::overSampling]->getInt();

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (overSampling) {
      halfIn.fill({});

      for (size_t j = 0; j < upFold; ++j) {
        if (note.state != NoteState::rest) {
          halfIn[j] += note.process(upRate);
        }

        if (isTransitioning) {
          halfIn[j] += transitionBuffer[trIndex];
          transitionBuffer[trIndex] = 0;
          trIndex = (trIndex + 1) % transitionBuffer.size();
          if (trIndex == trStop) isTransitioning = false;
        }

        const auto masterGain = interpMasterGain.process();
        halfIn[j] *= masterGain;
      }

      auto folded = halfbandIir.process(halfIn);
      out0[i] = folded;
      out1[i] = folded;
    } else {
      float sig = 0;

      if (note.state != NoteState::rest) {
        sig += note.process(upRate);
      }

      if (isTransitioning) {
        sig += transitionBuffer[trIndex];
        transitionBuffer[trIndex] = 0;
        trIndex = (trIndex + 1) % transitionBuffer.size();
        if (trIndex == trStop) isTransitioning = false;
      }

      const auto masterGain = interpMasterGain.process();
      sig *= masterGain;

      out0[i] = sig;
      out1[i] = sig;
    }
  }
}

void Note::noteOn(
  int_fast32_t noteId,
  float notePitch,
  float velocity,
  float pan,
  float sampleRate,
  GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  id = noteId;

  this->velocity = velocity;

  modEnvelopePhase.noteOn(sampleRate, pv[ID::modEnvelopeTime]->getFloat());

  // Oscillator.
  impulse = pv[ID::impulseGain]->getFloat();
  pulsar.reset();
  oscEnvelope.noteOn(
    pv[ID::oscGain]->getFloat(), sampleRate * pv[ID::oscAttack]->getFloat(),
    sampleRate * pv[ID::oscDecay]->getFloat());

  // FDN.

  std::uniform_real_distribution<float> overtoneDist(-1.0, 1.0);
  for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
    overtoneRandomness[idx]
      = overtoneDist(rng) * pv[ID::fdnOvertoneRandomness]->getFloat();
  }

  std::uniform_int_distribution<unsigned> seedDist{
    0, std::numeric_limits<unsigned>::max()};
  fdn.randomOrthogonal(
    seedDist(rng), pv[ID::fdnMatrixIdentityAmount]->getFloat(),
    pv[ID::fdnRandomizeRatio]->getFloat(), fdnMatrixRandomBase);

  fdn.delay.rate = pv[ID::fdnInterpRate]->getFloat();
  auto fdnInterpLowpassSecond = pv[ID::fdnInterpLowpassSecond]->getFloat();
  fdn.delay.kp = fdnInterpLowpassSecond == 0
    ? float(1)
    : float(EMAFilter<double>::cutoffToP(sampleRate, 1.0 / fdnInterpLowpassSecond));

  gate.reset();

  // Resetting.
  pitchSlideKp = float(
    EMAFilter<double>::secondToP(sampleRate, pv[ID::slideTimeSecond]->getDouble()));
  auto fdnPitchTarget = calcNotePitch(notePitch, eqTemp);
  if (pv[ID::resetAtNoteOn]->getInt() || state == NoteState::rest) {
    ASSIGN_FILTER_PARAMETER(reset);

    oscEnvelopeSmoother.reset();
    oscLowpass.reset();

    auto slideType = pv[ID::slideType]->getInt();
    if (slideType == 0) { // Sustain
      fdnPitch.reset(fdnPitchTarget);
    } else if (slideType == 2) { // Reset to 0
      fdnPitch.reset(std::numeric_limits<float>::epsilon());
    }

    fdn.reset();
    float overtone = 1.0f;
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      fdn.delay.resetDelayTimeAt(
        idx, sampleRate,
        fdnOvertoneOffset.getValue() + (float(1) + overtoneRandomness[idx]) * overtone,
        fdnFreq);
      auto ot = overtone * fdnOvertoneMul.getValue() + fdnOvertoneAdd.getValue();
      if (fdnOvertoneModulo.getValue() >= std::numeric_limits<float>::epsilon()) {
        ot /= float(1) + fdnOvertoneModulo.getValue();
        ot -= std::floor(ot);
        ot *= float(1) + fdnOvertoneModulo.getValue();
      }
      overtone = ot;
    }

    tremolo.reset(pv[ID::tremoloModulationToDelayTimeOffset]->getFloat());

    gateSmoother.reset();
  } else {
    ASSIGN_FILTER_PARAMETER(push);
  }
  fdnPitch.push(fdnPitchTarget);

  state = NoteState::active;
}

void Note::slide(int_fast32_t noteId, float notePitch, float velocity, float sampleRate)
{
  id = noteId;
  this->velocity = velocity;
  fdnPitch.push(calcNotePitch(notePitch, eqTemp));
}

void Note::release(float sampleRate, GlobalParameter &param)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (state == NoteState::rest) return;
  state = NoteState::release;

  if (pv[ID::slideType]->getInt() == 2) {
    fdnPitch.push(std::numeric_limits<float>::epsilon());
    pitchSlideKp = float(
      EMAFilter<double>::secondToP(sampleRate, pv[ID::gateReleaseSecond]->getDouble()));
  }

  gate.release();
}

void DSPCore::noteOn(
  int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (pv[ID::resetAtNoteOn]->getInt()) fillTransitionBuffer();

  auto notePitch = float(pitch) + tuning;
  note.noteOn(noteId, notePitch, velocity, float(0.5), upRate, param);

  NoteInfo info;
  info.id = noteId;
  info.notePitch = notePitch;
  info.velocity = velocity;
  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (note.id == noteId) {
    if (noteStack.empty()) {
      note.release(upRate, param);
    } else {
      auto info = noteStack.back();
      note.slide(info.id, info.notePitch, info.velocity, upRate);
    }
  }
}

void DSPCore::fillTransitionBuffer()
{
  if (note.state == NoteState::rest) return;

  isTransitioning = true;

  // Beware the negative overflow. trStop is size_t.
  trStop = trIndex - 1;
  if (trStop >= transitionBuffer.size()) trStop += transitionBuffer.size();

  for (size_t bufIdx = 0; bufIdx < transitionBuffer.size(); ++bufIdx) {
    auto oscOut = note.process(upRate);
    auto idx = (trIndex + bufIdx) % transitionBuffer.size();
    auto interp = float(1) - float(bufIdx) / transitionBuffer.size();

    transitionBuffer[idx] += oscOut * interp;
  }
}
