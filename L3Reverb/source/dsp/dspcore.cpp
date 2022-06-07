// (c) 2020 Takamitsu Endo
//
// This file is part of L3Reverb.
//
// L3Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// L3Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with L3Reverb.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#ifdef USE_VECTORCLASS
  #if INSTRSET >= 10
    #define DSPCORE_NAME DSPCore_AVX512
  #elif INSTRSET >= 8
    #define DSPCORE_NAME DSPCore_AVX2
  #elif INSTRSET >= 7
    #define DSPCORE_NAME DSPCore_AVX
  #else
    #error Unsupported instruction set
  #endif
#else
  #define DSPCORE_NAME DSPCore_Plain
#endif

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  for (auto &dly : delay) dly.setup(this->sampleRate, float(Scales::time.getMax()));

  reset();
}

inline std::array<float, 2> calcOffset(float offset, float mul)
{
  if (offset >= 0) return {mul, (1.0f - offset) * mul};
  return {(1.0f + offset) * mul, mul};
}

#define ASSIGN_ALLPASS_PARAMETER(METHOD)                                                 \
  auto timeMul = param.value[ID::timeMultiply]->getFloat();                              \
  auto innerMul = param.value[ID::innerFeedMultiply]->getFloat();                        \
  auto d1FeedMul = param.value[ID::d1FeedMultiply]->getFloat();                          \
  auto d2FeedMul = param.value[ID::d2FeedMultiply]->getFloat();                          \
  auto d3FeedMul = param.value[ID::d3FeedMultiply]->getFloat();                          \
  auto d4FeedMul = param.value[ID::d4FeedMultiply]->getFloat();                          \
                                                                                         \
  auto timeOfs = param.value[ID::timeOffsetRange]->getFloat();                           \
  auto innerOfs = param.value[ID::innerFeedOffsetRange]->getFloat();                     \
  auto d1FeedOfs = param.value[ID::d1FeedOffsetRange]->getFloat();                       \
  auto d2FeedOfs = param.value[ID::d2FeedOffsetRange]->getFloat();                       \
  auto d3FeedOfs = param.value[ID::d3FeedOffsetRange]->getFloat();                       \
  auto d4FeedOfs = param.value[ID::d4FeedOffsetRange]->getFloat();                       \
                                                                                         \
  std::uniform_real_distribution<float> timeOffsetDist(-timeOfs, timeOfs);               \
  std::uniform_real_distribution<float> innerOffsetDist(-innerOfs, innerOfs);            \
  std::uniform_real_distribution<float> d1FeedOffsetDist(-d1FeedOfs, d1FeedOfs);         \
  std::uniform_real_distribution<float> d2FeedOffsetDist(-d2FeedOfs, d2FeedOfs);         \
  std::uniform_real_distribution<float> d3FeedOffsetDist(-d3FeedOfs, d3FeedOfs);         \
  std::uniform_real_distribution<float> d4FeedOffsetDist(-d4FeedOfs, d4FeedOfs);         \
                                                                                         \
  uint16_t i1 = 0;                                                                       \
  uint16_t i2 = 0;                                                                       \
  uint16_t i3 = 0;                                                                       \
  uint16_t i4 = 0;                                                                       \
                                                                                         \
  auto &ap4L = delay[0];                                                                 \
  auto &ap4R = delay[1];                                                                 \
  for (uint8_t d4 = 0; d4 < nSection4; ++d4) {                                           \
    auto &ap3L = ap4L.allpass[d4];                                                       \
    auto &ap3R = ap4R.allpass[d4];                                                       \
    for (uint8_t d3 = 0; d3 < nSection3; ++d3) {                                         \
      auto &ap2L = ap3L.allpass[d3];                                                     \
      auto &ap2R = ap3R.allpass[d3];                                                     \
      for (uint8_t d2 = 0; d2 < nSection2; ++d2) {                                       \
        auto &ap1L = ap2L.allpass[d2];                                                   \
        auto &ap1R = ap2R.allpass[d2];                                                   \
        for (uint8_t d1 = 0; d1 < nSection1; ++d1) {                                     \
          auto d1TimeOffset = calcOffset(timeOffsetDist(timeRng), timeMul);              \
          auto innerFeedOffset = calcOffset(innerOffsetDist(innerRng), innerMul);        \
          auto d1FeedOffset = calcOffset(d1FeedOffsetDist(d1FeedRng), d1FeedMul);        \
                                                                                         \
          ap1L.seconds[d1].METHOD(                                                       \
            param.value[ID::time0 + i1]->getFloat() * d1TimeOffset[0]);                  \
          ap1L.innerFeed[d1].METHOD(                                                     \
            param.value[ID::innerFeed0 + i1]->getFloat() * innerFeedOffset[0]);          \
          ap1L.outerFeed[d1].METHOD(                                                     \
            param.value[ID::d1Feed0 + i1]->getFloat() * d1FeedOffset[0]);                \
                                                                                         \
          ap1R.seconds[d1].METHOD(                                                       \
            param.value[ID::time0 + i1]->getFloat() * d1TimeOffset[1]);                  \
          ap1R.innerFeed[d1].METHOD(                                                     \
            param.value[ID::innerFeed0 + i1]->getFloat() * innerFeedOffset[1]);          \
          ap1R.outerFeed[d1].METHOD(                                                     \
            param.value[ID::d1Feed0 + i1]->getFloat() * d1FeedOffset[1]);                \
                                                                                         \
          ++i1;                                                                          \
        }                                                                                \
                                                                                         \
        auto offsetD2Feed = calcOffset(d2FeedOffsetDist(d2FeedRng), d2FeedMul);          \
        ap2L.feed[d2].METHOD(                                                            \
          param.value[ID::d2Feed0 + i2]->getFloat() * offsetD2Feed[0]);                  \
        ap2R.feed[d2].METHOD(                                                            \
          param.value[ID::d2Feed0 + i2]->getFloat() * offsetD2Feed[1]);                  \
        ++i2;                                                                            \
      }                                                                                  \
                                                                                         \
      auto offsetD3Feed = calcOffset(d3FeedOffsetDist(d3FeedRng), d3FeedMul);            \
                                                                                         \
      ap3L.feed[d3].METHOD(param.value[ID::d3Feed0 + i3]->getFloat() * offsetD3Feed[0]); \
      ap3R.feed[d3].METHOD(param.value[ID::d3Feed0 + i3]->getFloat() * offsetD3Feed[1]); \
      ++i3;                                                                              \
    }                                                                                    \
                                                                                         \
    auto offsetD4Feed = calcOffset(d4FeedOffsetDist(d4FeedRng), d4FeedMul);              \
                                                                                         \
    ap4L.feed[d4].METHOD(param.value[ID::d4Feed0 + i4]->getFloat() * offsetD4Feed[0]);   \
    ap4R.feed[d4].METHOD(param.value[ID::d4Feed0 + i4]->getFloat() * offsetD4Feed[1]);   \
    ++i4;                                                                                \
  }                                                                                      \
                                                                                         \
  interpStereoCross.METHOD(param.value[ID::stereoCross]->getFloat());                    \
  interpStereoSpread.METHOD(param.value[ID::stereoSpread]->getFloat());                  \
  interpDry.METHOD(param.value[ID::dry]->getFloat());                                    \
  interpWet.METHOD(param.value[ID::wet]->getFloat());

