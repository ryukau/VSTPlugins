// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "fdn.hpp"
#include "lfo.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr float minOscNoteOffsetRate = float(9.5); // ~= 12 * log2(sqrt(3)).

inline float calcNotePitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2((note - 69.0f) / equalTemperament);
}

inline float noteToPitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2(note / equalTemperament);
}

enum class NoteState { active, release, rest };

#define NOTE_PROCESS_INFO_SMOOTHER(METHOD)                                                          \
  lfo.interpType = pv[ID::lfoInterpolation]->getInt();                                              \
  for (size_t idx = 0; idx < nLfoWavetable; ++idx) {                                                \
    lfo.source[idx + 1] = pv[ID::lfoWavetable0 + idx]->getFloat();                                  \
  }                                                                                                 \
                                                                                                    \
  envelope.interpType = pv[ID::modEnvelopeInterpolation]->getInt();                                 \
  for (size_t idx = 0; idx < nModEnvelopeWavetable; ++idx) {                                        \
    envelope.source[idx + 1] = pv[ID::modEnvelopeWavetable0 + idx]->getFloat();                     \
  }                                                                                                 \
  envelope.source[nModEnvelopeWavetable] = 0;                                                       \
                                                                                                    \
  fdnEnable = pv[ID::fdnEnable]->getInt();                                                          \
                                                                                                    \
  oscNoteOffsetRate = SmootherCommon<float>::timeInSamples >= 1                                     \
    ? minOscNoteOffsetRate / SmootherCommon<float>::timeInSamples                                   \
    : minOscNoteOffsetRate;                                                                         \
                                                                                                    \
  eqTemp = pv[ID::equalTemperament]->getFloat() + float(1);                                         \
  auto semitone = int_fast32_t(pv[ID::semitone]->getInt()) - 120;                                   \
  auto octave = int_fast32_t(pv[ID::octave]->getInt()) - 12;                                        \
  auto milli = float(0.001) * (int_fast32_t(pv[ID::milli]->getInt()) - 1000);                       \
  auto a4Hz = pv[ID::pitchA4Hz]->getFloat() + float(100);                                           \
  auto pitchBend = pv[ID::pitchBendRange]->getFloat() * pv[ID::pitchBend]->getFloat();              \
  auto oscOctave = int_fast32_t(pv[ID::oscOctave]->getInt()) - 12;                                  \
  auto oscFinePitch = pv[ID::oscFinePitch]->getFloat();                                             \
  auto centerPitch = std::log2(a4Hz / float(440));                                                  \
  oscNoteOffset.METHOD(                                                                             \
    float(12)                                                                                       \
    * (centerPitch + oscOctave + octave + (oscFinePitch + semitone + milli + pitchBend) / eqTemp)); \
  fdnFreqOffset.METHOD(a4Hz *calcNotePitch(                                                         \
    eqTemp *octave + semitone + milli + pitchBend + float(69), eqTemp));                            \
                                                                                                    \
  fdnOvertoneOffset.METHOD(pv[ID::fdnOvertoneOffset]->getFloat());                                  \
  fdnOvertoneMul.METHOD(pv[ID::fdnOvertoneMul]->getFloat());                                        \
  fdnOvertoneAdd.METHOD(pv[ID::fdnOvertoneAdd]->getFloat());                                        \
  fdnOvertoneModulo.METHOD(pv[ID::fdnOvertoneModulo]->getFloat());                                  \
  fdnLowpassQ.METHOD(pv[ID::lowpassQ]->getFloat());                                                 \
  fdnHighpassQ.METHOD(pv[ID::highpassQ]->getFloat());                                               \
  fdnFeedback.METHOD(pv[ID::fdnFeedback]->getFloat());                                              \
                                                                                                    \
  lfoToOscPitchAmount.METHOD(pv[ID::lfoToOscPitchAmount]->getFloat());                              \
  lfoToFdnPitchAmount.METHOD(pv[ID::lfoToFdnPitchAmount]->getFloat());                              \
  lfoToOscPitchAlignment = pv[ID::lfoToOscPitchAlignment]->getFloat();                              \
  lfoToFdnPitchAlignment = pv[ID::lfoToFdnPitchAlignment]->getFloat();                              \
                                                                                                    \
  modEnvelopeToFdnLowpassCutoff.METHOD(                                                             \
    pv[ID::modEnvelopeToFdnLowpassCutoff]->getFloat());                                             \
  modEnvelopeToFdnHighpassCutoff.METHOD(                                                            \
    pv[ID::modEnvelopeToFdnHighpassCutoff]->getFloat());                                            \
  modEnvelopeToOscPitch.METHOD(pv[ID::modEnvelopeToOscPitch]->getFloat());                          \
  modEnvelopeToFdnPitch.METHOD(pv[ID::modEnvelopeToFdnPitch]->getFloat());                          \
  modEnvelopeToFdnOvertoneAdd.METHOD(pv[ID::modEnvelopeToFdnOvertoneAdd]->getFloat());

