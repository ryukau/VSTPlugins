// (c) 2023 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "filter.hpp"

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
  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

  bool tooMuchFeedback = false;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length,
    const float *in0,
    const float *in1,
    const float *in2,
    const float *in3,
    float *out0,
    float *out1);
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
  void updateUpRate();
  std::array<double, 2>
  processFrame(const std::array<double, 2> &input, const std::array<double, 2> &modSig);
  double calcNotePitch(double note, double scale, double equalTemperament = 12);

  static constexpr size_t upFold = 8;
  static constexpr std::array<size_t, 3> fold{1, 2, upFold};

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  double pitchSmoothingKp = 1;
  double pitchReleaseKp = 1;
  ExpSmootherLocal<double> notePitchToAllpassCutoff;
  DoubleEMAFilter<double> notePitchToAllpassCutoffRelease;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> feedbackMix;
  ExpSmoother<double> inputMixSign;
  ExpSmoother<double> feedback;
  ExpSmoother<double> feedbackClip;
  ExpSmoother<double> feedbackHighpassG;
  ExpSmoother<double> outputHighpassG;
  ExpSmoother<double> modAmount;
  ExpSmoother<double> modAsymmetry;
  ExpSmoother<double> modLowpassKp;
  ExpSmoother<double> allpassSpread;
  ExpSmoother<double> allpassCenterCut;

  size_t oversampling = 1;
  size_t modType = 0;

  size_t currentAllpassStage = 0;
  size_t previousAllpassStage = 0;
  size_t transitionSamples = 2048;
  size_t transitionCounter = 0;

  std::array<double, 2> feedbackBuffer{};
  std::array<EMAFilter<double>, 2> modLowpass{};
  std::array<std::array<ZDFOnePoleAllpass<double>, maxAllpass>, 2> allpass;
  std::array<SVF<double>, 2> feedbackHighpass{};
  std::array<SVF<double>, 2> outputHighpass{};

  std::array<CubicUpSampler<double, upFold>, 4> upSampler;
  std::array<DecimationLowpass<double, Sos8FoldFirstStage<double>>, 2> decimationLowpass;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
