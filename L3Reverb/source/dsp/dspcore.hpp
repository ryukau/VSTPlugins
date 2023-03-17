// (c) 2020 Takamitsu Endo
//
// This file is part of L3Reverb.
//
// L3Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// L3Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with L3Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "delay.hpp"

#include <array>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

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

  void setup(double sampleRate);
  void reset();
  void startup();
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

private:
  void refreshSeed();
  void updateDelayTime();

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;
  float notePitchMultiplier = float(1);

  float sampleRate = 44100.0f;

  std::minstd_rand timeRng{0};
  std::minstd_rand innerRng{0};
  std::minstd_rand d1FeedRng{0};
  std::minstd_rand d2FeedRng{0};
  std::minstd_rand d3FeedRng{0};
  std::minstd_rand d4FeedRng{0};
  uint_fast32_t timeSeed = 0;
  uint_fast32_t innerSeed = 0;
  uint_fast32_t d1FeedSeed = 0;
  uint_fast32_t d2FeedSeed = 0;
  uint_fast32_t d3FeedSeed = 0;
  uint_fast32_t d4FeedSeed = 0;

  std::array<NestD4<float, nSection1, nSection2, nSection3, nSection4>, 2> delay;
  std::array<float, 2> delayOut{};
  ExpSmoother<float> interpStereoCross;
  ExpSmoother<float> interpStereoSpread;
  ExpSmoother<float> interpDry;
  ExpSmoother<float> interpWet;
};
