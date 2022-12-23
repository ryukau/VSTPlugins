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

constexpr float defaultTempo = 120.0f;

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

float DSPCORE_NAME::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getFloat();
  if (lfoRate > Scales::lfoRate.getMax()) return 0.0f;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto &&upper = pv[ID::lfoTempoUpper]->getFloat() + float(1);
  auto &&lower = pv[ID::lfoTempoLower]->getFloat() + float(1);
  return float(4) * upper / lower / lfoRate;
}

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  phaseSyncCutoffKp = float(EMAFilter<double>::cutoffToP(sampleRate, 0.1));

  SmootherCommon<float>::setSampleRate(this->sampleRate * OverSampler::fold);
  SmootherCommon<float>::setTime(0.2f);

  synchronizer.reset(this->sampleRate * OverSampler::fold, defaultTempo, 1.0f);
  lfo.setup(this->sampleRate * OverSampler::fold, 0.02f * OverSampler::fold);

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
  SmootherCommon<float>::setTime(pv[ID::smoothingTime]->getFloat());                     \
                                                                                         \
  lfo.interpType = pv[ID::lfoInterpolation]->getInt();                                   \
  for (size_t idx = 0; idx < nLfoWavetable; ++idx) {                                     \
    lfo.source[idx + 1] = pv[ID::lfoWavetable0 + idx]->getFloat();                       \
  }                                                                                      \
                                                                                         \
  auto pitchMain = pv[ID::pitch]->getFloat();                                            \
  auto pitchUnison = pitchMain;                                                          \
  if (pv[ID::mirrorUnisonPitch]->getInt()) {                                             \
    pitchUnison = pitchUnison > 0 ? float(1) / pitchUnison : float(1000);                \
  }                                                                                      \
  pitchUnison += pv[ID::unisonPitchOffset]->getFloat();                                  \
  interpPitchMain.METHOD(                                                                \
    pitchMain *(pv[ID::shifterMainReverse]->getInt() ? float(-1) : float(1)));           \
  interpPitchUnison.METHOD(                                                              \
    pitchUnison *(pv[ID::shifterUnisonReverse]->getInt() ? float(-1) : float(1)));       \
                                                                                         \
  interpLfoStereoOffset.METHOD(pv[ID::lfoStereoOffset]->getFloat());                     \
  interpLfoUnisonOffset.METHOD(pv[ID::lfoUnisonOffset]->getFloat());                     \
  interpLfoToPitch.METHOD(pv[ID::lfoToPitch]->getFloat());                               \
  interpLfoToUnison.METHOD(pv[ID::lfoToUnison]->getFloat());                             \
  interpDelayTime.METHOD(                                                                \
    pv[ID::delayTime]->getFloat() * sampleRate * OverSampler::fold);                     \
  interpStereoLean.METHOD(pv[ID::stereoLean]->getFloat());                               \
  interpFeedback.METHOD(pv[ID::feedback]->getFloat());                                   \
  interpHighpassCutoffKp.METHOD(float(                                                   \
    EMAFilter<double>::cutoffToP(sampleRate, pv[ID::highpassCutoffHz]->getFloat())));    \
  interpPitchCross.METHOD(pv[ID::pitchCross]->getFloat());                               \
  interpStereoCross.METHOD(pv[ID::stereoCross]->getFloat());                             \
  interpUnisonMix.METHOD(pv[ID::unisonMix]->getFloat());                                 \
  interpDry.METHOD(pv[ID::dry]->getFloat());                                             \
  interpWet.METHOD(pv[ID::wet]->getFloat());

void DSPCORE_NAME::reset()
{
  ASSIGN_PARAMETER(reset);

  lfo.reset();

  shifterMainOut.fill({});
  shifterUnisonOut.fill({});
  for (auto &os : overSampler) os.reset();
  for (auto &shf : shifterMain) shf.reset();
  for (auto &shf : shifterUnison) shf.reset();

  startup();
}

void DSPCORE_NAME::startup()
{
  synchronizer.reset(sampleRate * OverSampler::fold, tempo, getTempoSyncInterval());
}

void DSPCORE_NAME::setParameters() { ASSIGN_PARAMETER(push); }

