// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../parameter.hpp"
#include "constants.hpp"
#include "delay.hpp"
#include "envelope.hpp"
#include "iir.hpp"
#include "noise.hpp"
#include "oscillator.hpp"
#include "smoother.hpp"

#include <array>
#include <cmath>
#include <memory>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr size_t nPitch = 8;
constexpr size_t nChord = 4;
constexpr size_t nOvertone = 16;
constexpr size_t biquadOscSize = nPitch * nOvertone;

enum class NoteState { active, release, rest };

template<typename Sample> class Note {
public:
  NoteState state = NoteState::rest;

  Sample sampleRate = 44100;

  int32_t id = -1;
  Sample normalizedKey = 0;
  Sample velocity = 0;
  Sample gain = 0;
  Sample frequency = 0;

#ifdef __AVX2__
  std::array<BiquadOscAVX2<nOvertone>, nChord> oscillator;
#else
  std::array<BiquadOscN<Sample, biquadOscSize>, nChord> oscillator;
#endif
  std::array<Sample, nChord> chordPan{};

  ExpADSREnvelope<Sample> gainEnvelope;
  Sample gainEnvCurve = 0;

  void setup(Sample sampleRate);
  void noteOn(
    int32_t noteId,
    Sample normalizedKey,
    Sample frequency,
    Sample velocity,
    GlobalParameter &param,
    White<float> &rng);
  void release();
  void rest();
  std::array<Sample, 2> process();
};

class DSPCore {
public:
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

  void processMidiNote(uint32_t frame)
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

  White<float> rng{0};

  size_t nVoice = 32;
  std::array<Note<float>, maxVoice> notes;
  float lastNoteFreq = 1.0f;

  std::array<Chorus<float>, 3> chorus;

  LinearSmoother<float> interpTremoloMix;
  LinearSmoother<float> interpMasterGain;

  // Transition happens when synth is playing all notes and user send a new note on.
  // transitionBuffer is used to store a release of a note to reduce pop noise.
  std::vector<std::array<float, 2>> transitionBuffer{};
  bool isTransitioning = false;
  size_t mptIndex = 0; // mpt for Max Poly Transition.
  size_t mptStop = 0;
};
