// (c) 2023 Takamitsu Endo
//
// This file is part of GrowlSynth.
//
// GrowlSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrowlSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrowlSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "oscillator.hpp"

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
  void process(const size_t length, float *out0, float *out1);
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
  static constexpr size_t upFold = 2;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  DecibelScale<double> velocityMap{-60, 0, true};
  DecibelScale<double> velocityToCouplingDecayMap{-40, 0, false};
  double velocity = 0;

  std::minstd_rand rng{0};
  std::minstd_rand formantRng{0};

  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 69.0;
  double pitchSmoothingKp = 1.0;
  ExpSmootherLocal<double> interpPitch;
  ExpSmootherLocal<double> noteFrequency;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> envelopeAM;
  ExpSmoother<double> pulseGain;
  ExpSmoother<double> pulsePitchOctave;
  ExpSmoother<double> pulseBendOctave;
  ExpSmoother<double> pulsePitchModMix;
  ExpSmoother<double> pulseFormantOctave;
  ExpSmoother<double> breathGain;
  ExpSmoother<double> breathFormantOctave;
  ExpSmoother<double> combFollowNote;
  ExpSmoother<double> combFeedbackFollowEnvelope;

  DoubleEmaADEnvelope<double> mainEnvelope;
  TransitionReleaseSmoother<double> releaseSmoother;
  AccumulateAM<double> accumulateAM;
  BlitOscillator<double> blitOsc;
  MaybeFormant<double, decltype(formantRng)> blitFormant;
  EnvelopedNoise<double> breathNoise;
  NoiseFormant<double, decltype(formantRng)> breathFormant;
  ModCombScaler<double, decltype(rng), nModDelay> modCombScaler;
  ParallelModComb<double, decltype(formantRng), nModDelay> modComb;
  NoteGate<double> noteGate;
  HalfBandIIR<double, HalfBandCoefficient<double>> halfbandIir;
  SVFHighpass<double> safetyHighpass;

  double calcNotePitch(double note);
  double processSample();
};
