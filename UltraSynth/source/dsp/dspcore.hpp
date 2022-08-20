// (c) 2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lfo.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "filter.hpp"

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
  static constexpr size_t upFold = 64;
  static constexpr size_t firstStateFold = Sos64FoldFirstStage<float>::fold;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  DecibelScale<double> velocityMap{-36, 0, true};
  double velocity = 0;

  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 69.0;
  double pitchSmoothingKp = 1.0;
  double lowpassCutoffDecayKp = 1.0;
  ExpSmootherLocal<double> interpPitch;

  double baseRateKp = 1.0;
  ExpSmootherLocal<double> interpOutputGain;
  ExpSmootherLocal<double> interpLfoWaveShape;
  ExpSmootherLocal<double> interpLfoToPitch;
  ExpSmootherLocal<double> interpLfoToOscMix;
  ExpSmootherLocal<double> interpLfoToCutoff;
  ExpSmootherLocal<double> interpLfoToPreSaturation;
  ExpSmootherLocal<double> interpLfoToOsc1WaveShape;
  ExpSmootherLocal<double> interpLfoToOsc2WaveShape;

  LinearTempoSynchronizer<double> synchronizer;
  LFOPhase<double> lfoPhase;
  DoubleEMAFilter<double> lfoSmootherB;
  DoubleEMAFilter<double> lfoSmootherP;

  ExpSmoother<double> interpFrequencyHz;
  ExpSmoother<double> interpOsc1FrequencyOffsetPitch;
  ExpSmoother<double> interpOsc2FrequencyOffsetPitch;
  ExpSmoother<double> interpOsc1WaveShape;
  ExpSmoother<double> interpOsc2WaveShape;
  ExpSmoother<double> interpOsc1SawPulse;
  ExpSmoother<double> interpOsc2SawPulse;
  ExpSmoother<double> interpPhaseModFromLowpassToOsc1;
  ExpSmoother<double> interpPmPhase1ToPhase2;
  ExpSmoother<double> interpPmPhase2ToPhase1;
  ExpSmoother<double> interpPmOsc1ToPhase2;
  ExpSmoother<double> interpPmOsc2ToPhase1;
  ExpSmoother<double> interpOscMix;
  ExpSmoother<double> interpSvfG;
  ExpSmoother<double> interpSvfK;
  ExpSmoother<double> interpRectificationMix;
  ExpSmoother<double> interpSaturationMix;
  ExpSmoother<double> interpSustain;

  double feedback = 0;
  double phase1 = 0;
  double phase2 = 0;
  double o1 = 0;
  double o2 = 0;

  double gainAttackKp = 1.0;
  double gainDecayKp = 1.0;
  DoubleEMAFilter<double> attackEnvelope;
  DoubleEMAFilter<double> decayEnvelope;
  NoteGate<double> releaseEnvelope;

  SerialSVF<double> svf;

  DecimationLowpass<double, Sos64FoldFirstStage<double>> firstStageLowpass;
  std::array<double, 2> halfBandInput;
  HalfBandIIR<double, HalfBandCoefficient<double>> halfbandIir;

  double calcNotePitch(double note);
  double getTempoSyncInterval();
  void resetBuffer();
};