void DSPCORE_NAME::reset()
{
  using ID = ParameterID::ID;

  startup();

  for (auto &dly : delay) dly.reset();
  delayOut.fill(0);

  ASSIGN_ALLPASS_PARAMETER(reset);
}

void DSPCORE_NAME::startup()
{
  refreshSeed();

  timeRng.seed(timeSeed);
  innerRng.seed(innerSeed);
  d1FeedRng.seed(d1FeedSeed);
  d2FeedRng.seed(d2FeedSeed);
  d3FeedRng.seed(d3FeedSeed);
  d4FeedRng.seed(d4FeedSeed);
}

void DSPCORE_NAME::setParameters()
{
  using ID = ParameterID::ID;

  SmootherCommon<float>::setTime(param.value[ID::smoothness]->getFloat());

  refreshSeed();

  if (!param.value[ID::timeModulation]->getInt()) timeRng.seed(timeSeed);
  if (!param.value[ID::innerFeedModulation]->getInt()) innerRng.seed(innerSeed);
  if (!param.value[ID::d1FeedModulation]->getInt()) d1FeedRng.seed(d1FeedSeed);
  if (!param.value[ID::d2FeedModulation]->getInt()) d2FeedRng.seed(d2FeedSeed);
  if (!param.value[ID::d3FeedModulation]->getInt()) d3FeedRng.seed(d3FeedSeed);
  if (!param.value[ID::d4FeedModulation]->getInt()) d4FeedRng.seed(d4FeedSeed);

  ASSIGN_ALLPASS_PARAMETER(push);
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(float(length));

  for (size_t i = 0; i < length; ++i) {
    const auto cross = interpStereoCross.process();
    const auto delayOut0 = delayOut[0];
    const auto delayOut1 = delayOut[1];
    delayOut[0] = delay[0].process(in0[i] + cross * delayOut1, sampleRate);
    delayOut[1] = delay[1].process(in1[i] + cross * delayOut0, sampleRate);
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

void DSPCORE_NAME::refreshSeed()
{
  std::minstd_rand rng{param.value[ParameterID::seed]->getInt()};
  std::uniform_int_distribution<uint_fast32_t> dist(0, UINT32_MAX);

  timeSeed = dist(rng);
  innerSeed = dist(rng);
  d1FeedSeed = dist(rng);
  d2FeedSeed = dist(rng);
  d3FeedSeed = dist(rng);
  d4FeedSeed = dist(rng);
}
