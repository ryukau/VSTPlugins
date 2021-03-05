// (c) 2020 Takamitsu Endo
//
// This file is part of L4Reverb.
//
// L4Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// L4Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with L4Reverb.  If not, see <https://www.gnu.org/licenses/>.

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
    void refreshSeed();                                                                  \
                                                                                         \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    std::minstd_rand timeRng{0};                                                         \
    std::minstd_rand innerRng{0};                                                        \
    std::minstd_rand d1FeedRng{0};                                                       \
    std::minstd_rand d2FeedRng{0};                                                       \
    std::minstd_rand d3FeedRng{0};                                                       \
    std::minstd_rand d4FeedRng{0};                                                       \
    uint_fast32_t timeSeed = 0;                                                          \
    uint_fast32_t innerSeed = 0;                                                         \
    uint_fast32_t d1FeedSeed = 0;                                                        \
    uint_fast32_t d2FeedSeed = 0;                                                        \
    uint_fast32_t d3FeedSeed = 0;                                                        \
    uint_fast32_t d4FeedSeed = 0;                                                        \
                                                                                         \
    std::array<NestD4<float, 4>, 2> delay;                                               \
    std::array<float, 2> delayOut{};                                                     \
    ExpSmoother<float> interpStereoCross;                                                \
    ExpSmoother<float> interpStereoSpread;                                               \
    ExpSmoother<float> interpDry;                                                        \
    ExpSmoother<float> interpWet;                                                        \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
