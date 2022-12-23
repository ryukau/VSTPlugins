// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float defaultTempo = 120.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T processLfoWave(T phase, T clip, T skew) noexcept
{
  phase -= std::floor(phase);
  auto sn = std::sin(T(twopi) * std::pow(phase, skew));
  return std::clamp(clip * sn, T(-1), T(1));
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  for (auto &ps : pitchShifter)
    ps.setup(size_t(this->sampleRate * maxUpFold * maxDelayTime));

  reset();
  startup();
}

size_t DSPCore::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  SmootherCommon<double>::setTime(pv[ID::parameterSmoothingSecond]->getDouble());        \
                                                                                         \
  lfoPhaseOffset.METHOD(pv[ID::lfoPhaseOffset]->getDouble());                            \
  lfoPhaseConstant.METHOD(pv[ID::lfoPhaseConstant]->getDouble());                        \
  lfoShapeClip.METHOD(pv[ID::lfoShapeClip]->getDouble());                                \
  lfoShapeSkew.METHOD(pv[ID::lfoShapeSkew]->getDouble());                                \
  dryGain.METHOD(pv[ID::dryGain]->getDouble());                                          \
  wetGain.METHOD(pv[ID::wetGain]->getDouble());                                          \
  feedback.METHOD(pv[ID::feedback]->getDouble());                                        \
  delayTimeSamples.METHOD(pv[ID::delayTimeSeconds]->getDouble() * upRate);               \
  shiftPitch.METHOD(std::exp2(pv[ID::shiftPitch]->getDouble()));                         \
  shiftFreq.METHOD(pv[ID::shiftHz]->getDouble() / upRate);                               \
  lfoToPrimaryDelayTime.METHOD(pv[ID::lfoToPrimaryDelayTime]->getDouble());              \
  lfoToPrimaryShiftPitch.METHOD(pv[ID::lfoToPrimaryShiftPitch]->getDouble());            \
  lfoToPrimaryShiftHz.METHOD(pv[ID::lfoToPrimaryShiftHz]->getDouble());                  \
                                                                                         \
  for (auto &hp : feedbackHighpass) {                                                    \
    hp.METHOD##Cutoff(pv[ID::highpassHz]->getDouble() / upRate, double(0.7));            \
  }                                                                                      \
  for (auto &lp : feedbackLowpass) {                                                     \
    lp.METHOD##Cutoff(pv[ID::lowpassHz]->getDouble() / upRate, double(0.7));             \
  }

void DSPCore::updateUpRate()
{
  constexpr std::array<size_t, 3> fold{1, 2, 8};
  upRate = double(sampleRate) * fold[oversampling];

  SmootherCommon<double>::setSampleRate(upRate);

  synchronizer.reset(upRate, defaultTempo, double(1));
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  feedbackBuffer.fill({});
  for (auto &x : upSampler) x.reset();
  for (auto &x : feedbackHighpass) x.reset();
  for (auto &x : feedbackLowpass) x.reset();
  for (auto &x : frequencyShifter) x.reset();
  for (auto &x : pitchShifter) x.reset();
  for (auto &x : decimationLowpass) x.reset();
  for (auto &x : halfbandIir) x.reset();

  startup();
}

void DSPCore::startup() { synchronizer.reset(upRate, tempo, getTempoSyncInterval()); }

void DSPCore::setParameters()
{
  size_t newOversampling = param.value[ParameterID::ID::oversampling]->getInt();
  if (oversampling != newOversampling) {
    oversampling = newOversampling;
    updateUpRate();
  }

  ASSIGN_PARAMETER(push);
}

