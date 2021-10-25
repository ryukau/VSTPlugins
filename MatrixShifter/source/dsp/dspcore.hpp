// (c) 2021 Takamitsu Endo
//
// This file is part of MatrixShifter.
//
// MatrixShifter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixShifter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixShifter.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "lfo.hpp"
#include "shifter.hpp"

#include <array>
#include <cmath>
#include <memory>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  float beatsElapsed = 0.0f;

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
    float getTempoSyncInterval();                                                        \
                                                                                         \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    /* Temporary variables. */                                                           \
    std::array<float, 2> lfoOut{};                                                       \
    std::array<float, 2> lfoDelay{};                                                     \
    std::array<float, 2> feedbackCutoffHz{};                                             \
    std::array<float, 2> lfoHz{};                                                        \
                                                                                         \
    ExpSmoother<float> interpGain;                                                       \
    ExpSmoother<float> interpShiftFeedbackGain;                                          \
    ExpSmoother<float> interpShiftFeedbackCutoff;                                        \
    ExpSmoother<float> interpSectionGain;                                                \
    ExpSmoother<float> interpLfoLrPhaseOffset;                                           \
    ExpSmoother<float> interpLfoToDelay;                                                 \
    ExpSmoother<float> interpLfoSkew;                                                    \
    ExpSmoother<float> interpLfoToPitchShift;                                            \
    ExpSmoother<float> interpLfoToFeedbackCutoff;                                        \
    std::array<std::array<ExpSmoother<float>, nParallel>, nSerial> interpShiftHz;        \
    std::array<ExpSmoother<float>, nSerial> interpShiftDelay;                            \
    std::array<ExpSmoother<float>, nSerial + 1> interpShiftGain;                         \
                                                                                         \
    TempoSynchronizer<float> syncer;                                                     \
    std::array<LFO<float>, 2> lfo;                                                       \
    std::array<MultiShifter<float, nParallel, nSerial>, 2> shifter;                      \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)
