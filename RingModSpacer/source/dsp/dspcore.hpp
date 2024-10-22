// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/basiclimiter.hpp"
#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  DSPCore() {}

  GlobalParameter param;
  bool isPlaying = false;

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
  std::array<double, 2> processFrame(const std::array<double, 4> &frame);

  double sampleRate = 44100;

  ExpSmoother<double> outputGain;
  ExpSmoother<double> sideMix;
  ExpSmoother<double> ringSubtractMix;
  ExpSmoother<double> inputGain;
  ExpSmoother<double> sideGain;

  std::array<IntDelay<double>, 4> delay;
  std::array<BasicLimiter<double>, 4> limiter;
};
