// (c) 2022 Takamitsu Endo
//
// This file is part of MiniCliffEQ.
//
// MiniCliffEQ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MiniCliffEQ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MiniCliffEQ.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#ifdef USE_VECTORCLASS
  #include "../../../lib/vcl/vectorclass.h"
#endif

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

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual size_t getLatency() = 0;
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
    size_t getLatency() override;                                                        \
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
    bool prepareRefresh = false;                                                         \
    bool isFirRefreshed = false;                                                         \
                                                                                         \
    ExpSmoother<float> interpHighpassGain;                                               \
    ExpSmoother<float> interpLowpassGain;                                                \
                                                                                         \
    std::array<SplitConvolver<nBlock, blockSizeInPow2>, 2> convolver;                    \
    std::array<FixedIntDelay<float, fftconvLatency>, 2> delay;                           \
  };

#ifdef USE_VECTORCLASS
DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
#else
DSPCORE_CLASS(Plain)
#endif
