// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
  static constexpr size_t firstStateFold = Sos64FoldFirstStage<double>::fold;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  double pitchSmoothingKp = 1;
  ExpSmootherLocal<double> interpPitch;

  ExpSmoother<double> interpPreClipGain;
  ExpSmoother<double> interpOutputGain;
  ExpSmoother<double> interpMix;
  ExpSmoother<double> interpFrequencyHz;
  ExpSmoother<double> interpDCOffset;
  ExpSmoother<double> interpFeedbackGain;
  ExpSmoother<double> interpModFrequencyScaling;
  ExpSmoother<double> interpModWrapMix;
  ExpSmoother<double> interpHardclipMix;

  std::array<LinearUpSampler<double, upFold>, 2> upSampler;
  std::array<DecimationLowpass<double, Sos64FoldFirstStage<double>>, 2> firstStageLowpass;
  std::array<double, 2> feedback;
  std::array<std::array<double, 2>, 2> halfBandInput;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;

  double phase = 0;

  double calcNotePitch(double note, double equalTemperament = 12);
};
