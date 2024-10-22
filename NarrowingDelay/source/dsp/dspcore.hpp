// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
  std::array<double, 2> processFrame(double in0, double in1);
  double getTempoSyncInterval();
  double calcNotePitch(double note, double equalTemperament = 12);

  static constexpr size_t maxUpFold = 8;

  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> noteStack;

  double sampleRate = 44100;
  double upRate = maxUpFold * 44100;

  double phaseSyncKp = 1;
  double pitchSmoothingKp = 1;
  ExpSmootherLocal<double> notePitch;

  RotarySmoother<double> lfoPhaseConstant;
  RotarySmoother<double> lfoPhaseOffset;

  ExpSmoother<double> lfoShapeClip;
  ExpSmoother<double> lfoShapeSkew;
  ExpSmoother<double> outputGain;
  ExpSmoother<double> dryGain;
  ExpSmoother<double> wetGain;
  ExpSmoother<double> feedback;
  ExpSmoother<double> delayTimeSamples;
  ExpSmoother<double> shiftPitch;
  ExpSmoother<double> shiftFreq;
  ExpSmoother<double> lfoToPrimaryDelayTime;
  ExpSmoother<double> lfoToPrimaryShiftPitch;
  ExpSmoother<double> lfoToPrimaryShiftHz;

  size_t oversampling = 2;

  LinearTempoSynchronizer<double, 32768> synchronizer;

  std::array<double, 2> feedbackBuffer{};
  std::array<CubicUpSampler<double, maxUpFold>, 2> upSampler;
  std::array<SVF<double>, 2> feedbackHighpass;
  std::array<SVF<double>, 2> feedbackLowpass;
  std::array<AMFrequencyShifter<double>, 2> frequencyShifter;
  std::array<PitchShiftDelay<double>, 2> pitchShifter;
  std::array<DecimationLowpass<double, Sos8FoldFirstStage<double>>, 2> decimationLowpass;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
