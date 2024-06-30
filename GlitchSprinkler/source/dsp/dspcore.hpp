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

class DSPCore {
public:
  struct NoteInfo {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int channel;
    int noteNumber;
    float cent;
    float velocity;
  };

  DSPCore()
  {
    midiNotes.reserve(1024);
    activeNote.reserve(1024);
    activeModifier.reserve(1024);
  }

  GlobalParameter param;
  bool isPlaying = false;
  double tempo = 120.0;
  double beatsElapsed = 0.0;
  double previousBeatsElapsed = 0.0;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

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

private:
  std::array<double, 2> processFrame(double currentBeat);
  void updateNote();
  void resetArpeggio();

  // Maybe make it possible to change the pitch modifier channel.
  static constexpr size_t pitchModifierChannel = 15;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> activeNote;
  std::vector<NoteInfo> activeModifier;

  DecibelScale<double> velocityMap{-60, 0, true};
  double velocity = 0;

  double sampleRate = 44100.0;

  ExpSmoother<double> outputGain;

  std::minstd_rand rngParam{0};
  std::minstd_rand rngArpeggio{0};

  uint_fast32_t arpeggioDuration = std::numeric_limits<uint_fast32_t>::max();
  uint_fast32_t arpeggioTie = 1;
  uint_fast32_t arpeggioTimer = 0;
  uint_fast32_t arpeggioLoopLength = std::numeric_limits<uint_fast32_t>::max();
  uint_fast32_t arpeggioLoopCounter = 0;

  bool scheduleUpdateNote = false;
  uint_fast32_t phasePeriod = 0;
  uint_fast32_t phaseCounter = 0;
  double oscSync = double(1);
  double fmIndex = double(0);
  double saturationGain = double(1);
  double decayGain = double(0);
  double decayRatio = double(1);
  PolynomialCoefficientSolver<double, nPolyOscControl> polynomial;
};
