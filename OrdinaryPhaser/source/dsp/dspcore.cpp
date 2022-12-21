// (c) 2022 Takamitsu Endo
//
// This file is part of OrdinaryPhaser.
//
// OrdinaryPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OrdinaryPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OrdinaryPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float defaultTempo = 120.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);
  upRate = double(sampleRate) * upFold;

  SmootherCommon<double>::setSampleRate(upRate);

  synchronizer.reset(upRate, defaultTempo, double(1));
  lfo.setup(upRate, double(0.1) * upFold);

  for (auto &dly : feedbackDelay) dly.setup(upRate, maxDelayTime);

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
  lfo.interpType = pv[ID::lfoInterpolation]->getInt();                                   \
  for (size_t idx = 0; idx < nLfoWavetable; ++idx) {                                     \
    lfo.source[idx + 1] = pv[ID::lfoWavetable0 + idx]->getFloat();                       \
  }                                                                                      \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  mix.METHOD(pv[ID::mix]->getDouble());                                                  \
  lfoPhaseOffset.METHOD(pv[ID::lfoPhaseOffset]->getDouble());                            \
  lfoPhaseConstant.METHOD(pv[ID::lfoPhaseConstant]->getDouble());                        \
  cutoffSpread.METHOD(pv[ID::cutoffSpread]->getDouble());                                \
  cutoffMinHz.METHOD(pv[ID::cutoffMinHz]->getDouble());                                  \
  cutoffMaxHz.METHOD(pv[ID::cutoffMaxHz]->getDouble());                                  \
  feedback.METHOD(pv[ID::feedback]->getDouble());                                        \
  delayTimeSamples.METHOD(pv[ID::delayTimeSeconds]->getDouble() * upRate);               \
  lfoToDelay.METHOD(pv[ID::lfoToDelayAmount]->getDouble());                              \
  lfoToAmplitude.METHOD(pv[ID::lfoToAmplitude]->getDouble());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  previousInput.fill(0);
  frame.fill({});

  currentAllpassStage = pv[ID::stage]->getInt();
  previousAllpassStage = currentAllpassStage;
  transitionSamples = size_t(upRate * pv[ID::parameterSmoothingSecond]->getDouble());
  transitionCounter = 0;

  feedbackBuffer.fill({});
  for (auto &channel : allpass) {
    for (auto &ap : channel) ap.reset();
  }
  for (auto &dly : feedbackDelay) dly.reset();
  for (auto &hb : halfbandIir) hb.reset();

  startup();
}

