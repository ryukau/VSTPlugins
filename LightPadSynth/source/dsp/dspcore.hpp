// (c) 2020 Takamitsu Endo
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

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "delay.hpp"
#include "envelope.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

enum class NoteState { active, release, rest };

struct NoteProcessInfo {
  std::minstd_rand rng{0};

  LinearSmoother<float> masterPitch;
  LinearSmoother<float> equalTemperament;
  LinearSmoother<float> pitchA4Hz;
  LinearSmoother<float> filterCutoff;
  LinearSmoother<float> filterResonance;
  LinearSmoother<float> filterAmount;
  LinearSmoother<float> filterKeyFollow;
  LinearSmoother<float> delayMix;
  LinearSmoother<float> delayDetune;
  LinearSmoother<float> delayFeedback;
  LinearSmoother<float> lfoFrequency;
  LinearSmoother<float> lfoAmount;
  LinearSmoother<float> lfoLowpass;

  LfoTableOsc<lfoTableSize> lfo;
  EMAFilter<float> lowpass;
  float lfoOut = 0;

  void process(float sampleRate, LfoWavetable<lfoTableSize> &lfoWavetable)
  {
    masterPitch.process();
    equalTemperament.process();
    pitchA4Hz.process();
    filterCutoff.process();
    filterResonance.process();
    filterAmount.process();
    filterKeyFollow.process();
    delayMix.process();
    delayDetune.process();
    delayFeedback.process();
    lfoFrequency.process();
    lfoAmount.process();
    lfoLowpass.process();

    lowpass.setP(lfoLowpass.getValue());
    lfoOut = 1.0f
      + lfoAmount.getValue()
        * lowpass.process(
          lfo.process(lfoWavetable.table, sampleRate, lfoFrequency.getValue()));
    if (lfoOut < 0.0f) lfoOut = 0.0f;
  }

  void reset()
  {
    masterPitch.reset(1.0f);
    equalTemperament.reset(12.0f);
    pitchA4Hz.reset(440.0f);
    filterCutoff.reset(0.0f);
    filterResonance.reset(0.0f);
    filterAmount.reset(0.0f);
    filterKeyFollow.reset(0.0f);
    delayMix.reset(0.5f);
    delayDetune.reset(1.0f);
    delayFeedback.reset(0.5f);
    lfoFrequency.reset(0.0f);
    lfoAmount.reset(0.0f);
    lfoLowpass.reset(1.0f);

    lfo.reset();
    lowpass.reset();
    lfoOut = 0;
  }
};

#define NOTE_CLASS(INSTRSET)                                                             \
  class Note_##INSTRSET {                                                                \
  public:                                                                                \
    NoteState state = NoteState::rest;                                                   \
                                                                                         \
    int32_t id = -1;                                                                     \
    float velocity = 0;                                                                  \
    float noteFreq = 1;                                                                  \
    float pan = 0.5f;                                                                    \
    float gain = 0;                                                                      \
                                                                                         \
    ExpADSREnvelope<float> gainEnvelope;                                                 \
    LinearADSREnvelope<float> filterEnvelope;                                            \
    AttackGate<float> delayGate;                                                         \
    TableOsc osc;                                                                        \
    LP3<float> filter;                                                                   \
    Delay<float> delay;                                                                  \
    float delaySeconds = 0;                                                              \
                                                                                         \
    void setup(float sampleRate);                                                        \
    void noteOn(                                                                         \
      int32_t noteId,                                                                    \
      float notePitch,                                                                   \
      float velocity,                                                                    \
      float pan,                                                                         \
      float phase,                                                                       \
      float sampleRate,                                                                  \
      Wavetable &wavetable,                                                              \
      NoteProcessInfo &info,                                                             \
      PlugParameter &param);                                                           \
    void release();                                                                      \
    void release(float seconds);                                                         \
    void rest();                                                                         \
    bool isAttacking();                                                                  \
    float getGain();                                                                     \
    std::array<float, 2>                                                                 \
    process(float sampleRate, Wavetable &wavetable, NoteProcessInfo &info);              \
  };

NOTE_CLASS(AVX512)
NOTE_CLASS(AVX2)
NOTE_CLASS(AVX)

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  static const size_t maxVoice = 128;
  PlugParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters(float tempo) = 0;
  virtual void process(const size_t length, float *out0, float *out1) = 0;
  virtual void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) = 0;
  virtual void noteOff(int32_t noteId) = 0;
  virtual void refreshTable() = 0;
  virtual void refreshLfo() = 0;

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  virtual void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
    float tuning,
    float velocity)
    = 0;
  virtual void processMidiNote(uint32_t frame) = 0;
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
    void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) override;   \
    void fillTransitionBuffer(size_t noteIndex);                                         \
    void noteOff(int32_t noteId) override;                                               \
    void refreshTable() override;                                                        \
    void refreshLfo() override;                                                          \
                                                                                         \
    void pushMidiNote(                                                                   \
      bool isNoteOn,                                                                     \
      uint32_t frame,                                                                    \
      int32_t noteId,                                                                    \
      int16_t pitch,                                                                     \
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
    void processMidiNote(uint32_t frame) override                                        \
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
    void setUnisonPan(size_t nUnison);                                                   \
                                                                                         \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    std::vector<PeakInfo<float>> peakInfos;                                              \
                                                                                         \
    bool prepareRefresh = true;                                                          \
    bool isTableRefeshed = false;                                                        \
    bool isLFORefreshed = false;                                                         \
    Wavetable wavetable;                                                                 \
    LfoWavetable<lfoTableSize> lfoWavetable;                                             \
                                                                                         \
    size_t nVoice = 32;                                                                  \
    int32_t panCounter = 0;                                                              \
    std::vector<size_t> noteIndices;                                                     \
    std::vector<size_t> voiceIndices;                                                    \
    std::vector<float> unisonPan;                                                        \
    std::array<Note_##INSTRSET, maxVoice> notes;                                         \
                                                                                         \
    NoteProcessInfo info;                                                                \
    LinearSmoother<float> interpMasterGain;                                              \
                                                                                         \
    std::vector<std::array<float, 2>> transitionBuffer{};                                \
    bool isTransitioning = false;                                                        \
    size_t trIndex = 0;                                                                  \
    size_t trStop = 0;                                                                   \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
