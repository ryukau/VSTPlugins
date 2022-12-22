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
  auto sn = std::sin(T(twopi) * std::pow(phase, skew));
  return std::clamp(clip * sn, T(-1), T(1));
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);
  upRate = double(sampleRate) * upFold;

  SmootherCommon<double>::setSampleRate(upRate);

  synchronizer.reset(upRate, defaultTempo, double(1));

  for (auto &ps : pitchShifter) ps.setup(size_t(upRate * maxDelayTime));

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

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  previousInput.fill(0);
  frame.fill({});

  feedbackBuffer.fill({});
  secondaryBuffer.fill({});
  for (auto &hp : feedbackHighpass) hp.reset();
  for (auto &lp : feedbackLowpass) lp.reset();
  for (auto &fs : frequencyShifter) fs.reset();
  for (auto &ps : pitchShifter) ps.reset();
  for (auto &hb : halfbandIir) hb.reset();

  startup();
}

void DSPCore::startup() { synchronizer.reset(upRate, tempo, getTempoSyncInterval()); }

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

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
    // Crude up-sampling with linear interpolation.
    frame[0][0] = double(0.5) * (previousInput[0] + in0[i]);
    frame[1][0] = double(0.5) * (previousInput[1] + in1[i]);
    frame[0][1] = in0[i];
    frame[1][1] = in1[i];

    for (size_t j = 0; j < upFold; ++j) {
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
      auto fb0 = feedbackLowpass[0].lowpass(feedbackHighpass[0].highpass(
        frame[0][j] - feedback.getValue() * feedbackBuffer[0]));
      auto fb1 = feedbackLowpass[1].lowpass(feedbackHighpass[1].highpass(
        frame[1][j] - feedback.getValue() * feedbackBuffer[1]));

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
      frame[0][j]
        = dryGain.getValue() * frame[0][j] + wetGain.getValue() * feedbackBuffer[0];
      frame[1][j]
        = dryGain.getValue() * frame[1][j] + wetGain.getValue() * feedbackBuffer[1];
    }

    out0[i] = halfbandIir[0].process(frame[0]);
    out1[i] = halfbandIir[1].process(frame[1]);

    previousInput[0] = in0[i];
    previousInput[1] = in1[i];
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
