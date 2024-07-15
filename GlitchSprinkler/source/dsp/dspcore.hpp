// (c) 2024 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "polynomial.hpp"

#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

struct NoteInfo {
  bool isNoteOn;
  uint32_t frame;
  int32_t id;
  int32_t channel;
  int32_t noteNumber;
  float cent;
  float velocity;
};

class DSPCore;
using PolySolver = PolynomialCoefficientSolver<double, nPolyOscControl>;

class Voice {
private:
  DSPCore &core;

public:
  Voice(DSPCore &core) : core(core) {}

  enum class State { active, release, terminate, rest };
  State state = State::rest;
  double lastGain = 0; // Used for note stealing to choose the most quite voice.

  int32_t noteId = -1;
  int32_t noteNumber = 0;
  double noteCent = 0;
  double noteVelocity = 0;
  double notePan = 0.5;

  std::minstd_rand rngArpeggio{0};

  uint_fast32_t arpeggioTie = 1;
  uint_fast32_t arpeggioTimer = 0;
  uint_fast32_t arpeggioLoopCounter = 0;

  bool scheduleUpdateNote = false;
  uint_fast32_t phasePeriod = 0;
  uint_fast32_t phaseCounter = 0;

  unsigned unisonIndex = 1;
  double unisonGain = double(1);
  double unisonPan = double(0.5);
  double unisonPanNext = double(0.5);
  double unisonCentNext = double(0);

  double oscSync = double(1);
  double fmIndex = double(0);
  double saturationGain = double(1);
  double decayGain = double(0);
  double decayRatio = double(1);
  std::array<double, PolySolver::nPolynomialPoint> polynomialCoefficients{};

  void reset();
  void setParameters();
  std::array<double, 2> processFrame();
  void updateNote();
  void resetArpeggio(unsigned seed);
};

class DSPCore {
public:
  GlobalParameter param;

  double sampleRate = 44100.0;

  bool isPlaying = false;
  double tempo = 120.0;
  double beatsElapsed = 0.0;
  double previousBeatsElapsed = 0.0;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

  unsigned seed = 0;
  double currentBeat = 0;
  double pitchModifier = double(1);
  bool isPolyphonic = true;
  Voice::State noteOffState = Voice::State::terminate;

  uint_fast32_t arpeggioDuration = std::numeric_limits<uint_fast32_t>::max();
  uint_fast32_t arpeggioLoopLength = std::numeric_limits<uint_fast32_t>::max();

  DecibelScale<double> velocityMap{-60, 0, true};
  ExpSmoother<double> outputGain;

  bool isPolynomialUpdated = false;
  PolySolver polynomial;

  // Maybe make it possible to change the pitch modifier channel.
  static constexpr size_t pitchModifierChannel = 15;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> activeNote;
  std::vector<NoteInfo> activeModifier;
  std::vector<size_t> noteIndices;

  unsigned nextSteal = 0;
  std::vector<Voice> voices;

  DSPCore()
  {
    midiNotes.reserve(2048);
    activeNote.reserve(2048);
    activeModifier.reserve(2048);

    noteIndices.reserve(2048);
    std::fill(noteIndices.begin(), noteIndices.end(), size_t(0));

    voices.reserve(256);
    for (size_t i = 0; i < 256; ++i) voices.emplace_back(*this);
  }

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(NoteInfo &info);
  void noteOff(int_fast32_t noteId);

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t channel,
    int16_t noteNumber,
    float tuning,
    float velocity)
  {
    NoteInfo note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.channel = channel;
    note.noteNumber = noteNumber;
    note.cent = tuning;
    note.velocity = velocity;

    if (
      midiNotes.back().channel == pitchModifierChannel && midiNotes.back().frame == frame
      && channel != pitchModifierChannel)
    {
      std::swap(note, midiNotes.back());
    }
    midiNotes.push_back(note);
  }

  void processMidiNote(size_t frame)
  {
    while (true) {
      auto it = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const NoteInfo &nt) {
        return nt.frame == frame;
      });
      if (it == std::end(midiNotes)) return;
      if (it->isNoteOn)
        noteOn(*it);
      else
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }
};
