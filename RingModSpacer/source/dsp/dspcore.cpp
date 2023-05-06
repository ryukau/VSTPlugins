// (c) 2023 Takamitsu Endo
//
// This file is part of RingModSpacer.
//
// RingModSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RingModSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RingModSpacer.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <complex>
#include <limits>
#include <numeric>

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T freqToSvfG(T normalizedFreq)
{
  static constexpr T minCutoff = T(0.00001);
  static constexpr T nyquist = T(0.49998);

  return std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * T(pi));
}

template<typename T> inline void syncPhase(T &phase0, T &phase1, T kp)
{
  auto d1 = phase1 - phase0;
  if (d1 < 0) {
    auto d2 = d1 + double(1);
    phase0 += kp * (d2 < -d1 ? d2 : d1);
  } else {
    auto d2 = d1 - double(1);
    phase0 += kp * (-d2 < d1 ? d2 : d1);
  }
}

template<typename T> inline T crossPhase(T phase0, T phase1, T ratio)
{
  auto d1 = phase1 - phase0;
  if (d1 < 0) {
    auto d2 = d1 + double(1);
    return phase0 + ratio * (d2 < -d1 ? d2 : d1);
  }
  auto d2 = d1 - double(1);
  return phase0 + ratio * (-d2 < d1 ? d2 : d1);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  SmootherCommon<double>::setSampleRate(sampleRate);

  size_t bufferSize = size_t(sampleRate * maxLimiterAttackSeconds) + 1;
  for (auto &x : delay) x.resize(bufferSize);
  for (auto &x : limiter) x.resize(bufferSize);

  reset();
  startup();
}

size_t DSPCore::getLatency()
{
  return std::max(limiter[0].latency(1), limiter[2].latency(1));
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  SmootherCommon<double>::setTime(pv[ID::parameterSmoothingSecond]->getDouble());        \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  sideMix.METHOD(pv[ID::sideMix]->getDouble());                                          \
  ringSubtractMix.METHOD(pv[ID::ringSubtractMix]->getDouble());                          \
  inputGain.METHOD(pv[ID::inputGain]->getDouble());                                      \
  sideGain.METHOD(pv[ID::sideGain]->getDouble());                                        \
                                                                                         \
  auto inputLimiterAttack = pv[ID::inputLimiterAttackSeconds]->getDouble();              \
  auto inputLimiterRelease = pv[ID::inputLimiterReleaseSeconds]->getDouble();            \
  limiter[0].prepare(sampleRate, inputLimiterAttack, inputLimiterRelease, double(1));    \
  limiter[1].prepare(sampleRate, inputLimiterAttack, inputLimiterRelease, double(1));    \
                                                                                         \
  auto sideLimiterAttack = pv[ID::sideLimiterAttackSeconds]->getDouble();                \
  auto sideLimiterRelease = pv[ID::sideLimiterReleaseSeconds]->getDouble();              \
  limiter[2].prepare(sampleRate, sideLimiterAttack, sideLimiterRelease, double(1));      \
  limiter[3].prepare(sampleRate, sideLimiterAttack, sideLimiterRelease, double(1));      \
                                                                                         \
  auto inputLatency = limiter[0].latency(1);                                             \
  auto sideLatency = limiter[2].latency(1);                                              \
  if (inputLatency >= sideLatency) {                                                     \
    delay[0].setFrames(0);                                                               \
    delay[1].setFrames(0);                                                               \
    delay[2].setFrames(inputLatency - sideLatency);                                      \
    delay[3].setFrames(inputLatency - sideLatency);                                      \
  } else {                                                                               \
    delay[0].setFrames(sideLatency - inputLatency);                                      \
    delay[1].setFrames(sideLatency - inputLatency);                                      \
    delay[2].setFrames(0);                                                               \
    delay[3].setFrames(0);                                                               \
  }

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  for (auto &x : delay) x.reset();
  for (auto &x : limiter) x.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

std::array<double, 2> DSPCore::processFrame(const std::array<double, 4> &frame)
{
  outputGain.process();
  sideMix.process();
  ringSubtractMix.process();
  inputGain.process();
  sideGain.process();

  auto in0 = frame[0] * inputGain.getValue();
  auto in1 = frame[1] * inputGain.getValue();
  auto side0 = frame[2] * sideGain.getValue();
  auto side1 = frame[3] * sideGain.getValue();
  in0 = delay[0].process(limiter[0].process(in0));
  in1 = delay[1].process(limiter[1].process(in1));
  side0 = delay[2].process(limiter[2].process(side0));
  side1 = delay[3].process(limiter[3].process(side1));

  auto ring0 = in0 * (double(1) - std::abs(side0));
  auto ring1 = in1 * (double(1) - std::abs(side1));

  auto sub0 = std::copysign(std::max(double(0), std::abs(in0) - std::abs(side0)), in0);
  auto sub1 = std::copysign(std::max(double(0), std::abs(in1) - std::abs(side1)), in1);

  auto sig0 = lerp(ring0, sub0, ringSubtractMix.getValue()) + side0 * sideMix.getValue();
  auto sig1 = lerp(ring1, sub1, ringSubtractMix.getValue()) + side1 * sideMix.getValue();

  return {outputGain.process() * sig0, outputGain.process() * sig1};
}

void DSPCore::process(
  const size_t length,
  const float *in0,
  const float *in1,
  const float *in2,
  const float *in3,
  float *out0,
  float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    auto frame = processFrame({in0[i], in1[i], in2[i], in3[i]});
    out0[i] = frame[0];
    out1[i] = frame[1];
  }
}
