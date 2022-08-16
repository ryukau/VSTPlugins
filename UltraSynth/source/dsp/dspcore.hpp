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
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;

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

  float sampleRate = 44100.0f;
  float upRate = upFold * 44100.0f;

  float noteNumber = 60.0f;
  float pitchSmoothingKp = 1.0f;
  float lowpassCutoffDecayKp = 1.0f;
  ExpSmootherLocal<float> interpPitch;

  float baseRateKp = 1.0f;
  ExpSmootherLocal<float> interpOutputGain;
  ExpSmootherLocal<float> interpLfoToPitch;
  ExpSmootherLocal<float> interpLfoToOscMix;
  ExpSmootherLocal<float> interpLfoToCutoff;
  ExpSmootherLocal<float> interpLfoToPreSaturation;
  ExpSmootherLocal<float> interpLfoToOsc1WaveShape;
  ExpSmootherLocal<float> interpLfoToOsc2WaveShape;

  LinearTempoSynchronizer<float> synchronizer;
  LFOPhase<float> lfoPhase;
  DoubleEMAFilter<double> lfoSmootherB;
  DoubleEMAFilter<double> lfoSmootherP;

  ExpSmoother<float> interpFrequencyHz;
  ExpSmoother<float> interpOsc1FrequencyOffsetPitch;
  ExpSmoother<float> interpOsc2FrequencyOffsetPitch;
  ExpSmoother<float> interpOsc1WaveShape;
  ExpSmoother<float> interpOsc2WaveShape;
  ExpSmoother<float> interpOsc1SawPulse;
  ExpSmoother<float> interpOsc2SawPulse;
  ExpSmoother<float> interpPhaseModFromLowpassToOsc1;
  ExpSmoother<float> interpPmPhase1ToPhase2;
  ExpSmoother<float> interpPmPhase2ToPhase1;
  ExpSmoother<float> interpPmOsc1ToPhase2;
  ExpSmoother<float> interpPmOsc2ToPhase1;
  ExpSmoother<float> interpOscMix;
  ExpSmoother<float> interpSvfG;
  ExpSmoother<float> interpSvfK;
  ExpSmoother<float> interpRectificationMix;
  ExpSmoother<float> interpSaturationMix;
  ExpSmoother<float> interpSustain;

  double feedback = 0;
  double phase1 = 0;
  double phase2 = 0;
  double o1 = 0;
  double o2 = 0;

  double gainAttackKp = 1.0f;
  double gainDecayKp = 1.0f;
  DoubleEMAFilter<double> attackEnvelope;
  DoubleEMAFilter<double> decayEnvelope;
  NoteGate<double> releaseEnvelope;

  SerialSVF<double> svf;

  DecimationLowpass<float, Sos64FoldFirstStage<float>> firstStageLowpass;
  std::array<float, 2> halfBandInput;
  HalfBandIIR<float, HalfBandCoefficient<float>> halfbandIir;

  float calcNotePitch(float note);
  float getTempoSyncInterval();
  void resetBuffer();
};
