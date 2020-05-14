// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"

#include "delay.hpp"

#include <array>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  static const size_t maxVoice = 32;
  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters(float tempo) = 0;
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
    std::minstd_rand rng{0};                                                             \
    std::array<std::array<PController<float>, nestingDepth>, 2> lowpassLfoTime;          \
                                                                                         \
    StereoLongAllpass<float, nestingDepth> delay;                                        \
    std::array<std::array<ExpSmoother<float>, nestingDepth>, 2> interpTime;              \
    std::array<std::array<ExpSmoother<float>, nestingDepth>, 2> interpOuterFeed;         \
    std::array<std::array<ExpSmoother<float>, nestingDepth>, 2> interpInnerFeed;         \
    std::array<ExpSmoother<float>, nestingDepth> interpLowpassCutoff;                    \
    ExpSmoother<float> interpStereoCross;                                                \
    ExpSmoother<float> interpStereoSpread;                                               \
    ExpSmoother<float> interpDry;                                                        \
    ExpSmoother<float> interpWet;                                                        \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
