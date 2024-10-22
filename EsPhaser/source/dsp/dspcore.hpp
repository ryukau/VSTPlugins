// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "phaser.hpp"

#include <array>
#include <cmath>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPInterface {
public:
  virtual ~DSPInterface() {};

  static const size_t maxVoice = 32;
  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters() = 0;
  virtual void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1)
    = 0;
};

#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    void setParameters() override;                                                       \
    void process(                                                                        \
      const size_t length,                                                               \
      const float *in0,                                                                  \
      const float *in1,                                                                  \
      float *out0,                                                                       \
      float *out1) override;                                                             \
                                                                                         \
  private:                                                                               \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    std::array<Thiran2Phaser, 2> phaser;                                                 \
                                                                                         \
    LinearSmoother<float> interpMix;                                                     \
    LinearSmoother<float> interpFrequency;                                               \
    LinearSmoother<float> interpFreqSpread;                                              \
    LinearSmoother<float> interpFeedback;                                                \
    LinearSmoother<float> interpRange;                                                   \
    LinearSmoother<float> interpMin;                                                     \
    RotarySmoother<float> interpPhase;                                                   \
    LinearSmoother<float> interpStereoOffset;                                            \
    LinearSmoother<float> interpCascadeOffset;                                           \
  };

DSPCORE_CLASS(FixedInstruction)