inline void convertToMidSide(float &left, float &right)
{
  auto mid = left + right;
  auto side = left - right;
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

inline std::array<float, 2> getStereoLean(float value, float lean)
{
  if (lean < 0) return {value * (float(1) + lean), value};
  return {value, value * (float(1) - lean)};
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    sampleRate * OverSampler::fold, isTempoSyncing ? tempo : defaultTempo,
    getTempoSyncInterval(), beatsElapsed, !isTempoSyncing || !isPlaying);

  bool enableMidSide = pv[ID::channelType]->getInt();

  for (size_t i = 0; i < length; ++i) {
    float sig0 = in0[i];
    float sig1 = in1[i];
    if (enableMidSide) convertToMidSide(sig0, sig1);

    overSampler[0].push(sig0);
    overSampler[1].push(sig1);
    for (size_t idx = 0; idx < OverSampler::fold; ++idx) {
      auto pitchMain = interpPitchMain.process();
      auto pitchUnison = interpPitchUnison.process();
      auto lfoStereoOffset = interpLfoStereoOffset.process();
      auto lfoUnisonOffset = interpLfoUnisonOffset.process();
      auto lfoToPitch = interpLfoToPitch.process();
      auto lfoToUnison = interpLfoToUnison.process();
      auto delayTime = interpDelayTime.process();
      auto stereoLean = interpStereoLean.process();
      auto feedback = interpFeedback.process();
      auto highpassKp = interpHighpassCutoffKp.process();
      auto pitchCross = interpPitchCross.process();
      auto stereoCross = interpStereoCross.process();
      auto unisonMix = interpUnisonMix.process();
      auto dry = interpDry.process();
      auto wet = interpWet.process();

      auto crossMainTemp0 = lerp(shifterMainOut[0], shifterUnisonOut[0], pitchCross);
      auto crossMainTemp1 = lerp(shifterMainOut[1], shifterUnisonOut[1], pitchCross);
      auto crossMain0 = lerp(crossMainTemp0, crossMainTemp1, stereoCross);
      auto crossMain1 = lerp(crossMainTemp1, crossMainTemp0, stereoCross);

      auto crossUnisonTemp0 = lerp(shifterUnisonOut[0], shifterMainOut[0], pitchCross);
      auto crossUnisonTemp1 = lerp(shifterUnisonOut[1], shifterMainOut[1], pitchCross);
      auto crossUnison0 = lerp(crossUnisonTemp0, crossUnisonTemp1, stereoCross);
      auto crossUnison1 = lerp(crossUnisonTemp1, crossUnisonTemp0, stereoCross);

      lfo.offset[0] = lfoStereoOffset < 0 ? -lfoStereoOffset : 0; // main0
      lfo.offset[1] = lfoStereoOffset > 0 ? lfoStereoOffset : 0;  // main1
      lfo.offset[2] = lfo.offset[0] + lfoUnisonOffset;            // unison0
      lfo.offset[3] = lfo.offset[1] + lfoUnisonOffset;            // unison1

      lfo.process(synchronizer.process());

      auto lfoPitchMain0 = lfoToPitch * lfo.output[0];
      auto lfoPitchMain1 = lfoToPitch * lfo.output[1];
      auto lfoPitchUnison0 = (lfoToPitch + lfoToUnison) * lfo.output[2];
      auto lfoPitchUnison1 = (lfoToPitch + lfoToUnison) * lfo.output[3];

      auto leanedDelayTime = getStereoLean(delayTime, stereoLean);
      constexpr float eps = std::numeric_limits<float>::epsilon();
      if (std::fabs(stereoLean) < eps && std::fabs(lfoStereoOffset) < eps) {
        if (shifterMain[1].phase != shifterMain[0].phase) {
          shifterMain[1].syncPhase(shifterMain[0].phase, phaseSyncCutoffKp);
        }
        if (shifterUnison[1].phase != shifterUnison[0].phase) {
          shifterUnison[1].syncPhase(shifterUnison[0].phase, phaseSyncCutoffKp);
        }
      }

      shifterMainOut[0] = shifterMain[0].process(
        overSampler[0].at(idx), feedback * crossMain0, highpassKp,
        pitchMain + lfoPitchMain0, leanedDelayTime[0]);
      shifterMainOut[1] = shifterMain[1].process(
        overSampler[1].at(idx), feedback * crossMain1, highpassKp,
        pitchMain + lfoPitchMain1, leanedDelayTime[1]);

      shifterUnisonOut[0] = shifterUnison[0].process(
        overSampler[0].at(idx), feedback * crossUnison0, highpassKp,
        pitchUnison + lfoPitchUnison0, leanedDelayTime[0]);
      shifterUnisonOut[1] = shifterUnison[1].process(
        overSampler[1].at(idx), feedback * crossUnison1, highpassKp,
        pitchUnison + lfoPitchUnison1, leanedDelayTime[1]);

      overSampler[0].inputBuffer[idx] = dry * overSampler[0].at(idx)
        + wet * lerp(shifterMainOut[0], shifterUnisonOut[0], unisonMix);
      overSampler[1].inputBuffer[idx] = dry * overSampler[1].at(idx)
        + wet * lerp(shifterMainOut[1], shifterUnisonOut[1], unisonMix);
    }

    sig0 = overSampler[0].process();
    sig1 = overSampler[1].process();
    if (enableMidSide) convertToLeftRight(sig0, sig1);

    out0[i] = sig0;
    out1[i] = sig1;
  }
}
