// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#ifdef USE_VECTORCLASS
  #include "../../../lib/vcl/vectorclass.h"
#endif

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "limiter.hpp"
#include "polyphase.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

using UpSamplerFir = UpSamplerFir8Fold<float>;
using DownSamplerFir = DownSamplerFir8Fold<float>;

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
  std::array<float, 2> processStereoLink(float in0, float in1);

  float sampleRate = 44100.0f;

  ExpSmoother<float> interpStereoLink;

  std::array<Limiter<float>, 2> limiter;
  std::array<NaiveConvolver<float, HighEliminationFir<float>>, 2> highEliminator;
  std::array<FirPolyPhaseUpSampler<float, UpSamplerFir>, 2> upSampler;
  std::array<FirDownSampler<float, DownSamplerFir>, 2> downSampler;
};
