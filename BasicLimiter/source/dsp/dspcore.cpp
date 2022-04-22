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

  for (auto &lm : limiter)
    lm.resize(
      size_t(FractionalDelayFir::upfold * maxAttackSeconds * this->sampleRate) + 1);

  reset();
  startup();
}

void DSPCORE_NAME::reset()
{
  for (auto &lm : limiter) lm.reset();
  for (auto &he : highEliminator) he.reset();
  for (auto &us : upSampler) us.reset();
  for (auto &ds : downSampler) ds.reset();
  startup();
}

void DSPCORE_NAME::startup() {}

size_t DSPCORE_NAME::getLatency()
{
  bool truepeak = param.value[ParameterID::truePeak]->getInt();
  auto latency = limiter[0].latency(truepeak ? FractionalDelayFir::upfold : 1);
  if (truepeak) {
    latency += FractionalDelayFir::intDelay + DownSamplerFir::intDelay
      + highEliminator[0].delay + 1;
  }
  return latency;
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  for (auto &lm : limiter) {
    if (param.value[ParameterID::truePeak]->getInt()) {
      lm.prepare(
        FractionalDelayFir::upfold * sampleRate, pv[ID::limiterAttack]->getFloat(),
        pv[ID::limiterSustain]->getFloat(), pv[ID::limiterRelease]->getFloat(),
        pv[ID::limiterThreshold]->getFloat(), pv[ID::limiterGate]->getFloat());
    } else {
      lm.prepare(
        sampleRate, pv[ID::limiterAttack]->getFloat(), pv[ID::limiterSustain]->getFloat(),
        pv[ID::limiterRelease]->getFloat(), pv[ID::limiterThreshold]->getFloat(),
        pv[ID::limiterGate]->getFloat());
    }
  }
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  if (param.value[ParameterID::truePeak]->getInt()) {
    constexpr size_t upfold = FractionalDelayFir::upfold;
    for (size_t i = 0; i < length; ++i) {
      auto &&sig0 = highEliminator[0].process(in0[i]);
      auto &&sig1 = highEliminator[1].process(in1[i]);

      upSampler[0].process(sig0);
      upSampler[1].process(sig1);

      std::array<std::array<float, upfold>, 2> expanded;
      for (size_t j = 0; j < upfold; ++j) {
        auto tp0 = upSampler[0].output[j];
        auto tp1 = upSampler[1].output[j];
        auto inAbs = std::fmax(std::fabs(tp0), std::fabs(tp1));
        expanded[0][j] = limiter[0].process(tp0, inAbs);
        expanded[1][j] = limiter[1].process(tp1, inAbs);
      }

      out0[i] = downSampler[0].process(expanded[0]);
      out1[i] = downSampler[1].process(expanded[1]);
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      auto &&inAbs = std::fmax(std::fabs(in0[i]), std::fabs(in1[i]));
      out0[i] = limiter[0].process(in0[i], inAbs);
      out1[i] = limiter[1].process(in1[i], inAbs);
    }
  }
}
