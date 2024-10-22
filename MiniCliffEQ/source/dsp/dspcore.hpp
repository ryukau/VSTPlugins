// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lightlimiter.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "fftconvolver.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr size_t firLengthInPow2 = 15;
constexpr size_t blockSizeInPow2 = 11;
constexpr size_t nBlock = size_t(1) << (firLengthInPow2 - blockSizeInPow2);
constexpr size_t fftconvLatency = (size_t(1) << firLengthInPow2) / 2 - 1;

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
  bool prepareRefresh = false;
  bool isFirRefreshed = false;

  ExpSmoother<float> interpHighpassGain;
  ExpSmoother<float> interpLowpassGain;

  std::array<SplitConvolver<nBlock, blockSizeInPow2>, 2> convolver;
  std::array<FixedIntDelay<float, fftconvLatency>, 2> delay;
};
