// (c) 2024 Takamitsu Endo
//
// This file is part of DoubleLoopCymbal.
//
// DoubleLoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DoubleLoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DoubleLoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "../parameter.hpp"
#include "delay.hpp"

#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  struct NoteInfo {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    float noteNumber;
    float velocity;
  };

  DSPCore()
  {
    midiNotes.reserve(1024);
    noteStack.reserve(1024);
  }

  GlobalParameter param;
  bool isPlaying = false;
  double tempo = 120.0;
  double beatsElapsed = 0.0;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);
  void noteOn(NoteInfo &info);
  void noteOff(int_fast32_t noteId, double noteOffVelocity);

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t noteNumber,
    float tuning,
    float velocity)
  {
    NoteInfo note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.noteNumber = noteNumber + tuning;
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
        noteOff(it->id, it->velocity);
      midiNotes.erase(it);
    }
  }

private:
  void updateUpRate();
  void updateDelayTime();
  double calcNotePitch(double note);
  double processFrame(const std::array<double, 2> &externalInput);

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  double noteOnVelocity = 0;

  static constexpr size_t upFold = 2;
  static constexpr std::array<size_t, 2> fold{1, upFold};
  size_t overSampling = 2;
  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 69.0;
  double pitchSmoothingKp = 1.0;
  ExpSmootherLocal<double> interpPitch;

  ExpSmoother<double> externalInputGain;
  ExpSmoother<double> noiseTextureMix;
  ExpSmoother<double> halfClosedGain;
  ExpSmoother<double> halfClosedDensity;
  ExpSmoother<double> halfClosedHighpassCutoff;
  ExpSmoother<double> delayTimeModAmount;
  ExpSmoother<double> allpassFeed1;
  ExpSmoother<double> allpassFeed2;
  ExpSmoother<double> allpassMixSpike;
  ExpSmoother<double> allpassMixAltSign;
  ExpSmoother<double> highShelfCutoff;
  ExpSmoother<double> highShelfGain;
  ExpSmoother<double> lowShelfCutoff;
  ExpSmoother<double> lowShelfGain;
  ExpSmoother<double> outputGain;

  bool useExternalInput = false;
  double halfClosedDensityScaler = double(1);
  double halfClosedHighpassScaler = double(1);
  double allpassFilterScaler = double(1);
  double delayTimeModOffset = 0;

  pcg64 rngNoiseRolling{0};
  pcg64 rngNoisePinned{0};
  pcg64 rngParam{0};
  double impulse = 0;
  TransitionReleaseSmoother<double> releaseSmoother;
  ExpDecay<double> envelopeNoise;
  ExpDSREnvelope<double> envelopeHalfClosed;
  ExpSREnvelope<double> envelopeRelease;
  ExpADEnvelope<double> envelopeClose;
  HalfClosedNoise<double> halfClosedNoise;
  double feedbackBuffer1 = 0;
  double feedbackBuffer2 = 0;
  SerialAllpass<double, nAllpass> serialAllpass1;
  SerialAllpass<double, nAllpass> serialAllpass2;

  double baseSampleRateKp = double(1);
  ExpSmootherLocal<double> spreaderSplit;
  ExpSmootherLocal<double> spreaderSpread;
  Spreader<double> spreader;

  std::array<double, 2> prevExtIn{};
  HalfBandIIR<double, HalfBandCoefficient<double>> halfbandIir;
};
