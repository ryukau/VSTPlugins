// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "iir.hpp"
#include "noise.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <vector>

using namespace SomeDSP;
using namespace Steinberg::Synth;

template<typename Sample> struct NoteProcessInfo {
  Sample osc1Gain;
  Sample osc1Pitch;
  Sample osc1Sync;
  int32_t osc1SyncType;
  uint32_t osc1PTROrder;

  Sample osc2Gain;
  Sample osc2Pitch;
  Sample osc2Sync;
  int32_t osc2SyncType;
  uint32_t osc2PTROrder;

  Sample fmOsc1ToSync1;
  Sample fmOsc1ToFreq2;
  Sample fmOsc2ToSync1;

  Sample gainEnvelopeCurve;

  Sample filterCutoff;
  Sample filterResonance;
  Sample filterFeedback;
  Sample filterSaturation;
  Sample filterCutoffAmount;
  Sample filterResonanceAmount;
  Sample filterKeyToCutoff;
  Sample filterKeyToFeedback;

  Sample modEnvelopeToFreq1;
  Sample modEnvelopeToSync1;
  Sample modEnvelopeToFreq2;
  Sample modEnvelopeToSync2;
  Sample modLFO;
  Sample modLFOToFreq1;
  Sample modLFOToSync1;
  Sample modLFOToFreq2;
  Sample modLFOToSync2;
};

enum class NoteState { active, release, rest };

template<typename Sample> class Note {
public:
  NoteState state = NoteState::rest;

  int32_t id = -1;
  Sample normalizedKey = 0;
  Sample velocity = 0;
  Sample gain = 0;
  Sample frequency = 0;
  bool bypassFilter = false;

  PTRSyncSaw<Sample> saw1;
  PTRSyncSaw<Sample> saw2;
  std::array<float, 2> oscBuffer = {0, 0};

  SerialFilter4<Sample> filter;

  ExpADSREnvelope<float> gainEnvelope;
  LinearEnvelope<float> filterEnvelope;
  PolyExpEnvelope<double> modEnvelope;

  Note(Sample sampleRate)
    : saw1(sampleRate, 0, 0)
    , saw2(sampleRate, 0, 0)
    , filter(sampleRate, Sample(20000), Sample(0.5))
    , gainEnvelope(sampleRate, Sample(0.2), Sample(0.5), Sample(0.2), Sample(1))
    , filterEnvelope(sampleRate, Sample(0.2), Sample(0.5), Sample(0.2), Sample(1))
    , modEnvelope(sampleRate, 0, 1)
  {
  }

  void setup(
    int32_t noteId,
    Sample normalizedKey,
    Sample frequency,
    Sample velocity,
    GlobalParameter &param);
  void release();
  void rest();
  void reset();
  Sample process(NoteProcessInfo<Sample> &info);
};

class DSPCore {
public:
  DSPCore();

  static const size_t maxVoice = 32;
  GlobalParameter param;

  void setup(double sampleRate);
  void free();    // Release memory.
  void reset();   // Stop sounds.
  void startup(); // Reset phase, random seed etc.
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity);
  void noteOff(int32_t noteId);

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
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
  float sampleRate = 44100.0f;
  float lfoPhase = 0.0f;
  float lfoValue = 0.0f;

  Pink<float> noise{0};

  NoteProcessInfo<float> noteInfo;

  ExpSmoother<float> interpMasterGain;
  ExpSmoother<float> interpOsc1Gain;
  ExpSmoother<float> interpOsc1Pitch;
  ExpSmoother<float> interpOsc1Sync;
  ExpSmoother<float> interpOsc2Gain;
  ExpSmoother<float> interpOsc2Pitch;
  ExpSmoother<float> interpOsc2Sync;
  ExpSmoother<float> interpFMOsc1ToSync1;
  ExpSmoother<float> interpFMOsc1ToFreq2;
  ExpSmoother<float> interpFMOsc2ToSync1;
  ExpSmoother<float> interpModEnvelopeToFreq1;
  ExpSmoother<float> interpModEnvelopeToSync1;
  ExpSmoother<float> interpModEnvelopeToFreq2;
  ExpSmoother<float> interpModEnvelopeToSync2;
  ExpSmoother<float> interpModLFOFrequency;
  ExpSmoother<float> interpModLFONoiseMix;
  ExpSmoother<float> interpModLFOToFreq1;
  ExpSmoother<float> interpModLFOToSync1;
  ExpSmoother<float> interpModLFOToFreq2;
  ExpSmoother<float> interpModLFOToSync2;
  ExpSmoother<float> interpGainEnvelopeCurve;
  ExpSmoother<float> interpFilterCutoff;
  ExpSmoother<float> interpFilterResonance;
  ExpSmoother<float> interpFilterFeedback;
  ExpSmoother<float> interpFilterSaturation;
  ExpSmoother<float> interpFilterCutoffAmount;
  ExpSmoother<float> interpFilterResonanceAmount;
  ExpSmoother<float> interpFilterKeyToCutoff;
  ExpSmoother<float> interpFilterKeyToFeedback;

  size_t nVoice = 32;
  std::array<std::array<std::unique_ptr<Note<float>>, 2>, maxVoice> notes;

  // Transition happens when synth is playing all notes and user send a new note on.
  // transitionBuffer is used to store release of a note to reduce pop noise.
  std::vector<float> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};
