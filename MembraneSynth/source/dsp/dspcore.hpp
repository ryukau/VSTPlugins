// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "fdn.hpp"

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

    fdnMatrixRandomBase.resize(fdnSize);
    for (size_t i = 0; i < fdnMatrixRandomBase.size(); ++i) {
      fdnMatrixRandomBase[i].resize(fdnSize - i);
    }
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
  static constexpr std::array<double, fdnSize> circularMode{
    1.000000000000000,  1.593340505695112,  2.1355487866494034, 2.295417267427694,
    2.6530664045492145, 2.9172954551172228, 3.155464815408362,  3.5001474903090264,
    3.5984846739581138, 3.6474511791052775, 4.058931883331434,  4.131738159726707,
    4.230439127905234,  4.6010445344331075, 4.610051645437306,  4.831885262930598,
  };

  static constexpr size_t upFold = 2;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  DecibelScale<double> velocityMap{-60, 0, true};
  double velocity = 0;

  double pulseAmp = 1;
  bool enableModEnv = true;

  double sampleRate = 44100.0;
  double upRate = upFold * 44100.0;

  double noteNumber = 69.0;
  double pitchSmoothingKp = 1.0;
  ExpSmootherLocal<double> interpPitch;
  ExpSmootherLocal<double> frequencyHz;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> fdnShape;
  ExpSmoother<double> fdnFeedback;
  ExpSmoother<double> fdnModulation;
  ExpSmoother<double> fdnInterpRate;
  ExpSmoother<double> fdnMinModulation;

  uint32_t previousSeed = 0;
  pcg64 rng;
  std::vector<std::vector<double>> fdnMatrixRandomBase;
  std::array<double, fdnSize> fdnAdditionalOvertone;

  PulseGenerator<double> pulse;
  SREnvelope<double> modulationEnvelope;
  ModulatedFDN<double, fdnSize> fdn;
  HalfBandIIR<double, HalfBandCoefficient<double>> halfbandIir;

  double calcNotePitch(double note);
  double processSample();
};
