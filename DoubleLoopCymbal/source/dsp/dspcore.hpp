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
  void noteOff(int_fast32_t noteId);

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
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }

private:
  void updateUpRate();
  void updateDelayTime();
  double calcNotePitch(double note);
  std::array<double, 2> processFrame(const std::array<double, 2> &externalInput);

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  DecibelScale<double> velocityMap{-60, 0, true};
  DecibelScale<double> velocityToCouplingDecayMap{-40, 0, false};
  double velocity = 0;

  static constexpr size_t upFold = 2;
  static constexpr std::array<size_t, 2> fold{1, upFold};
  size_t overSampling = 2;
  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 69.0;
  double pitchSmoothingKp = 1.0;
  ExpSmootherLocal<double> interpPitch;

  ExpSmoother<double> externalInputGain;
  ExpSmoother<double> noiseSustainLevel;
  ExpSmoother<double> delayTimeModAmount;
  ExpSmoother<double> allpassFeed1;
  ExpSmoother<double> allpassFeed2;
  ExpSmoother<double> allpassMixSpike;
  ExpSmoother<double> allpassMixAltSign;
  ExpSmoother<double> highShelfCutoff;
  ExpSmoother<double> highShelfGain;
  ExpSmoother<double> lowShelfCutoff;
  ExpSmoother<double> lowShelfGain;
  ExpSmoother<double> notchMix;
  ExpSmoother<double> notchNarrowness;
  ExpSmoother<double> stereoBalance;
  ExpSmoother<double> stereoMerge;
  ExpSmoother<double> outputGain;

  bool useExternalInput = false;

  std::minstd_rand noiseRng{0};
  std::minstd_rand paramRng{0};
  double impulse = 0;
  TransitionReleaseSmoother<double> releaseSmoother;
  ExpDecay<double> envelopeNoise;
  ExpDecay<double> envelopeRelease;
  ExpADEnvelope<double> envelopeClose;
  std::array<double, 2> feedbackBuffer1{};
  std::array<double, 2> feedbackBuffer2{};
  std::array<SerialAllpass<double, nAllpass, nNotch>, 2> serialAllpass1;
  std::array<SerialAllpass<double, nAllpass, nNotch>, 2> serialAllpass2;

  std::array<std::array<double, 2>, 2> halfbandInput{};
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
