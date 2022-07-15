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

#pragma once

#ifdef USE_VECTORCLASS
  #include "../../../lib/vcl/vectorclass.h"
#endif

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
#include <memory>
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
  for (size_t idx = 0; idx < nModWavetable; ++idx) {                                                \
    lfo.source[idx + 1] = pv[ID::lfoWavetable0 + idx]->getFloat();                                  \
  }                                                                                                 \
                                                                                                    \
  envelope.interpType = pv[ID::modEnvelopeInterpolation]->getInt();                                 \
  for (size_t idx = 0; idx < nModWavetable; ++idx) {                                                \
    envelope.source[idx + 1] = pv[ID::modEnvelopeWavetable0 + idx]->getFloat();                     \
  }                                                                                                 \
  envelope.source[nModWavetable] = 0;                                                               \
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
  lfoToOscPitchAmount.METHOD(pv[ID::lfoToOscPitchAmount]->getFloat());                              \
  lfoToFdnPitchAmount.METHOD(pv[ID::lfoToFdnPitchAmount]->getFloat());                              \
  lfoToOscPitchAlignment = pv[ID::lfoToOscPitchAlignment]->getFloat();                              \
  lfoToFdnPitchAlignment = pv[ID::lfoToFdnPitchAlignment]->getFloat();                              \
  modEnvelopeToOscPitch.METHOD(pv[ID::modEnvelopeToOscPitch]->getFloat());                          \
  modEnvelopeToFdnPitch.METHOD(pv[ID::modEnvelopeToFdnPitch]->getFloat());                          \
  modEnvelopeToLfoToPOscPitch.METHOD(pv[ID::modEnvelopeToLfoToPOscPitch]->getFloat());              \
  modEnvelopeToLfoToPFdnPitch.METHOD(pv[ID::modEnvelopeToLfoToPFdnPitch]->getFloat());              \
  modEnvelopeToFdnLowpassCutoff.METHOD(                                                             \
    noteToPitch(pv[ID::modEnvelopeToFdnLowpassCutoff]->getFloat(), eqTemp));                        \
  modEnvelopeToFdnHighpassCutoff.METHOD(                                                            \
    noteToPitch(pv[ID::modEnvelopeToFdnHighpassCutoff]->getFloat(), eqTemp));

struct NoteProcessInfo {
  pcg64 rng;
  pcg64 fdnRng;
  std::vector<std::vector<float>> fdnMatrixRandomBase;
  Wavetable<float, oscOvertoneSize> wavetable;

  TableLFO<float, nModWavetable, 1024, TableLFOType::lfo> lfo;
  TableLFO<float, nModWavetable + 1, 1024, TableLFOType::envelope> envelope;
  LinearTempoSynchronizer<float> synchronizer;
  float lfoPhase = 0;

  bool fdnEnable = true;
  float oscNoteOffsetRate = 1.0f;
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
  ExpSmoother<float> modEnvelopeToOscPitch;
  ExpSmoother<float> modEnvelopeToFdnPitch;
  ExpSmoother<float> modEnvelopeToLfoToPOscPitch;
  ExpSmoother<float> modEnvelopeToLfoToPFdnPitch;
  ExpSmoother<float> modEnvelopeToFdnLowpassCutoff;
  ExpSmoother<float> modEnvelopeToFdnHighpassCutoff;

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

    rng.seed(9999991);
    fdnRng.seed(pv[ID::fdnSeed]->getInt());

    std::normal_distribution<float> dist{}; // mean 0, stddev 1.
    for (auto &row : fdnMatrixRandomBase) {
      for (auto &value : row) value = dist(fdnRng);
    }

    lfoPhase = 0;

    NOTE_PROCESS_INFO_SMOOTHER(reset);
  }

  void setParameters(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

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
    modEnvelopeToOscPitch.process();
    modEnvelopeToFdnPitch.process();
    modEnvelopeToLfoToPOscPitch.process();
    modEnvelopeToLfoToPFdnPitch.process();
    modEnvelopeToFdnLowpassCutoff.process();
    modEnvelopeToFdnHighpassCutoff.process();
  }
};

#define NOTE_CLASS(INSTRSET)                                                             \
  class Note_##INSTRSET {                                                                \
  public:                                                                                \
    NoteState state = NoteState::rest;                                                   \
                                                                                         \
    int_fast32_t id = -1;                                                                \
    float velocity = 0;                                                                  \
    float fdnPitch = 0;                                                                  \
    float oscNote = 0;                                                                   \
    float pan = 0.5f;                                                                    \
    float gain = 0;                                                                      \
    float releaseSwitch = 1.0f;                                                          \
                                                                                         \
    ExpSmoother<float> fdnLowpassCutoff;  /* In normalized frequency. */                 \
    ExpSmoother<float> fdnHighpassCutoff; /* In normalized frequency. */                 \
                                                                                         \
    DoubleEmaADEnvelope<float> envelope;                                                 \
    LFOPhase<float> lfoPhase;                                                            \
    EnvelopePhase<float> modEnvelopePhase;                                               \
    TableOsc<float, oscOvertoneSize> osc;                                                \
    std::array<float, fdnMatrixSize> overtoneRandomness{};                               \
    FeedbackDelayNetwork<float, fdnMatrixSize> fdn;                                      \
    NoteGate<float> gate;                                                                \
                                                                                         \
    void setup(float sampleRate);                                                        \
    void reset();                                                                        \
    void setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param); \
    void noteOn(                                                                         \
      int_fast32_t noteId,                                                               \
      float notePitch,                                                                   \
      float velocity,                                                                    \
      float pan,                                                                         \
      float sampleRate,                                                                  \
      NoteProcessInfo &info,                                                             \
      GlobalParameter &param);                                                           \
    void release(float sampleRate);                                                      \
    void rest();                                                                         \
    bool isAttacking();                                                                  \
    float getGain();                                                                     \
    std::array<float, 2> process(float sampleRate, NoteProcessInfo &info);               \
  };

