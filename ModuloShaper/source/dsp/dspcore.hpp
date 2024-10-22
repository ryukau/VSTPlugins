// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lightlimiter.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "moduloshaper.hpp"

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

  std::array<ModuloShaper<float>, 2> shaperNaive;
  std::array<ModuloShaperPolyBLEP<double>, 2> shaperBlep;
  std::array<Butter8Lowpass<float>, 2> lowpass;
  std::array<LightLimiter<float, 64>, 2> limiter;

  size_t shaperType = 0; /* 0: naive, 1: oversample, 2: P-BLEP4, 3: P-BLEP8 */
  bool activateLowpass = true;
  bool activateLimiter = true;
  ExpSmoother<float> interpInputGain;
  ExpSmoother<float> interpClipGain;
  ExpSmoother<float> interpOutputGain;
  ExpSmoother<float> interpAdd;
  ExpSmoother<float> interpMul;
  ExpSmoother<float> interpCutoff;
};
