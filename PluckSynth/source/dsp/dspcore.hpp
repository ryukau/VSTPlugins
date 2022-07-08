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
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr float minNoteOffsetRate = float(9.5); // ~= 12 * log2(sqrt(3)).

enum class NoteState { active, release, rest };

// TODO: 12 * Math.log2(440 / a4Hz);
#define NOTE_PROCESS_INFO_SMOOTHER(METHOD)                                               \
  fdnEnable = pv[ID::fdnEnable]->getInt();                                               \
                                                                                         \
  noteOffsetRate = SmootherCommon<float>::timeInSamples >= 1                             \
    ? minNoteOffsetRate / SmootherCommon<float>::timeInSamples                           \
    : minNoteOffsetRate;                                                                 \
                                                                                         \
  auto eqTemp = pv[ID::equalTemperament]->getFloat() + 1;                                \
  auto semitone = int_fast32_t(pv[ID::semitone]->getInt()) - 120;                        \
  auto octave = int_fast32_t(pv[ID::octave]->getInt()) - 12;                             \
  auto milli = 0.001f * (int_fast32_t(pv[ID::milli]->getInt()) - 1000);                  \
  auto a4Hz = pv[ID::pitchA4Hz]->getFloat() + 100;                                       \
  auto centerPitch = std::log2(a4Hz / float(440));                                       \
  noteOffset.METHOD(float(12) * (octave + (semitone + milli) / eqTemp + centerPitch));   \
                                                                                         \
  fdnFeedback.METHOD(pv[ID::fdnFeedback]->getFloat());

struct NoteProcessInfo {
  pcg64 rng;
  pcg64 fdnRng;
  Wavetable<float, oscOvertoneSize> wavetable;

  bool fdnEnable = true;
  float noteOffsetRate = 1.0f;
  RateLimiter<float> noteOffset; // In 12ET semitones.
  ExpSmoother<float> fdnFeedback;

  void reset(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    rng.seed(9999991);
    fdnRng.seed(pv[ID::fdnSeed]->getInt());

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
    fdnFeedback.process();
    noteOffset.process(noteOffsetRate);
  }
};

#define NOTE_CLASS(INSTRSET)                                                             \
  class Note_##INSTRSET {                                                                \
  public:                                                                                \
    NoteState state = NoteState::rest;                                                   \
                                                                                         \
    int_fast32_t id = -1;                                                                \
    float velocity = 0;                                                                  \
    float noteFreq = 1;                                                                  \
    float oscNote = 0;                                                                   \
    float pan = 0.5f;                                                                    \
    float gain = 0;                                                                      \
    float releaseSwitch = 1.0f;                                                          \
                                                                                         \
    DoubleEmaADEnvelope<float> envelope;                                                 \
    TableOsc<float, oscOvertoneSize> osc;                                                \
    FeedbackDelayNetwork<float, fdnMatrixSize> fdn;                                      \
    NoteGate<float> gate;                                                                \
                                                                                         \
    void setup(float sampleRate);                                                        \
    void reset();                                                                        \
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

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters(float tempo) = 0;
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
    void setParameters(float tempo) override;                                            \
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
