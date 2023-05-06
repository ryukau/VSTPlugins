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

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float feedbackLimiterAttackSeconds = 64.0f / 48000.0f;

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  reset();
  startup();
  prepareRefresh = true;
}

size_t DSPCore::getLatency() { return fftconvLatency; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  interpHighpassGain.METHOD(pv[ID::highpassGain]->getFloat());                           \
  interpLowpassGain.METHOD(pv[ID::lowpassGain]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  for (auto &cnv : convolver) cnv.reset();
  for (auto &dly : delay) dly.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER(push);

  if (prepareRefresh || (!isFirRefreshed && pv[ID::refreshFir]->getInt())) {
    auto cutoffHz = pv[ID::cutoffHz]->getFloat();
    for (auto &cnv : convolver) cnv.refreshFir(sampleRate, cutoffHz, false);
  }
  isFirRefreshed = pv[ID::refreshFir]->getInt();
  prepareRefresh = false;
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  for (size_t i = 0; i < length; ++i) {
    auto lp0 = convolver[0].process(in0[i]);
    auto lp1 = convolver[1].process(in1[i]);

    auto hp0 = delay[0].process(in0[i]) - lp0;
    auto hp1 = delay[1].process(in1[i]) - lp1;

    auto hpGain = interpHighpassGain.process();
    auto lpGain = interpLowpassGain.process();

    out0[i] = lpGain * lp0 + hpGain * hp0;
    out1[i] = lpGain * lp1 + hpGain * hp1;
  }
}
