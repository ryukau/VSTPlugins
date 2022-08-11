// (c) 2022 Takamitsu Endo
//
// This file is part of UltrasonicRingMod.
//
// UltrasonicRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltrasonicRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltrasonicRingMod.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

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
  static constexpr size_t upFold = 64;
  static constexpr size_t firstStateFold = Sos64FoldFirstStage<float>::fold;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  float sampleRate = 44100.0f;
  float upRate = upFold * 44100.0f;

  float pitchSmoothingKp = 1.0f;
  ExpSmootherLocal<float> interpPitch;

  ExpSmoother<float> interpPreClipGain;
  ExpSmoother<float> interpOutputGain;
  ExpSmoother<float> interpMix;
  ExpSmoother<float> interpFrequencyHz;
  ExpSmoother<float> interpDCOffset;
  ExpSmoother<float> interpFeedbackGain;
  ExpSmoother<float> interpModFrequencyScaling;
  ExpSmoother<float> interpModWrapMix;
  ExpSmoother<float> interpHardclipMix;

  std::array<LinearUpSampler<float, upFold>, 2> upSampler;
  std::array<DecimationLowpass<float, Sos64FoldFirstStage<float>>, 2> firstStageLowpass;
  std::array<float, 2> feedback;
  std::array<std::array<float, 2>, 2> halfBandInput;
  std::array<HalfBandIIR<float, HalfBandCoefficient<float>>, 2> halfbandIir;

  float phase = 0;

  float calcNotePitch(float note, float equalTemperament = 12.0f);
};
