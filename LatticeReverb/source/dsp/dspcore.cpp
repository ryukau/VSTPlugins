// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

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

  delay.setup(sampleRate, param.scale.time.getMax());

  reset();
}

inline std::array<float, 2> calcOffset(float offset, float mul)
{
  if (offset >= 0) return {1.0f, 1.0f - mul * offset};
  return {1.0f + mul * offset, 1.0f};
}

void DSPCORE_NAME::reset()
{
  using ID = ParameterID::ID;

  delay.reset();

  auto timeMul = param.value[ID::timeMultiply]->getFloat();
  auto outerMul = param.value[ID::outerFeedMultiply]->getFloat();
  auto innerMul = param.value[ID::innerFeedMultiply]->getFloat();
  auto timeOffsetMul = param.value[ID::timeOffsetMultiply]->getFloat();
  auto outerOffsetMul = param.value[ID::outerFeedOffsetMultiply]->getFloat();
  auto innerOffsetMul = param.value[ID::innerFeedOffsetMultiply]->getFloat();
  auto timeLfoLowpassKp = param.value[ID::timeLfoLowpass]->getFloat();
  for (size_t idx = 0; idx < nestingDepth; ++idx) {
    auto timeOffset
      = calcOffset(param.value[ID::timeOffset0 + idx]->getFloat(), timeOffsetMul);
    auto time = param.value[ID::time0 + idx]->getFloat();
    interpTime[0][idx].reset(timeOffset[0] * timeMul * time);
    interpTime[1][idx].reset(timeOffset[1] * timeMul * time);
    lowpassLfoTime[0][idx].reset();
    lowpassLfoTime[1][idx].reset();
    lowpassLfoTime[0][idx].kp = timeLfoLowpassKp;
    lowpassLfoTime[1][idx].kp = timeLfoLowpassKp;

    auto outerOffset
      = calcOffset(param.value[ID::outerFeedOffset0 + idx]->getFloat(), outerOffsetMul);
    auto outerFeed = param.value[ID::outerFeed0 + idx]->getFloat();
    interpOuterFeed[0][idx].reset(outerOffset[0] * outerMul * outerFeed);
    interpOuterFeed[1][idx].reset(outerOffset[1] * outerMul * outerFeed);

    auto innerOffset
      = calcOffset(param.value[ID::innerFeedOffset0 + idx]->getFloat(), innerOffsetMul);
    auto innerFeed = param.value[ID::innerFeed0 + idx]->getFloat();
    interpInnerFeed[0][idx].reset(innerOffset[0] * innerMul * innerFeed);
    interpInnerFeed[1][idx].reset(innerOffset[1] * innerMul * innerFeed);

    interpLowpassCutoff[idx].reset(param.value[ID::lowpassCutoff0 + idx]->getFloat());
  }
  interpStereoCross.reset(param.value[ID::stereoCross]->getFloat());
  interpStereoSpread.reset(param.value[ID::stereoSpread]->getFloat());
  interpDry.reset(param.value[ID::dry]->getFloat());
  interpWet.reset(param.value[ID::wet]->getFloat());
}

void DSPCORE_NAME::startup() { rng.seed(0); }

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  auto timeMul = param.value[ID::timeMultiply]->getFloat();
  auto outerMul = param.value[ID::outerFeedMultiply]->getFloat();
  auto innerMul = param.value[ID::innerFeedMultiply]->getFloat();
  auto timeOffsetMul = param.value[ID::timeOffsetMultiply]->getFloat();
  auto outerOffsetMul = param.value[ID::outerFeedOffsetMultiply]->getFloat();
  auto innerOffsetMul = param.value[ID::innerFeedOffsetMultiply]->getFloat();
  auto timeLfoLowpassKp = param.value[ID::timeLfoLowpass]->getFloat();

  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (size_t idx = 0; idx < nestingDepth; ++idx) {
    auto timeOffset
      = calcOffset(param.value[ID::timeOffset0 + idx]->getFloat(), timeOffsetMul);
    auto time = param.value[ID::time0 + idx]->getFloat();
    auto timeLfo = param.value[ID::timeLfoAmount0 + idx]->getFloat();
    lowpassLfoTime[0][idx].kp = timeLfoLowpassKp;
    lowpassLfoTime[1][idx].kp = timeLfoLowpassKp;
    interpTime[0][idx].push(std::clamp<float>(
      timeOffset[0] * timeMul * time
        + timeLfo * lowpassLfoTime[0][idx].process(dist(rng)),
      0.0f, 1.0f));
    interpTime[1][idx].push(std::clamp<float>(
      timeOffset[1] * timeMul * time
        + timeLfo * lowpassLfoTime[1][idx].process(dist(rng)),
      0.0f, 1.0f));

    auto outerOffset
      = calcOffset(param.value[ID::outerFeedOffset0 + idx]->getFloat(), outerOffsetMul);
    auto outerFeed = param.value[ID::outerFeed0 + idx]->getFloat();
    interpOuterFeed[0][idx].push(outerOffset[0] * outerMul * outerFeed);
    interpOuterFeed[1][idx].push(outerOffset[1] * outerMul * outerFeed);

    auto innerOffset
      = calcOffset(param.value[ID::innerFeedOffset0 + idx]->getFloat(), innerOffsetMul);
    auto innerFeed = param.value[ID::innerFeed0 + idx]->getFloat();
    interpInnerFeed[0][idx].push(innerOffset[0] * innerMul * innerFeed);
    interpInnerFeed[1][idx].push(innerOffset[1] * innerMul * innerFeed);

    interpLowpassCutoff[idx].push(param.value[ID::lowpassCutoff0 + idx]->getFloat());
  }
  interpStereoCross.push(param.value[ID::stereoCross]->getFloat());
  interpStereoSpread.push(param.value[ID::stereoSpread]->getFloat());
  interpDry.push(param.value[ID::dry]->getFloat());
  interpWet.push(param.value[ID::wet]->getFloat());
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  for (size_t i = 0; i < length; ++i) {
    for (size_t idx = 0; idx < nestingDepth; ++idx) {
      auto lpCut = interpLowpassCutoff[idx].process();

      delay.apL.data[idx].seconds = interpTime[0][idx].process();
      delay.apL.data[idx].outerFeed = interpOuterFeed[0][idx].process();
      delay.apL.data[idx].innerFeed = interpInnerFeed[0][idx].process();
      delay.apL.data[idx].lowpassKp = lpCut;

      delay.apR.data[idx].seconds = interpTime[1][idx].process();
      delay.apR.data[idx].outerFeed = interpOuterFeed[1][idx].process();
      delay.apR.data[idx].innerFeed = interpInnerFeed[1][idx].process();
      delay.apR.data[idx].lowpassKp = lpCut;
    }

    auto delayOut
      = delay.process(in0[i], in1[i], sampleRate, interpStereoCross.process());
    const auto mid = delayOut[0] + delayOut[1];
    const auto side = delayOut[0] - delayOut[1];

    const auto spread = interpStereoSpread.process();
    delayOut[0] = mid - spread * (mid - side);
    delayOut[1] = mid - spread * (mid + side);

    const auto dry = interpDry.process();
    const auto wet = interpWet.process();
    out0[i] = dry * in0[i] + wet * delayOut[0];
    out1[i] = dry * in1[i] + wet * delayOut[1];
  }
}
