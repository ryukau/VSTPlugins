// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

  static constexpr size_t upFold = 2;
  static constexpr std::array<size_t, 2> fold{1, upFold};
  size_t overSampling = 2;
  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 60.0;
  double noteOnVelocity = 0;
  double pitchSmoothingKp = 1.0;
  ExpSmootherLocal<double> interpPitch;

  ExpSmoother<double> externalInputGain;
  ExpSmoother<double> impactTextureMix;
  ExpSmoother<double> impactHighpassCutoff;
  ExpSmoother<double> halfClosedGain;
  ExpSmoother<double> halfClosedSustain;
  ExpSmoother<double> halfClosedDensity;
  ExpSmoother<double> halfClosedHighpassCutoff;
  ExpSmoother<double> closingHighpassCutoff;
  ExpSmoother<double> delayTimeModAmount;
  ExpSmoother<double> allpassLoopGain;
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
  double impactHighpassScaler = double(1);
  double halfClosedDensityScaler = double(1);
  double halfClosedHighpassScaler = double(1);
  double closingHighpassScaler = double(1);
  double allpassLoopGainScaler = double(1);
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
  Highpass2<double> impactHighpass;
  HalfClosedNoise<double> halfClosedNoise;
  ClosingNoise<double> closingNoise;
  double feedbackBuffer1 = 0;
  double feedbackBuffer2 = 0;
  AllpassLoop<double, nAllpass> allpassLoop1;
  AllpassLoop<double, nAllpass> allpassLoop2;

  double baseSampleRateKp = double(1);
  ExpSmootherLocal<double> spreaderSplit;
  ExpSmootherLocal<double> spreaderSpread;
  Spreader<double> spreader;

  std::array<double, 2> prevExtIn{};
  HalfBandIIR<double, HalfBandCoefficient<double>> halfbandIir;
};