std::array<double, 2> DSPCore::processFrame(double in0, double in1)
{
  lfoPhaseConstant.process();
  lfoPhaseOffset.process();
  lfoShapeClip.process();
  lfoShapeSkew.process();
  outputGain.process();
  dryGain.process();
  wetGain.process();
  feedback.process();
  delayTimeSamples.process();
  shiftPitch.process();
  shiftFreq.process();
  lfoToPrimaryDelayTime.process();
  lfoToPrimaryShiftPitch.process();
  lfoToPrimaryShiftHz.process();

  // LFO.
  auto lfoPhase = synchronizer.process() + lfoPhaseConstant.getValue();
  auto lfoOut0 = processLfoWave(
    lfoPhase + lfoPhaseOffset.getValue(), lfoShapeClip.getValue(),
    lfoShapeSkew.getValue());
  auto lfoOut1 = processLfoWave(
    lfoPhase - lfoPhaseOffset.getValue(), lfoShapeClip.getValue(),
    lfoShapeSkew.getValue());

  // Primary feedback shifter.
  auto fb0 = feedbackLowpass[0].lowpass(
    feedbackHighpass[0].highpass(in0 - feedback.getValue() * feedbackBuffer[0]));
  auto fb1 = feedbackLowpass[1].lowpass(
    feedbackHighpass[1].highpass(in1 - feedback.getValue() * feedbackBuffer[1]));

  auto modHz0 = std::exp2(lfoOut0 * lfoToPrimaryShiftHz.getValue());
  auto modHz1 = std::exp2(lfoOut1 * lfoToPrimaryShiftHz.getValue());
  auto fs0 = frequencyShifter[0].process(fb0, shiftFreq.getValue() * modHz0);
  auto fs1 = frequencyShifter[1].process(fb1, shiftFreq.getValue() * modHz1);

  auto modPitch0 = std::exp2(lfoOut0 * lfoToPrimaryShiftPitch.getValue());
  auto modPitch1 = std::exp2(lfoOut1 * lfoToPrimaryShiftPitch.getValue());
  auto modTime0 = std::exp2(lfoOut0 * lfoToPrimaryDelayTime.getValue());
  auto modTime1 = std::exp2(lfoOut1 * lfoToPrimaryDelayTime.getValue());
  feedbackBuffer[0] = pitchShifter[0].process(
    fs0, shiftPitch.getValue() * modPitch0, delayTimeSamples.getValue() * modTime0);
  feedbackBuffer[1] = pitchShifter[1].process(
    fs1, shiftPitch.getValue() * modPitch1, delayTimeSamples.getValue() * modTime1);

  // Output mix.
  in0 = dryGain.getValue() * in0 + wetGain.getValue() * feedbackBuffer[0];
  in1 = dryGain.getValue() * in1 + wetGain.getValue() * feedbackBuffer[1];

  return {in0, in1};
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    upRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(), beatsElapsed,
    !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);

    if (oversampling == 0) { // 1x.
      auto frame = processFrame(upSampler[0].output[0], upSampler[1].output[0]);
      out0[i] = frame[0];
      out1[i] = frame[1];
    } else if (oversampling == 1) { // 2x.
      for (size_t j = 0; j < 8; j += 4) {
        auto frame = processFrame(upSampler[0].output[j], upSampler[1].output[j]);
        upSampler[0].output[j] = frame[0];
        upSampler[1].output[j] = frame[1];
      }
      out0[i] = halfbandIir[0].process({upSampler[0].output[0], upSampler[0].output[4]});
      out1[i] = halfbandIir[1].process({upSampler[1].output[0], upSampler[1].output[4]});
    } else { // `oversampling == 2`, or 8x.
      for (size_t j = 0; j < maxUpFold; ++j) {
        auto frame = processFrame(upSampler[0].output[j], upSampler[1].output[j]);
        decimationLowpass[0].push(frame[0]);
        decimationLowpass[1].push(frame[1]);
        upSampler[0].output[j] = decimationLowpass[0].output();
        upSampler[1].output[j] = decimationLowpass[1].output();
      }
      out0[i] = halfbandIir[0].process({upSampler[0].output[0], upSampler[0].output[4]});
      out1[i] = halfbandIir[1].process({upSampler[1].output[0], upSampler[1].output[4]});
    }
  }
}

double DSPCore::getTempoSyncInterval()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto lfoRate = pv[ID::lfoRate]->getDouble();
  if (lfoRate > Scales::lfoRate.getMax()) return 0;

  // Multiplying with 4 because 1 beat is 1/4 bar.
  auto upper = pv[ID::lfoTempoUpper]->getDouble() + double(1);
  auto lower = pv[ID::lfoTempoLower]->getDouble() + double(1);
  return pv[ID::lfoTempoSync]->getInt()
    ? (4 * timeSigUpper * upper) / (timeSigLower * lower * lfoRate)
    : (4 * upper) / (lower * lfoRate);
}
