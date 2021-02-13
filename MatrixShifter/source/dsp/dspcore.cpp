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

#include "dspcore.hpp"

#include "../../../lib/vcl/vectormath_exp.h"

#include <algorithm>
#include <numeric>

#if INSTRSET >= 10
#define DSPCORE_NAME DSPCore_AVX512
#elif INSTRSET >= 8
#define DSPCORE_NAME DSPCore_AVX2
#elif INSTRSET >= 7
#define DSPCORE_NAME DSPCore_AVX
#else
#error Unsupported instruction set
#endif

inline std::array<float, 2> calcPhaseOffset(float offset)
{
  if (offset < 0) return {-offset, 0};
  return {0, offset};
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.04f);

  for (auto &shf : shifter) shf.setup(sampleRate, maxShiftDelaySeconds);

  reset();
}

// template<typename Sample>

#define ASSIGN_PARAMETER(METHOD)                                                         \
  intperGain.METHOD(param.value[ID::gain]->getFloat());                                  \
                                                                                         \
  auto shiftMix = param.value[ID::shiftMix]->getFloat();                                 \
  auto bypassMix = 1 - shiftMix;                                                         \
  intperShiftPhase.METHOD(param.value[ID::shiftPhase]->getFloat());                      \
  intperShiftFeedback.METHOD(param.value[ID::shiftFeedback]->getFloat());                \
                                                                                         \
  interpLfoHz.METHOD(param.value[ID::lfoHz]->getFloat());                                \
  interpLfoAmount.METHOD(param.value[ID::lfoAmount]->getFloat());                        \
  interpLfoSkew.METHOD(param.value[ID::lfoSkew]->getFloat());                            \
  interpLfoShiftOffset.METHOD(param.value[ID::lfoShiftOffset]->getFloat());              \
                                                                                         \
  auto shiftMul = param.value[ID::shiftSemiMultiplier]->getFloat();                      \
  for (size_t x = 0; x < nSerial; ++x) {                                                 \
    interpShiftDelay[x].METHOD(param.value[ID::shiftDelay0 + x]->getFloat());            \
    interpShiftGain[x].METHOD(shiftMix *param.value[ID::shiftGain0 + x]->getFloat());    \
    for (size_t y = 0; y < nParallel; ++y) {                                             \
      interpShiftHz[x][y].METHOD(                                                        \
        shiftMul *param.value[ID::shiftSemi0 + nParallel * x + y]->getFloat());          \
    }                                                                                    \
  }                                                                                      \
  interpShiftGain.back().METHOD(bypassMix);                                              \
                                                                                         \
  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

void DSPCORE_NAME::reset()
{
  using ID = ParameterID::ID;

  startup();
  for (auto &shf : shifter) shf.reset();

  ASSIGN_PARAMETER(reset);
}

void DSPCORE_NAME::startup()
{
  for (auto &osc : lfo) osc.reset();
}

void DSPCORE_NAME::setParameters(float tempo)
{
  using ID = ParameterID::ID;

  ASSIGN_PARAMETER(push);
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> lfoAmt{};
  std::array<float, 2> lfoHz{};
  for (size_t i = 0; i < length; ++i) {
    const auto phaseOffset = calcPhaseOffset(intperShiftPhase.process());

    const auto lfoShiftOffset = interpLfoShiftOffset.process();
    if (lfoShiftOffset >= 0.0) {
      lfoHz[0] = 1.0f + lfoShiftOffset * (lfoAmt[0] - 1.0f);
      lfoHz[1] = 1.0f + lfoShiftOffset * (lfoAmt[1] - 1.0f);
    } else {
      lfoHz[0] = 1.0f - lfoShiftOffset * (lfoAmt[1] - 1.0f);
      lfoHz[1] = 1.0f - lfoShiftOffset * (lfoAmt[0] - 1.0f);
    }

    for (size_t j = 0; j < lfo.size(); ++j) {
      const float lfoOut = lfo[j].process(
        sampleRate, interpLfoHz.process(), interpLfoSkew.process(), phaseOffset[j]);
      lfoAmt[j] = 1.0f + interpLfoAmount.process() * (lfoOut - 1.0f);
    }

    for (size_t x = 0; x < nSerial; ++x) {
      auto delay = interpShiftDelay[x].process();
      shifter[0].seconds[x] = lfoAmt[0] * delay;
      shifter[1].seconds[x] = lfoAmt[1] * delay;

      auto gain = interpShiftGain[x].process();
      shifter[0].gain[x] = gain;
      shifter[1].gain[x] = gain;

      for (size_t y = 0; y < nParallel; ++y) {
        auto hz = interpShiftHz[x][y].process();
        shifter[0].hz[x][y] = lfoHz[0] * hz;
        shifter[1].hz[x][y] = lfoHz[1] * hz;
      }
    }
    auto bypassGain = interpShiftGain.back().process();
    shifter[0].gain.back() = bypassGain;
    shifter[1].gain.back() = bypassGain;

    const auto gain = intperGain.process();
    const auto feedback = intperShiftFeedback.process();
    out0[i] = gain * shifter[0].process(sampleRate, in0[i], phaseOffset[0], feedback);
    out1[i] = gain * shifter[1].process(sampleRate, in1[i], phaseOffset[1], feedback);
  }
}
