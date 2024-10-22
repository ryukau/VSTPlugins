// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "iir.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <vector>

using namespace SomeDSP;
using namespace Steinberg::Synth;

struct NoteInfo {
  int32_t id;
  float frequency;
};

template<typename Sample> class TpzMono {
public:
  const static int32_t rngPitchDriftSeed = 987654321;

  const std::array<Sample, 9> octaveTable{0.0625, 0.125, 0.25, 0.5, 1.0,
                                          2.0,    4.0,   8.0,  16.0};

  Sample feedbackBuffer = 0;
  Sample tpzOsc2Buffer = 0;
  PTRTrapezoidOsc tpzOsc1{44100, 0};
  PTRTrapezoidOsc tpzOsc2{44100, 0};
  Random<float> rngPitchDrift{rngPitchDriftSeed};
  DecimationLowpass<Sample> decimationLP;

  SerialZDF1Pole<Sample> filter;

  LFO<Sample> lfo;

  ADSREnvelope<
    Sample,
    TableCurve<Sample, EnvelopeCurveType::attack, 128>,
    TableCurve<Sample, EnvelopeCurveType::decay, 128>,
    TableCurve<Sample, EnvelopeCurveType::decay, 128>>
    gainEnvelope;

  ADSREnvelope<
    Sample,
    TableCurve<Sample, EnvelopeCurveType::attack, 128>,
    TableCurve<Sample, EnvelopeCurveType::decay, 128>,
    TableCurve<Sample, EnvelopeCurveType::decay, 128>>
    filterEnvelope;

  PolyExpEnvelope<double> modEnvelope1;
  PolyExpEnvelope<double> modEnvelope2;

  AMPitchShiter<Sample> shifter1;
  AMPitchShiter<Sample> shifter2;

  Sample noteFreq = 0;
  Sample normalizedKey = 0;

  LinearSmootherLocal<Sample> interpOctave;
  LinearSmootherLocal<Sample> interpOsc1Pitch;
  LinearSmootherLocal<Sample> interpOsc2Pitch;
  LinearSmoother<Sample> interpOsc1Slope;
  LinearSmoother<Sample> interpOsc1PulseWidth;
  LinearSmoother<Sample> interpOsc2Slope;
  LinearSmoother<Sample> interpOsc2PulseWidth;
  LinearSmoother<Sample> interpPhaseMod;
  LinearSmoother<Sample> interpOscMix;
  LinearSmoother<Sample> interpPitchDrift;
  LinearSmoother<Sample> interpFeedback;
  LinearSmoother<Sample> interpFilterCutoff;
  LinearSmoother<Sample> interpFilterFeedback;
  LinearSmoother<Sample> interpFilterSaturation;
  LinearSmoother<Sample> interpFilterEnvToCutoff;
  LinearSmoother<Sample> interpFilterKeyToCutoff;
  LinearSmoother<Sample> interpOscMixToFilterCutoff;
  LinearSmoother<Sample> interpMod1EnvToPhaseMod;
  LinearSmoother<Sample> interpMod2EnvToFeedback;
  LinearSmoother<Sample> interpMod2EnvToLFOFrequency;
  LinearSmoother<Sample> interpModEnv2ToOsc2Slope;
  LinearSmoother<Sample> interpMod2EnvToShifter1;
  LinearSmoother<Sample> interpLFOFrequency;
  LinearSmoother<Sample> interpLFOShape;
  LinearSmoother<Sample> interpLFOToPitch;
  LinearSmoother<Sample> interpLFOToSlope;
  LinearSmoother<Sample> interpLFOToPulseWidth;
  LinearSmoother<Sample> interpLFOToCutoff;
  LinearSmoother<Sample> interpShifter1Pitch;
  LinearSmoother<Sample> interpShifter1Gain;
  LinearSmoother<Sample> interpShifter2Pitch;
  LinearSmoother<Sample> interpShifter2Gain;

  void setup(Sample sampleRate);
  void reset(GlobalParameter &param);
  void startup();
  void setParameters(Sample tempo, GlobalParameter &param);
  void
  noteOn(bool wasResting, Sample frequency, Sample normalizedKey, GlobalParameter &param);
  void noteOff(Sample frequency);
  void release(bool resetPitch);
  Sample process(const size_t bufferSize);

private:
  Sample getOctave(GlobalParameter &param);
  Sample getOsc1Pitch(GlobalParameter &param);
  Sample getOsc2Pitch(GlobalParameter &param);
};

class DSPCore {
public:
  static const size_t maxVoice = 32;
  GlobalParameter param;

  DSPCore();

  void setup(double sampleRate);
  void free();    // Release memory.
  void reset();   // Stop sounds.
  void startup(); // Reset phase, random seed etc.
  void setParameters(double tempo);
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity);
  void noteOff(int32_t noteId);

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
    float tuning,
    float velocity)
  {
    MidiNote note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch;
    note.tuning = tuning;
    note.velocity = velocity;
    midiNotes.push_back(note);
  }

  void processMidiNote(uint32_t frame)
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
  float sampleRate = 44100.0f;

  float velocity = 0;
  std::vector<NoteInfo> noteStack; // Top of this stack is current note.

  TpzMono<float> tpz1;

  LinearSmoother<float> interpMasterGain;
};
