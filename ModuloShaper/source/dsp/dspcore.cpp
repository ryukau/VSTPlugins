// (c) 2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

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

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  SmootherCommon<float>::setSampleRate(sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &shaper : shaperNaive) shaper.reset();
  for (auto &shaper : shaperBlep) shaper.reset();
  for (auto &lp : lowpass) lp.reset();
  startup();
}

void DSPCORE_NAME::startup() {}

uint32_t DSPCORE_NAME::getLatency()
{
  if (shaperType == 2) // 4 point PolyBLEP residual.
    return 4;
  else if (shaperType == 3) // 8 point PolyBLEP residual.
    return 8;
  return 0;
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  interpInputGain.push(param.value[ID::inputGain]->getFloat());
  interpClipGain.push(param.value[ID::clipGain]->getFloat());
  interpOutputGain.push(param.value[ID::outputGain]->getFloat());
  interpAdd.push(param.value[ID::add]->getFloat() * param.value[ID::moreAdd]->getFloat());
  interpMul.push(param.value[ID::mul]->getFloat() * param.value[ID::moreMul]->getFloat());
  interpCutoff.push(param.value[ID::lowpassCutoff]->getFloat());

  shaperType = param.value[ID::type]->getInt();
  activateLowpass = param.value[ID::lowpass]->getInt();

  bool hardclip = param.value[ID::hardclip]->getInt();
  for (auto &shaper : shaperNaive) shaper.hardclip = hardclip;
  for (auto &shaper : shaperBlep) shaper.hardclip = hardclip;
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  std::array<float, 2> frame;
  for (uint32_t i = 0; i < length; ++i) {
    auto inGain = interpInputGain.process();
    auto clipGain = interpClipGain.process();
    auto outGain = interpOutputGain.process();
    auto add = interpAdd.process();
    auto mul = interpMul.process();
    auto cutoff = interpCutoff.process();

    if (mul > 1.0f) clipGain /= mul;

    if (activateLowpass) {
      lowpass[0].setCutoff(sampleRate, cutoff);
      lowpass[1].setCutoff(sampleRate, cutoff);
      frame[0] = lowpass[0].process(in0[i]);
      frame[1] = lowpass[1].process(in1[i]);
    } else {
      frame[0] = in0[i];
      frame[1] = in1[i];
    }

    switch (shaperType) {
      case 0: // Naive.
        shaperNaive[0].add = add;
        shaperNaive[1].add = add;
        shaperNaive[0].mul = mul;
        shaperNaive[1].mul = mul;

        frame[0] = clipGain * shaperNaive[0].process(inGain * frame[0]);
        frame[1] = clipGain * shaperNaive[1].process(inGain * frame[1]);
        break;

      case 1: // Naive 4x oversampling.
        shaperNaive[0].add = add;
        shaperNaive[1].add = add;
        shaperNaive[0].mul = mul;
        shaperNaive[1].mul = mul;

        frame[0] = clipGain * shaperNaive[0].process4x(inGain * frame[0]);
        frame[1] = clipGain * shaperNaive[1].process4x(inGain * frame[1]);
        break;

      case 2: // 4 point PolyBLEP residual.
        shaperBlep[0].add = add;
        shaperBlep[1].add = add;
        shaperBlep[0].mul = mul;
        shaperBlep[1].mul = mul;

        frame[0] = clipGain * shaperBlep[0].process4(inGain * frame[0]);
        frame[1] = clipGain * shaperBlep[1].process4(inGain * frame[1]);
        break;

      case 3: // 8 point PolyBLEP residual.
        shaperBlep[0].add = add;
        shaperBlep[1].add = add;
        shaperBlep[0].mul = mul;
        shaperBlep[1].mul = mul;

        frame[0] = clipGain * shaperBlep[0].process8(inGain * frame[0]);
        frame[1] = clipGain * shaperBlep[1].process8(inGain * frame[1]);
        break;
    }

    frame[0] *= outGain;
    frame[1] *= outGain;

    out0[i] = std::isfinite(frame[0]) ? std::clamp(frame[0], -128.0f, 128.0f) : 0;
    out1[i] = std::isfinite(frame[1]) ? std::clamp(frame[1], -128.0f, 128.0f) : 0;
  }
}
