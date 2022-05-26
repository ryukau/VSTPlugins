// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
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

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  for (auto &cmb : comb) cmb.setup(sampleRate, maxDelayTime);

  reset();
  startup();
}

size_t DSPCORE_NAME::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  auto timeMul = pv[ID::timeMultiplier]->getFloat();                                     \
  for (size_t idx = 0; idx < nCombTaps; ++idx) {                                         \
    auto time = timeMul * sampleRate * pv[ID::delayTime0 + idx]->getFloat();             \
    comb[0].time[idx].METHOD(time);                                                      \
    comb[1].time[idx].METHOD(time);                                                      \
  }                                                                                      \
  interpCombInterpRate.METHOD(pv[ID::delayTimeInterpRate]->getFloat());                  \
  interpFeedback.METHOD(pv[ID::feedback]->getFloat());                                   \
  interpFeedbackHighpassCutoffKp.METHOD(float(EMAFilter<double>::cutoffToP(              \
    sampleRate, pv[ID::feedbackHighpassCutoffHz]->getFloat())));                         \
  interpDry.METHOD(pv[ID::dry]->getFloat());                                             \
  interpWet.METHOD(pv[ID::wet]->getFloat());

void DSPCORE_NAME::reset()
{
  ASSIGN_PARAMETER(reset);

  delayOut.fill(0);

  for (auto &cmb : comb) cmb.reset();
  for (auto &lm : feedbackLimiter) lm.reset();

  startup();
}

void DSPCORE_NAME::startup() {}

void DSPCORE_NAME::setParameters()
{
  ASSIGN_PARAMETER(push);

  for (auto &lm : feedbackLimiter) {
    lm.prepare(sampleRate, pv[ID::feedbackLimiterRelease]->getFloat(), 1.0f);
  }
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(float(length));

  for (size_t i = 0; i < length; ++i) {
    auto combInterpRate = interpCombInterpRate.process();
    auto feedback = interpFeedback.process();
    auto feedbackHighpassKp = interpFeedbackHighpassCutoffKp.process();
    auto dry = interpDry.process();
    auto wet = interpWet.process();

    auto combOut0 = comb[0].process(in0[i] + feedback * delayOut[0], combInterpRate);
    auto combOut1 = comb[1].process(in1[i] + feedback * delayOut[1], combInterpRate);

    delayOut[0] = feedbackLimiter[0].process(
      feedbackHighpass[0].process(combOut0, feedbackHighpassKp));
    delayOut[1] = feedbackLimiter[1].process(
      feedbackHighpass[1].process(combOut1, feedbackHighpassKp));

    out0[i] = dry * in0[i] + wet * delayOut[0];
    out1[i] = dry * in1[i] + wet * delayOut[1];
  }
}
