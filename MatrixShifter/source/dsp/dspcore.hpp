// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "lfo.hpp"
#include "shifter.hpp"

#include <array>
#include <cmath>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  float beatsElapsed = 0.0f;

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

private:
  float getTempoSyncInterval();

  float sampleRate = 44100.0f;

  // Temporary variables.
  std::array<float, 2> lfoOut{};
  std::array<float, 2> lfoDelay{};
  std::array<float, 2> feedbackCutoffHz{};
  std::array<float, 2> lfoHz{};

  ExpSmoother<float> interpGain;
  ExpSmoother<float> interpShiftFeedbackGain;
  ExpSmoother<float> interpShiftFeedbackCutoff;
  ExpSmoother<float> interpSectionGain;
  ExpSmoother<float> interpLfoLrPhaseOffset;
  ExpSmoother<float> interpLfoToDelay;
  ExpSmoother<float> interpLfoSkew;
  ExpSmoother<float> interpLfoToPitchShift;
  ExpSmoother<float> interpLfoToFeedbackCutoff;
  std::array<std::array<ExpSmoother<float>, nParallel>, nSerial> interpShiftHz;
  std::array<ExpSmoother<float>, nSerial> interpShiftDelay;
  std::array<ExpSmoother<float>, nSerial + 1> interpShiftGain;

  TempoSynchronizer<float> syncer;
  std::array<LFO<float>, 2> lfo;
  std::array<MultiShifter<float, nParallel, nSerial>, 2> shifter;
};
