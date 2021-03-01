// (c) 2021 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/decimationLowpass.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "limiter.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual uint32_t getLatency() = 0;
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
    uint32_t getLatency() override;                                                      \
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
    std::array<Limiter<float>, 2> limiter;                                               \
    std::array<TruePeakMeterFIR<float, SOCPFIR<float>>, 2> fracDelay;                    \
    std::array<IntDelay<float>, 2> latencyDelay{8, 8}; /* Compensate fracDelay. */       \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
