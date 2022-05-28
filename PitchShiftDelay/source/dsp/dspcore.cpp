// (c) 2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

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

  SmootherCommon<float>::setSampleRate(this->sampleRate * OverSampler::fold);
  SmootherCommon<float>::setTime(0.2f);

  size_t bufferSize = size_t(sampleRate * maxDelayTime) * OverSampler::fold + 1;
  for (auto &shf : shifterMain) shf.setup(bufferSize);
  for (auto &shf : shifterUnison) shf.setup(bufferSize);

  reset();
  startup();
}

size_t DSPCORE_NAME::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  auto pitchMain = pv[ID::pitch]->getFloat();                                            \
  auto pitchUnison = pitchMain;                                                          \
  if (pv[ID::inverseUnisonPitch]->getInt()) {                                            \
    pitchUnison = pitchUnison > 0 ? float(1) / pitchUnison : float(1000);                \
  }                                                                                      \
  interpPitchMain.METHOD(                                                                \
    pitchMain *(pv[ID::shifterMainReverse]->getInt() ? float(-1) : float(1)));           \
  interpPitchUnison.METHOD(                                                              \
    pitchUnison *(pv[ID::shifterUnisonReverse]->getInt() ? float(-1) : float(1)));       \
                                                                                         \
  interpDelayTime.METHOD(                                                                \
    pv[ID::delayTime]->getFloat() * sampleRate * OverSampler::fold);                     \
  interpFeedback.METHOD(pv[ID::feedback]->getFloat());                                   \
  interpCross.METHOD(pv[ID::cross]->getFloat());                                         \
  interpUnisonMix.METHOD(pv[ID::unisonMix]->getFloat());                                 \
  interpDry.METHOD(pv[ID::dry]->getFloat());                                             \
  interpWet.METHOD(pv[ID::wet]->getFloat());

void DSPCORE_NAME::reset()
{
  ASSIGN_PARAMETER(reset);

  shifterMainOut.fill({});
  shifterUnisonOut.fill({});
  for (auto &os : overSampler) os.reset();
  for (auto &shf : shifterMain) shf.reset();
  for (auto &shf : shifterUnison) shf.reset();

  startup();
}

void DSPCORE_NAME::startup() {}

void DSPCORE_NAME::setParameters() { ASSIGN_PARAMETER(push); }

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(float(length));

  for (size_t i = 0; i < length; ++i) {
    overSampler[0].push(in0[i]);
    overSampler[1].push(in1[i]);
    for (size_t idx = 0; idx < OverSampler::fold; ++idx) {
      auto pitchMain = interpPitchMain.process();
      auto pitchUnison = interpPitchUnison.process();
      auto delayTime = interpDelayTime.process();
      auto feedback = interpFeedback.process();
      auto cross = interpCross.process();
      auto unisonMix = interpUnisonMix.process();
      auto dry = interpDry.process();
      auto wet = interpWet.process();

      auto crossMain0 = lerp(shifterMainOut[0], shifterUnisonOut[0], cross);
      auto crossMain1 = lerp(shifterMainOut[1], shifterUnisonOut[1], cross);
      auto crossUnison0 = lerp(shifterMainOut[0], shifterUnisonOut[0], float(1) - cross);
      auto crossUnison1 = lerp(shifterMainOut[1], shifterUnisonOut[1], float(1) - cross);

      shifterMainOut[0] = shifterMain[0].process(
        overSampler[0].at(idx) + feedback * crossMain0, pitchMain, delayTime);
      shifterMainOut[1] = shifterMain[1].process(
        overSampler[1].at(idx) + feedback * crossMain1, pitchMain, delayTime);

      shifterUnisonOut[0] = shifterUnison[0].process(
        overSampler[0].at(idx) + feedback * crossUnison0, pitchUnison, delayTime);
      shifterUnisonOut[1] = shifterUnison[1].process(
        overSampler[1].at(idx) + feedback * crossUnison1, pitchUnison, delayTime);

      overSampler[0].inputBuffer[idx] = dry * overSampler[0].at(idx)
        + wet * lerp(shifterMainOut[0], shifterUnisonOut[0], unisonMix);
      overSampler[1].inputBuffer[idx] = dry * overSampler[1].at(idx)
        + wet * lerp(shifterMainOut[1], shifterUnisonOut[1], unisonMix);
    }
    out0[i] = overSampler[0].process();
    out1[i] = overSampler[1].process();
  }
}