struct NoteProcessInfo {
  pcg64 fdnRng;
  uint32_t previousSeed = 0;
  std::vector<std::vector<float>> fdnMatrixRandomBase;
  Wavetable<float, oscOvertoneSize> wavetable;

  TableLFO<float, nLfoWavetable, 1024, TableLFOType::lfo> lfo;
  TableLFO<float, nModEnvelopeWavetable + 1, 1024, TableLFOType::envelope> envelope;
  LinearTempoSynchronizer<float> synchronizer;
  float lfoPhase = 0;

  bool fdnEnable = true;
  float smootherKp = float(1);
  float oscNoteOffsetRate = float(1);
  float eqTemp = float(12);
  float lfoToOscPitchAlignment = float(1);
  float lfoToFdnPitchAlignment = float(1);

  RateLimiter<float> oscNoteOffset; // In 12ET semitones.
  ExpSmoother<float> fdnFreqOffset;
  ExpSmoother<float> fdnOvertoneOffset;
  ExpSmoother<float> fdnOvertoneMul;
  ExpSmoother<float> fdnOvertoneAdd;
  ExpSmoother<float> fdnOvertoneModulo;
  ExpSmoother<float> fdnLowpassQ;
  ExpSmoother<float> fdnHighpassQ;
  ExpSmoother<float> fdnFeedback;
  ExpSmoother<float> lfoToOscPitchAmount;
  ExpSmoother<float> lfoToFdnPitchAmount;
  ExpSmoother<float> modEnvelopeToFdnLowpassCutoff;
  ExpSmoother<float> modEnvelopeToFdnHighpassCutoff;
  ExpSmoother<float> modEnvelopeToOscPitch;
  ExpSmoother<float> modEnvelopeToFdnPitch;
  ExpSmoother<float> modEnvelopeToFdnOvertoneAdd;

  NoteProcessInfo()
  {
    fdnMatrixRandomBase.resize(fdnMatrixSize);
    for (size_t i = 0; i < fdnMatrixRandomBase.size(); ++i) {
      fdnMatrixRandomBase[i].resize(fdnMatrixSize - i);
    }
  }

  void reset(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    previousSeed = pv[ID::fdnSeed]->getInt();
    fdnRng.seed(previousSeed);

    std::normal_distribution<float> dist{}; // mean 0, stddev 1.
    for (auto &row : fdnMatrixRandomBase) {
      for (auto &value : row) value = dist(fdnRng);
    }

    lfo.reset();
    envelope.reset();
    lfoPhase = 0;

    NOTE_PROCESS_INFO_SMOOTHER(reset);
  }

  void setParameters(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    auto seed = pv[ID::fdnSeed]->getInt();
    if (previousSeed != seed) {
      previousSeed = seed;

      std::normal_distribution<float> dist{};
      for (auto &row : fdnMatrixRandomBase) {
        for (auto &value : row) value = dist(fdnRng);
      }
    }

    NOTE_PROCESS_INFO_SMOOTHER(push);
  }

