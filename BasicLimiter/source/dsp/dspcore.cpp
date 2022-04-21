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

#include "dspcore.hpp"

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
  this->sampleRate = float(sampleRate);

  for (auto &lm : limiter) lm.resize(size_t(maxAttackSeconds * this->sampleRate) + 1);
  for (auto &dly : latencyDelay) dly.setFrames(SOCPFIR<float>::intDelay);

  reset();
  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &lm : limiter) lm.reset();
  for (auto &fd : fracDelay) fd.reset();
  for (auto &dly : latencyDelay) dly.reset();
  startup();
}

void DSPCORE_NAME::startup() {}

uint32_t DSPCORE_NAME::getLatency()
{
  uint32_t latency = limiter[0].latency();

  if (param.value[ParameterID::truePeak]->getInt())
    latency += uint32_t(SOCPFIR<float>::intDelay);

  return latency;
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  for (auto &lm : limiter) {
    lm.prepare(
      sampleRate, pv[ID::limiterAttack]->getFloat(), pv[ID::limiterSustain]->getFloat(),
      pv[ID::limiterRelease]->getFloat(), pv[ID::limiterThreshold]->getFloat(),
      pv[ID::limiterGate]->getFloat());
  }
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  if (param.value[ParameterID::truePeak]->getInt()) {
    for (size_t i = 0; i < length; ++i) {
      const auto tp0 = fracDelay[0].process(in0[i]);
      const auto tp1 = fracDelay[1].process(in1[i]);
      const auto inAbs = std::max(tp0, tp1);
      out0[i] = limiter[0].process(latencyDelay[0].process(in0[i]), inAbs);
      out1[i] = limiter[1].process(latencyDelay[1].process(in1[i]), inAbs);
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      const auto inAbs = std::max(std::fabs(in0[i]), std::fabs(in1[i]));
      out0[i] = limiter[0].process(in0[i], inAbs);
      out1[i] = limiter[1].process(in1[i], inAbs);
    }
  }
}