void DSPCore::startup()
{
  synchronizer.reset(upRate, tempo, getTempoSyncInterval());
  phase = 0;
}

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

  size_t lfoToDelayTuningType = pv[ID::lfoToDelayTuningType]->getInt();

  if (transitionCounter == 0) {
    currentAllpassStage = pv[ID::stage]->getInt();
    if (previousAllpassStage != currentAllpassStage) {
      auto scale = lerp(
        1.0, 0.2,
        std::min(previousAllpassStage, currentAllpassStage) / double(maxAllpass - 1));
      auto smoothingSecond
        = std::min(double(2), pv[ID::parameterSmoothingSecond]->getDouble());
      transitionSamples = size_t(1) + size_t(scale * upRate * smoothingSecond);
      transitionCounter = transitionSamples;
    }
  }

  for (size_t i = 0; i < length; ++i) {
    // Crude up-sampling with linear interpolation.
    frame[0][0] = double(0.5) * (previousInput[0] + in0[i]);
    frame[1][0] = double(0.5) * (previousInput[1] + in1[i]);
    frame[0][1] = in0[i];
    frame[1][1] = in1[i];

    for (size_t j = 0; j < upFold; ++j) {
      lfoPhaseConstant.process();
      outputGain.process();
      mix.process();
      lfoPhaseOffset.process();
      cutoffSpread.process();
      auto cutMinHz = cutoffMinHz.process();
      auto cutMaxHz = cutoffMaxHz.process();
      feedback.process();
      auto baseTime = delayTimeSamples.process();
      lfoToDelay.process();
      lfoToAmplitude.process();

      lfo.offset[0] = lfoPhaseConstant.getValue() + lfoPhaseOffset.getValue();
      lfo.offset[1] = lfoPhaseConstant.getValue() - lfoPhaseOffset.getValue();
      lfo.process(synchronizer.process());

      if (cutMinHz > cutMaxHz) std::swap(cutMaxHz, cutMaxHz);
      auto rangeHz = double(0.5) * (cutMaxHz - cutMinHz);
      auto centerHz = cutMinHz + rangeHz;
      auto apCut0 = (centerHz + rangeHz * lfo.output[0]) / upRate;
      auto apCut1 = (centerHz + rangeHz * lfo.output[1]) / upRate;

      std::array<double, 2> dt{};
      switch (lfoToDelayTuningType) {
        case 0: { // Exp Mul.
          auto amount = double(8) * lfoToDelay.getValue();
          dt[0] = baseTime * std::exp2(amount * lfo.output[0]);
          dt[1] = baseTime * std::exp2(amount * lfo.output[1]);
        } break;
        case 1: { // Linear Mul.
          dt[0] = baseTime * (double(1) + lfoToDelay.getValue() * lfo.output[0]);
          dt[1] = baseTime * (double(1) + lfoToDelay.getValue() * lfo.output[1]);
        } break;
        case 2: { // Add
          auto range = lfoToDelay.getValue() * maxDelayTime * upRate;
          dt[0] = baseTime + lfo.output[0] * range;
          dt[1] = baseTime + lfo.output[1] * range;
        } break;
        case 3: { // Fill Lower
          auto range = lfoToDelay.getValue() * double(0.5) * baseTime;
          dt[0] = baseTime - range + lfo.output[0] * range;
          dt[1] = baseTime - range + lfo.output[1] * range;
        } break;
        case 4: { // Fill Higher
          auto range
            = lfoToDelay.getValue() * double(0.5) * (maxDelayTime * upRate - baseTime);
          dt[0] = baseTime + range + lfo.output[0] * range;
          dt[1] = baseTime + range + lfo.output[1] * range;
        } break;
      }
      frame[0][j]
        += feedbackDelay[0].process(feedback.getValue() * feedbackBuffer[0], dt[0]);
      frame[1][j]
        += feedbackDelay[1].process(feedback.getValue() * feedbackBuffer[1], dt[1]);

      for (size_t idx = 0; idx < maxAllpass; ++idx) {
        auto multiplier = double(1) + idx * cutoffSpread.getValue();
        frame[0][j] = allpass[0][idx].process(frame[0][j], apCut0 * multiplier);
        frame[1][j] = allpass[1][idx].process(frame[1][j], apCut1 * multiplier);
      }

      auto apOut0 = allpass[0][currentAllpassStage].output();
      auto apOut1 = allpass[1][currentAllpassStage].output();

      // Process cross-fade only when allpass stage is changed.
      if (transitionCounter > 0) {
        --transitionCounter;
        auto ratio = double(transitionCounter) / double(transitionSamples);
        apOut0 += ratio * (allpass[0][previousAllpassStage].output() - apOut0);
        apOut1 += ratio * (allpass[1][previousAllpassStage].output() - apOut1);
      }

      auto am0 = lerp(double(1), lfo.output[0], lfoToAmplitude.getValue());
      auto am1 = lerp(double(1), lfo.output[1], lfoToAmplitude.getValue());

      feedbackBuffer[0] = lerp(double(in0[i]), am0 * apOut0, mix.getValue());
      feedbackBuffer[1] = lerp(double(in1[i]), am1 * apOut1, mix.getValue());

      frame[0][j] = feedbackBuffer[0] * outputGain.getValue();
      frame[1][j] = feedbackBuffer[1] * outputGain.getValue();
    }

    out0[i] = halfbandIir[0].process(frame[0]);
    out1[i] = halfbandIir[1].process(frame[1]);

    previousInput[0] = in0[i];
    previousInput[1] = in1[i];
  }

  if (transitionCounter == 0) previousAllpassStage = currentAllpassStage;
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