#ifdef USE_VECTORCLASS
NOTE_CLASS(AVX512)
NOTE_CLASS(AVX2)
NOTE_CLASS(AVX)
#else
NOTE_CLASS(Plain)
#endif

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  constexpr static size_t maxVoice = maximumVoice;
  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters() = 0;
  virtual void process(const size_t length, float *out0, float *out1) = 0;
  virtual void
  noteOn(int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity)
    = 0;
  virtual void noteOff(int_fast32_t noteId) = 0;

  struct MidiNote {
    bool isNoteOn;
    size_t frame;
    int_fast32_t id;
    int_fast16_t pitch;
    float tuning;
    float velocity;
  };

  virtual void pushMidiNote(
    bool isNoteOn,
    size_t frame,
    int_fast32_t noteId,
    int_fast16_t pitch,
    float tuning,
    float velocity)
    = 0;
  virtual void processMidiNote(size_t frame) = 0;
};

#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    DSPCore_##INSTRSET();                                                                \
                                                                                         \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    void setParameters() override;                                                       \
    void process(const size_t length, float *out0, float *out1) override;                \
    void noteOn(                                                                         \
      int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity) override;   \
    void noteOff(int_fast32_t noteId) override;                                          \
    void fillTransitionBuffer(size_t noteIndex);                                         \
                                                                                         \
    std::vector<MidiNote> midiNotes;                                                     \
                                                                                         \
    void pushMidiNote(                                                                   \
      bool isNoteOn,                                                                     \
      size_t frame,                                                                      \
      int_fast32_t noteId,                                                               \
      int_fast16_t pitch,                                                                \
      float tuning,                                                                      \
      float velocity) override                                                           \
    {                                                                                    \
      MidiNote note;                                                                     \
      note.isNoteOn = isNoteOn;                                                          \
      note.frame = frame;                                                                \
      note.id = noteId;                                                                  \
      note.pitch = pitch;                                                                \
      note.tuning = tuning;                                                              \
      note.velocity = velocity;                                                          \
      midiNotes.push_back(note);                                                         \
    }                                                                                    \
                                                                                         \
    void processMidiNote(size_t frame) override                                          \
    {                                                                                    \
      while (true) {                                                                     \
        auto it                                                                          \
          = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {   \
              return nt.frame == frame;                                                  \
            });                                                                          \
        if (it == std::end(midiNotes)) return;                                           \
        if (it->isNoteOn)                                                                \
          noteOn(it->id, it->pitch, it->tuning, it->velocity);                           \
        else                                                                             \
          noteOff(it->id);                                                               \
        midiNotes.erase(it);                                                             \
      }                                                                                  \
    }                                                                                    \
                                                                                         \
  private:                                                                               \
    float getTempoSyncInterval();                                                        \
                                                                                         \
    static constexpr size_t upFold = 2;                                                  \
    bool prepareRefresh = true;                                                          \
    bool isWavetableRefeshed = false;                                                    \
    float sampleRate = 44100.0f;                                                         \
    float upRate = 88200.0f;                                                             \
    float velocity = 0.0f;                                                               \
    DecibelScale<float> velocityMap{-60, 0, true};                                       \
                                                                                         \
    size_t nVoice = maxVoice;                                                            \
    size_t noteOnIndex = 0;                                                              \
    size_t panCounter = 0;                                                               \
    std::vector<size_t> noteIndices;                                                     \
    std::vector<size_t> voiceIndices;                                                    \
    std::vector<float> unisonPan;                                                        \
    std::array<Note_##INSTRSET, maxVoice> notes;                                         \
                                                                                         \
    NoteProcessInfo info;                                                                \
    ExpSmoother<float> interpMasterGain;                                                 \
                                                                                         \
    std::array<std::array<float, 2>, 2> halfIn{{}};                                      \
    std::array<HalfBandIIR<float, HalfBandCoefficient<float>>, 2> halfbandIir;           \
                                                                                         \
    std::vector<std::array<float, 2>> transitionBuffer{};                                \
    bool isTransitioning = false;                                                        \
    size_t trIndex = 0;                                                                  \
    size_t trStop = 0;                                                                   \
  };

#ifdef USE_VECTORCLASS
DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
#else
DSPCORE_CLASS(Plain)
#endif