  void process()
  {
    lfo.processRefresh();
    envelope.processRefresh();

    lfoPhase = synchronizer.process();

    oscNoteOffset.process(oscNoteOffsetRate);

    fdnFreqOffset.process();
    fdnOvertoneOffset.process();
    fdnOvertoneMul.process();
    fdnOvertoneAdd.process();
    fdnOvertoneModulo.process();
    fdnLowpassQ.process();
    fdnHighpassQ.process();
    fdnFeedback.process();
    lfoToOscPitchAmount.process();
    lfoToFdnPitchAmount.process();
    modEnvelopeToFdnLowpassCutoff.process();
    modEnvelopeToFdnHighpassCutoff.process();
    modEnvelopeToOscPitch.process();
    modEnvelopeToFdnPitch.process();
    modEnvelopeToFdnOvertoneAdd.process();
  }
};

class Note {
public:
  NoteState state = NoteState::rest;

  int_fast32_t id = -1;
  float velocity = 0;
  float fdnPitch = 0;
  float oscNote = 0;
  float gain = 0;
  float releaseSwitch = float(1);

  ExpSmootherLocal<float> pan;
  ExpSmootherLocal<float> fdnLowpassCutoff;  /* In normalized frequency. */
  ExpSmootherLocal<float> fdnHighpassCutoff; /* In normalized frequency. */

  float impulse = 0;
  DoubleEmaADEnvelope<float> envelope;
  LFOPhase<float> lfoPhase;
  EnvelopePhase<float> modEnvelopePhase;
  TableOsc<float, oscOvertoneSize> osc;
  std::array<float, fdnMatrixSize> overtoneRandomness{};
  FeedbackDelayNetwork<float, fdnMatrixSize> fdn;
  NoteGate<float> gate;
  DoubleEMAFilter<float> gateSmoother;

  void setup(float sampleRate);
  void reset(float sampleRate, NoteProcessInfo &info, GlobalParameter &param);
  void setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param);
  void noteOn(
    int_fast32_t noteId,
    float notePitch,
    float velocity,
    float pan,
    float sampleRate,
    NoteProcessInfo &info,
    GlobalParameter &param);
  void release(float sampleRate);
  void rest();
  bool isAttacking();
  float getGain();
  std::array<float, 2> process(float sampleRate, NoteProcessInfo &info);
};

class DSPCore {
public:
  struct MidiNote {
    bool isNoteOn;
    size_t frame;
    int_fast32_t id;
    int_fast16_t pitch;
    float tuning;
    float velocity;
  };

  GlobalParameter param;

  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;

  std::vector<MidiNote> midiNotes;

  DSPCore();

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity);
  void noteOff(int_fast32_t noteId);
  void fillTransitionBuffer(size_t noteIndex);

  void pushMidiNote(
    bool isNoteOn,
    size_t frame,
    int_fast32_t noteId,
    int_fast16_t pitch,
    float tuning,
    float velocity)
  {
    MidiNote note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch;
    note.tuning = tuning;
    note.velocity = velocity;
    midiNotes.push_back(note);
  }

  void processMidiNote(size_t frame)
  {
    while (true) {
      auto it = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {
        return nt.frame == frame;
      });
      if (it == std::end(midiNotes)) return;
      if (it->isNoteOn)
        noteOn(it->id, it->pitch, it->tuning, it->velocity);
      else
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }

private:
  float getTempoSyncInterval();

  static constexpr size_t upFold = 2;
  bool prepareRefresh = true;
  bool isWavetableRefeshed = false;
  float sampleRate = 44100.0f;
  float upRate = 88200.0f;
  DecibelScale<float> velocityMap{-60, 0, true};

  size_t nVoice = maximumVoice;
  size_t panCounter = 0;
  std::vector<size_t> noteIndices;
  std::vector<size_t> voiceIndices;
  std::vector<float> unisonPan;
  std::array<Note, maximumVoice> notes;

  NoteProcessInfo info;
  ExpSmoother<float> interpMasterGain;

  std::array<std::array<float, 2>, 2> halfIn{{}};
  std::array<HalfBandIIR<float, HalfBandCoefficient<float>>, 2> halfbandIir;

  std::vector<std::array<float, 2>> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};
