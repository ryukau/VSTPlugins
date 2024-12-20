// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "filter.hpp"

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  DSPCore() {}

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
    const float *in2,
    const float *in3,
    float *out0,
    float *out1);

private:
  void updateUpRate();
  std::array<double, 2> processFrame(const std::array<double, 4> &frame);

  static constexpr size_t upFold = 16;
  static constexpr std::array<size_t, 3> fold{1, 2, upFold};

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  RotarySmoother<double> stereoPhaseOffset;
  ExpSmoother<double> outputGain;
  ExpSmoother<double> mix;
  ExpSmoother<double> stereoPhaseLinkKp;
  ExpSmoother<double> stereoPhaseCross;
  ExpSmoother<double> phaseWarp;
  ExpSmoother<double> inputPhaseMod;
  ExpSmoother<double> inputPreAsymmetry;
  ExpSmoother<double> inputLowpassG;
  ExpSmoother<double> inputHighpassG;
  ExpSmoother<double> inputPostAsymmetry;
  ExpSmoother<double> sidePhaseMod;
  ExpSmoother<double> sidePreAsymmetry;
  ExpSmoother<double> sideLowpassG;
  ExpSmoother<double> sideHighpassG;
  ExpSmoother<double> sidePostAsymmetry;

  size_t oversampling = 2;
  bool enableInputEnvelope = false;
  bool enableSideEnvelope = false;

  std::array<double, 2> phase{};
  std::array<SVF<double>, 4> inputLowpass{};
  std::array<SVF<double>, 4> inputHighpass{};
  std::array<EasyGate<double>, 4> inputGate{};
  std::array<EnvelopeFollower<double>, 4> inputEnvelope{};

  std::array<CubicUpSampler<double, upFold>, 4> upSampler;
  std::array<DecimationLowpass<double, Sos16FoldFirstStage<double>>, 2> decimationLowpass;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
