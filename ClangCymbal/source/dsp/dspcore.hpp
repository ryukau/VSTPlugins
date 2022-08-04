// (c) 2022 Takamitsu Endo
//
// This file is part of ClangCymbal.
//
// ClangCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "fdn.hpp"
#include "lfo.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

enum class NoteState { active, release, rest };

class Note {
private:
  float velocity = 0;
  float eqTemp = float(12);

  uint32_t previousSeed = 0;
  pcg64 rng;
  std::vector<std::vector<float>> fdnMatrixRandomBase;

  ExpSmoother<float> modEnvelopeToFdnPitch;
  ExpSmoother<float> modEnvelopeToFdnOvertoneAdd;
  ExpSmoother<float> modEnvelopeToOscJitter;
  ExpSmoother<float> modEnvelopeToOscNoisePulseRatio;
  ExpSmoother<float> oscBounce;
  ExpSmoother<float> oscBounceCurve;
  ExpSmoother<float> oscJitter;
  ExpSmoother<float> oscDensity;
  ExpSmoother<float> oscPulseGainRandomness;
  ExpSmoother<float> oscNoisePulseRatio;
  ExpSmoother<float> fdnFreqOffset;
  ExpSmoother<float> fdnOvertoneOffset;
  ExpSmoother<float> fdnOvertoneMul;
  ExpSmoother<float> fdnOvertoneAdd;
  ExpSmoother<float> fdnOvertoneModulo;
  ExpSmoother<float> fdnFeedback;
  ExpSmoother<float> tremoloMix;
  ExpSmoother<float> tremoloDepth;
  ExpSmoother<float> tremoloDelayTime;
  ExpSmoother<float> tremoloModToDelayTimeOffset;
  ExpSmoother<float> tremoloModSmoothingKp;

  TableLFO<float, nModEnvelopeWavetable + 1, 1024, TableLFOType::envelope> envelopeTable;
  EnvelopePhase<float> modEnvelopePhase;

  float impulse = 0;
  DoubleEmaADEnvelope<float> oscEnvelope;
  DoubleEMAFilter<float> oscEnvelopeSmoother;
  SampleAndHoldNoise<float> pulsar;
  GenericSVF<float, 0> oscLowpass;

  ExpSmootherLocal<float> fdnPitch;
  float pitchSlideKp = float(1);
  bool fdnEnable = true;
  std::array<float, fdnMatrixSize> overtoneRandomness{};
  FeedbackDelayNetwork<float, fdnMatrixSize> fdn;

  Tremolo<float> tremolo;
  DoubleEMAFilter<float> tremoloSmoother;

  NoteGate<float> gate;
  DoubleEMAFilter<float> gateSmoother;

public:
  NoteState state = NoteState::rest;
  int_fast32_t id = -1;

  Note();
  void setup(float sampleRate);
  void reset(float sampleRate, GlobalParameter &param);
  void setParameters(float sampleRate, GlobalParameter &param);
  void noteOn(
    int_fast32_t noteId,
    float notePitch,
    float velocity,
    float pan,
    float sampleRate,
    GlobalParameter &param);
  void slide(int_fast32_t noteId, float notePitch, float velocity, float sampleRate);
  void release(float sampleRate, GlobalParameter &param);
  float process(float sampleRate);
};

class DSPCore final {
public:
  GlobalParameter param;
  bool isInitialized = false;
  bool isPlaying = false;

  DSPCore();

  void setup(double sampleRate); // Initialize when sampling rate is changed.
  void reset();                  // Stop sounds.
  void startup();                // Reset phase, random seed etc.
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity);
  void noteOff(int_fast32_t noteId);
  void fillTransitionBuffer();

  struct MidiNote {
    bool isNoteOn;
    size_t frame;
    int_fast32_t id;
    int_fast16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    size_t frame,
    int_fast32_t noteId,
    int_fast16_t pitch,
    float tuning,
    float velocity)
  {
    MidiNote note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch;
    note.tuning = tuning;
    note.velocity = velocityMap.map(velocity);
    midiNotes.push_back(note);
  }

  void processMidiNote(size_t frame)
  {
    while (true) {
      auto it = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {
        return nt.frame == frame;
      });
      if (it == std::end(midiNotes)) return;
      if (it->isNoteOn)
        noteOn(it->id, it->pitch, it->tuning, it->velocity);
      else
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }

private:
  struct NoteInfo {
    int32_t id;
    float notePitch;
    float velocity;
  };

  std::vector<NoteInfo> noteStack;

  static constexpr size_t upFold = 2;
  float sampleRate = 44100.0f;
  float upRate = 88200.0f;
  DecibelScale<float> velocityMap{-60, 0, true};

  Note note;
  ExpSmoother<float> interpMasterGain;

  std::array<float, 2> halfIn{{}};
  HalfBandIIR<float, HalfBandCoefficient<float>> halfbandIir;

  std::vector<float> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};
