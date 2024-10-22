// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lightlimiter.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "oddpowshaper.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  GlobalParameter param;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

private:
  float sampleRate = 44100.0f;
  float maxGain = 0.0f;

  std::array<OddPowShaper<float>, 2> shaper;
  std::array<LightLimiter<float, 64>, 2> limiter;

  bool oversample = true;
  bool activateLimiter = true;
  ExpSmoother<float> interpDrive;
  ExpSmoother<float> interpOutputGain;
};
