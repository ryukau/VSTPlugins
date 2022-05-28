// (c) 2022 Takamitsu Endo
//
// This file is part of BasicLimiterAutoMake.
//
// BasicLimiterAutoMake is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiterAutoMake is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiterAutoMake.  If not, see <https://www.gnu.org/licenses/>.

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

inline float maxAbs(const size_t length, const float *buffer)
{
  float max = 0.0f;
  for (size_t i = 0; i < length; ++i) max = std::max(max, std::fabs(buffer[i]));
  return max;
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  auto bufferSize
    = size_t(UpSamplerFir::upfold * maxAttackSeconds * this->sampleRate) + 1;

  for (auto &lm : limiter) lm.resize(bufferSize);

  autoMakeUp.resize(bufferSize);

  reset();
  startup();
}

size_t DSPCORE_NAME::getLatency()
{
  bool truepeak = param.value[ParameterID::truePeak]->getInt();
  auto latency = limiter[0].latency(truepeak ? UpSamplerFir::upfold : 1);
  if (truepeak) {
    latency += 1 + UpSamplerFir::intDelay + DownSamplerFir::intDelay
      + HighEliminationFir<float>::delay;
  }
  return latency;
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  interpStereoLink.METHOD(pv[ID::limiterStereoLink]->getFloat());                        \
  interpThreshold.METHOD(pv[ID::limiterThreshold]->getFloat());

void DSPCORE_NAME::reset()
{
  ASSIGN_PARAMETER(reset);

  pv[ID::overshoot]->setFromFloat(1.0);

  for (auto &lm : limiter) lm.reset();
  for (auto &he : highEliminatorMain) he.reset();
  for (auto &he : highEliminatorSide) he.reset();
  for (auto &us : upSamplerMain) us.reset();
  for (auto &us : upSamplerSide) us.reset();
  for (auto &ds : downSampler) ds.reset();

  autoMakeUp.reset(
    pv[ID::autoMakeupToggle]->getInt(), pv[ID::limiterThreshold]->getFloat(),
    pv[ID::autoMakeupTargetGain]->getFloat());

  startup();
}

void DSPCORE_NAME::startup() {}

void DSPCORE_NAME::setParameters()
{
  ASSIGN_PARAMETER(push);

  auto upfold = param.value[ParameterID::truePeak]->getInt() ? UpSamplerFir::upfold : 1;
  auto limiterThreshold = pv[ID::limiterThreshold]->getFloat();
  for (auto &lm : limiter) {
    lm.prepare(
      upfold * sampleRate, pv[ID::limiterAttack]->getFloat(),
      pv[ID::limiterSustain]->getFloat(), pv[ID::limiterRelease]->getFloat(),
      pv[ID::limiterGate]->getFloat());
  }

  autoMakeUp.prepare(
    pv[ID::autoMakeupToggle]->getInt(), limiterThreshold,
    pv[ID::autoMakeupTargetGain]->getFloat(), limiter[0].getAttackFrames());
}

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

std::array<float, 2> DSPCORE_NAME::processStereoLink(float in0, float in1)
{
  auto &&stereoLink = interpStereoLink.process();
  auto &&abs0 = std::fabs(in0);
  auto &&abs1 = std::fabs(in1);
  auto &&absMax = std::max(abs0, abs1);
  return {lerp(abs0, absMax, stereoLink), lerp(abs1, absMax, stereoLink)};
}

inline void convertToMidSide(float &left, float &right)
{
  auto mid = (left + right) * 0.5f;
  auto side = (left - right) * 0.5f;
  left = mid;
  right = side;
}

inline void convertToLeftRight(float &mid, float &side)
{
  auto left = (mid + side);
  auto right = (mid - side);
  mid = left;
  side = right;
}

void DSPCORE_NAME::process(
  const size_t length,
  const float *in0,
  const float *in1,
  const float *in2,
  const float *in3,
  float *out0,
  float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));

  bool &&enableSidechain = pv[ID::sidechain]->getInt();
  const float *sidechain0 = enableSidechain ? in2 : in0;
  const float *sidechain1 = enableSidechain ? in3 : in1;

  bool &&enableMidSide = pv[ID::channelType]->getInt();

  bool enableAutoMakeUp
    = !enableSidechain && static_cast<bool>(pv[ID::autoMakeupToggle]->getInt());
  float makeUpTarget = pv[ID::autoMakeupTargetGain]->getFloat();
  if (pv[ID::truePeak]->getInt()) {
    constexpr size_t upfold = UpSamplerFir::upfold;
    for (size_t i = 0; i < length; ++i) {
      auto sig0 = in0[i];
      auto sig1 = in1[i];
      if (enableMidSide) convertToMidSide(sig0, sig1);

      sig0 = highEliminatorMain[0].process(sig0);
      sig1 = highEliminatorMain[1].process(sig1);
      upSamplerMain[0].process(sig0);
      upSamplerMain[1].process(sig1);

      auto side0 = highEliminatorSide[0].process(sidechain0[i]);
      auto side1 = highEliminatorSide[1].process(sidechain1[i]);
      if (enableMidSide) convertToMidSide(side0, side1);

      upSamplerSide[0].process(side0);
      upSamplerSide[1].process(side1);

      auto threshold = interpThreshold.process();
      for (size_t j = 0; j < upfold; ++j) {
        auto &&sc0 = upSamplerSide[0].output[j];
        auto &&sc1 = upSamplerSide[1].output[j];

        auto &&inAbs = processStereoLink(sc0, sc1);
        auto &&makeup = autoMakeUp.process(enableAutoMakeUp, threshold, makeUpTarget);

        auto &&tp0 = upSamplerMain[0].output[j];
        auto &&tp1 = upSamplerMain[1].output[j];

        expanded[0][j] = makeup * limiter[0].process(tp0, inAbs[0], threshold);
        expanded[1][j] = makeup * limiter[1].process(tp1, inAbs[1], threshold);
      }

      sig0 = downSampler[0].process(expanded[0]);
      sig1 = downSampler[1].process(expanded[1]);

      if (enableMidSide) convertToLeftRight(sig0, sig1);

      out0[i] = sig0;
      out1[i] = sig1;
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      auto sig0 = in0[i];
      auto sig1 = in1[i];
      if (enableMidSide) convertToMidSide(sig0, sig1);

      auto side0 = sidechain0[i];
      auto side1 = sidechain1[i];
      if (enableMidSide) convertToMidSide(side0, side1);

      auto threshold = interpThreshold.process();
      auto &&makeup = autoMakeUp.process(enableAutoMakeUp, threshold, makeUpTarget);
      auto &&inAbs = processStereoLink(side0, side1);
      sig0 = makeup * limiter[0].process(sig0, inAbs[0], threshold);
      sig1 = makeup * limiter[1].process(sig1, inAbs[1], threshold);

      if (enableMidSide) convertToLeftRight(sig0, sig1);

      out0[i] = sig0;
      out1[i] = sig1;
    }
  }

  auto &&maxOut = std::max(maxAbs(length, out0), maxAbs(length, out1));
  auto &paramClippingPeak = pv[ID::overshoot];
  auto &&previousPeak = paramClippingPeak->getFloat();
  if (maxOut > previousPeak) paramClippingPeak->setFromFloat(maxOut);
}
