// (c) 2022 Takamitsu Endo
//
// This file is part of OrdinaryPhaser.
//
// OrdinaryPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OrdinaryPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OrdinaryPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lfo.hpp"
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
  void updateUpRate();
  void processFrame(std::array<double, 2> &input);
  double getTempoSyncInterval();
  double calcNotePitch(double note, double scale, double equalTemperament = 12);

  static constexpr size_t upFold = 2;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  double pitchSmoothingKp = 1;
  double pitchReleaseKp = 1;
  ExpSmootherLocal<double> notePitchToDelayTime;
  ExpSmootherLocal<double> notePitchToAllpassCutoff;
  DoubleEMAFilter<double> notePitchToDelayTimeRelease;
  DoubleEMAFilter<double> notePitchToAllpassCutoffRelease;

  RotarySmoother<double> lfoPhaseConstant;
  RotarySmoother<double> lfoPhaseOffset;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> mix;
  ExpSmoother<double> cutoffSpread;
  ExpSmoother<double> cutoffMinHz;
  ExpSmoother<double> cutoffMaxHz;
  ExpSmoother<double> feedback;
  ExpSmoother<double> delayTimeSamples;
  ExpSmoother<double> lfoToDelay;
  ExpSmoother<double> inputToFeedbackGain;
  ExpSmoother<double> inputToDelayTime;

  bool oversampling = true;
  size_t lfoToDelayTuningType = 0;

  size_t currentAllpassStage = 0;
  size_t previousAllpassStage = 0;
  size_t transitionSamples = 2048;
  size_t transitionCounter = 0;

  LinearTempoSynchronizer<double> synchronizer;
  TableLFO<double, nLfoWavetable, 2048, 2> lfo;

  std::array<double, 2> feedbackBuffer{};
  std::array<double, 2> previousInput{};
  std::array<std::array<double, 2>, 2> upsampleBuffer{};
  std::array<std::array<ZDFOnePoleAllpass<double>, maxAllpass>, 2> allpass;
  std::array<Delay<double>, 2> feedbackDelay;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
