// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/basiclimiter.hpp"
#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "easygate.hpp"
#include "parallelcomb.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

using OverSampler = OverSampler16<float>;

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
  size_t getLatency();
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
  std::array<float, 2> processInternal(float ch0, float ch1);
  void updateDelayTime();

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;
  float notePitchMultiplier = float(1);

  float sampleRate = 44100.0f;
  std::array<float, 2> delayOut{};

  ExpSmoother<float> interpCombInterpRate;
  ExpSmoother<float> interpCombInterpCutoffKp;
  ExpSmoother<float> interpFeedback;
  ExpSmoother<float> interpFeedbackHighpassCutoffKp;
  ExpSmoother<float> interpStereoCross;
  ExpSmoother<float> interpFeedbackToDelayTime;
  ExpSmoother<float> interpGateReleaseKp;
  ExpSmoother<float> interpDry;
  ExpSmoother<float> interpWet;

  std::array<OverSampler, 2> overSampler;
  EasyGate<float> gate;
  std::array<ParallelComb<float, nCombTaps>, 2> comb;
  std::array<EMAHighpass<float, 4>, 2> feedbackHighpass;
  std::array<BasicLimiter<float>, 2> feedbackLimiter;
};
