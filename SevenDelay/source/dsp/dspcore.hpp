// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "delay.hpp"
#include "iir.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

// Lagrange delay is very slow at debug build. If that's the case set Order to 1.
using DelayTypeName = DelayLagrange<double, 7>;
using FilterTypeName = SomeDSP::SVF<double>;
using DCKillerTypeName = SomeDSP::BiquadHighPass<double>;

class DSPCore {
public:
  struct NoteInfo {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    float pitch;
    float velocity;
  };

  DSPCore()
  {
    midiNotes.reserve(1024);
    noteStack.reserve(1024);
  }

  GlobalParameter param;
  double tempo = 120.0f; // tempo is beat per minutes.

  void setup(double sampleRate);
  void reset();   // Stop sounds.
  void startup(); // Reset phase, random seed etc.
  void setParameters();

  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

  void noteOn(NoteInfo &info);
  void noteOff(int_fast32_t noteId);

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
    float tuning,
    float velocity)
  {
    NoteInfo note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch + tuning;
    note.velocity = velocity;
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

protected:
  void updateDelayTime();

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;
  double notePitchMultiplier = double(1);

  std::array<LinearSmoother<double>, 2> interpTime{};
  LinearSmoother<double> interpWetMix;
  LinearSmoother<double> interpDryMix;
  LinearSmoother<double> interpFeedback;
  LinearSmoother<double> interpLfoTimeAmount;
  LinearSmoother<double> interpLfoToneAmount;
  LinearSmoother<double> interpLfoFrequency;
  LinearSmoother<double> interpLfoShape;
  LinearSmoother<double> interpPanIn;
  LinearSmoother<double> interpSpreadIn;
  LinearSmoother<double> interpPanOut;
  LinearSmoother<double> interpSpreadOut;
  LinearSmoother<double> interpToneCutoff;
  LinearSmoother<double> interpToneQ;
  LinearSmoother<double> interpToneMix;
  LinearSmoother<double> interpDCKill;
  LinearSmoother<double> interpDCKillMix;

  double lfoPhase;
  double lfoPhaseTick;
  std::array<double, 2> delayOut{};
  std::array<DelayTypeName, 2> delay;
  std::array<FilterTypeName, 2> filter;
  std::array<DCKillerTypeName, 2> dcKiller;
};
