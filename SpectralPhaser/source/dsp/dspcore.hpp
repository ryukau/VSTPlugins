// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "spectralfilter.hpp"

#include <limits>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  using Sample = float;

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
    const size_t length,
    const float *in0,
    const float *in1,
    const float *side0,
    const float *side1,
    float *out0,
    float *out1);

private:
  static constexpr size_t tableSize = size_t(1) << maxFrameSizeLog2;

  std::array<Sample, 2>
  processFrame(int frameIndex, Sample in0, Sample in1, Sample side0, Sample side1);
  Sample getTempoSyncFrequency();

  Sample sampleRate = 44100;
  TransformType previousTransform = TransformType::fft;
  LfoWaveform lfoWaveform = LfoWaveform::sine;

  ExpSmoother<Sample> lfoWaveMod;
  ExpSmoother<Sample> lfoRate;
  RotarySmoother<Sample> lfoStereoPhaseOffset;
  RotarySmoother<Sample> lfoInitialPhase;
  ExpSmoother<Sample> feedback;
  ExpSmoother<Sample> spectralShift;
  ExpSmoother<Sample> octaveDown;
  ExpSmoother<Sample> maskMix;
  ExpSmoother<Sample> maskPhase;
  RotarySmoother<Sample> maskFreq;
  ExpSmoother<Sample> maskThreshold;
  ExpSmoother<Sample> maskRotation;
  ExpSmoother<Sample> lfoToMaskMix;
  ExpSmoother<Sample> lfoToMaskPhase;
  ExpSmoother<Sample> lfoToMaskFreq;
  ExpSmoother<Sample> lfoToMaskThreshold;
  ExpSmoother<Sample> lfoToMaskRotation;
  ExpSmoother<Sample> lfoToSpectralShift;
  ExpSmoother<Sample> lfoToOctaveDown;
  ExpSmoother<Sample> outputGain;
  ExpSmoother<Sample> dryWetMix;

  Sample lfoTargetFreq = 0;
  Sample lfoSyncRate = Sample(0.0078125);
  LfoPhase<Sample> lfo;
  SpectralParameter spcParam;
  std::array<SpectralProcessor<maxFrameSizeLog2>, 2> spc;
};
