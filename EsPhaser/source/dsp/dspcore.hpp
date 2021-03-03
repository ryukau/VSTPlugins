// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "phaser.hpp"

#include "../../../lib/vcl/vectorclass.h"

#include <array>
#include <cmath>
#include <memory>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  static const size_t maxVoice = 32;
  PlugParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters(float tempo) = 0;
  virtual void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1)
    = 0;
};

/*
# About transitionBuffer
Transition happens when synth is playing all notes and user send a new note on.
transitionBuffer is used to store a release of a note to reduce pop noise.
*/
#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    void setParameters(float tempo) override;                                            \
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

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
