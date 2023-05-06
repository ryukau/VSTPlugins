// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelDetune.
//
// ParallelDetune is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelDetune is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelDetune.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float defaultTempo = 120.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T freqToSvfG(T normalizedFreq)
{
  static constexpr T minCutoff = T(0.00001);
  static constexpr T nyquist = T(0.49998);

  return std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * T(pi));
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);
  upRate = double(sampleRate) * upFold;

  pitchSmoothingKp = EMAFilter<double>::secondToP(upRate, double(0.01));
  phaseSyncKp = EMAFilter<double>::secondToP(upRate, double(1));

  for (auto &ps : pitchShifter) ps.setup(size_t(upRate * maxDelayTime));

  SmootherCommon<double>::setSampleRate(upRate);

  synchronizer.reset(upRate, defaultTempo, double(1));
  lfo.setup(upRate, double(0.1));

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
  lfoPhaseOffset.METHOD(pv[ID::lfoPhaseOffset]->getDouble());                            \
  lfoPhaseConstant.METHOD(pv[ID::lfoPhaseConstant]->getDouble());                        \
  dryGain.METHOD(pv[ID::dryGain]->getDouble());                                          \
  wetGain.METHOD(pv[ID::wetGain]->getDouble() / nShifter);                               \
  panSpread.METHOD(pv[ID::panSpread]->getDouble());                                      \
  lfoToPan.METHOD(pv[ID::lfoToPan]->getDouble());                                        \
  tremoloMix.METHOD(pv[ID::tremoloMix]->getDouble());                                    \
  tremoloLean.METHOD(pv[ID::tremoloLean]->getDouble());                                  \
  feed.METHOD(pv[ID::feed]->getDouble());                                                \
  lfoToDelayTime.METHOD(pv[ID::lfoToDelayTime]->getDouble());                            \
  lfoToShiftPitch.METHOD(pv[ID::lfoToShiftPitch]->getDouble());                          \
                                                                                         \
  auto shiftTransposeSemitone = pv[ID::shiftTransposeSemitone]->getDouble();             \
  auto shifterDelayTimeBase = pv[ID::delayTimeSeconds]->getDouble() * upRate;            \
  auto highpassNormalizedCutoff = pv[ID::highpassHz]->getDouble() / upRate;              \
  auto lowpassNormalizedCutoff = pv[ID::lowpassNormalizedCutoff]->getDouble();           \
  for (size_t idx = 0; idx < nShifter; ++idx) {                                          \
    auto semitone = shiftTransposeSemitone + pv[ID::shiftSemitone0 + idx]->getDouble();  \
    auto cent = pv[ID::shiftFineTuningCent0 + idx]->getDouble();                         \
    auto pitch = std::exp2(semitone / double(12) + cent / double(1200));                 \
    shiftPitch.METHOD##At(idx, pitch);                                                   \
                                                                                         \
    delayTimeSamples.METHOD##At(                                                         \
      idx,                                                                               \
      pv[ID::shifterDelayTimeMultiplier0 + idx]->getDouble() * shifterDelayTimeBase);    \
                                                                                         \
    shifterGain.METHOD##At(idx, pv[ID::shifterGain0 + idx]->getDouble());                \
                                                                                         \
    auto highpassOffset = std::exp2(pv[ID::shifterHighpassOffset0 + idx]->getDouble());  \
    auto lowpassOffset = std::exp2(pv[ID::shifterLowpassOffset0 + idx]->getDouble());    \
    highpassG.METHOD##At(idx, freqToSvfG(highpassOffset *highpassNormalizedCutoff));     \
    lowpassG.METHOD##At(idx, freqToSvfG(lowpassOffset *lowpassNormalizedCutoff));        \
  }

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  midiNotes.clear();
  noteStack.clear();

  notePitch.reset(double(1));

  lfo.reset();

  feedbackBuffer.fill({});
  mixBuffer.fill({});
  for (auto &x : upSampler) x.reset();
  for (auto &x : feedbackHighpass) x.reset();
  for (auto &channel : feedbackLowpass) {
    for (auto &x : channel) x.reset();
  }
  for (auto &x : pitchShifter) x.reset();
  for (auto &x : halfbandIir) x.reset();

  startup();
}

void DSPCore::startup() { synchronizer.reset(upRate, tempo, getTempoSyncInterval()); }

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

std::array<double, 2> DSPCore::processFrame(double in0, double in1)
{
  notePitch.process(pitchSmoothingKp);

  lfoPhaseConstant.process();
  lfoPhaseOffset.process();

  outputGain.process();
  dryGain.process();
  wetGain.process();
  panSpread.process();
  lfoToPan.process();
  tremoloMix.process();
  tremoloLean.process();
  feed.process();
  lfoToDelayTime.process();
  lfoToShiftPitch.process();

  shiftPitch.process();
  delayTimeSamples.process();
  shifterGain.process();

  highpassG.process();
  lowpassG.process();

  // LFO.
  lfo.offset[0] = lfoPhaseConstant.getValue() + lfoPhaseOffset.getValue();
  lfo.offset[1] = lfoPhaseConstant.getValue() - lfoPhaseOffset.getValue();
  auto lfoPhase = synchronizer.process();
  lfo.process(lfoPhase);

  // Allpass shifter.
  for (size_t idx = 0; idx < nShifter; ++idx) {
    feedbackBuffer[0][idx] = in0 - feed.getValue() * feedbackBuffer[0][idx];
    feedbackBuffer[1][idx] = in1 - feed.getValue() * feedbackBuffer[1][idx];
  }

  feedbackHighpass[0].highpass(feedbackBuffer[0], highpassG.value);
  feedbackHighpass[1].highpass(feedbackBuffer[1], highpassG.value);
  for (auto &lp : feedbackLowpass[0]) lp.lowpass(feedbackBuffer[0], lowpassG.value);
  for (auto &lp : feedbackLowpass[1]) lp.lowpass(feedbackBuffer[1], lowpassG.value);

  constexpr double eps = std::numeric_limits<double>::epsilon();
  if (lfoPhaseOffset.getValue() <= eps || lfoPhaseOffset.getValue() >= double(1) - eps) {
    pitchShifter[1].syncPhase(pitchShifter[0].getPhase(), phaseSyncKp);
  }

  auto modPitch0
    = notePitch.getValue() * std::exp2(lfo.output[0] * lfoToShiftPitch.getValue());
  auto modPitch1
    = notePitch.getValue() * std::exp2(lfo.output[1] * lfoToShiftPitch.getValue());
  auto modTime0 = std::exp2(lfo.output[0] * lfoToDelayTime.getValue());
  auto modTime1 = std::exp2(lfo.output[1] * lfoToDelayTime.getValue());
  pitchShifter[0].process(
    feedbackBuffer[0], shiftPitch.value, delayTimeSamples.value, modPitch0, modTime0);
  pitchShifter[1].process(
    feedbackBuffer[1], shiftPitch.value, delayTimeSamples.value, modPitch1, modTime1);

  // Assuming `nShifter` is even number.
  for (size_t idx = 0; idx < nShifter / 2; ++idx) {
    spreadGain[0][idx] = double(0.5)
      + double(0.5) * std::cos(double(twopi) * (lfoPhase + idx / double(nShifter)));
    spreadGain[1][idx] = double(1) - spreadGain[0][idx];

    spreadGain[0][idx + nShifter / 2] = -spreadGain[0][idx];
    spreadGain[1][idx + nShifter / 2] = -spreadGain[1][idx];
  }

  for (size_t idx = 0; idx < nShifter; ++idx) {
    auto tremoloGain
      = lerp(spreadGain[0][idx], spreadGain[1][idx], tremoloLean.getValue());
    auto gain
      = shifterGain.value[idx] * lerp(double(1), tremoloGain, tremoloMix.getValue());
    mixBuffer[0][idx] = feedbackBuffer[0][idx] * gain;
    mixBuffer[1][idx] = feedbackBuffer[1][idx] * gain;
  }

  // Output mix.
  auto wet0 = std::accumulate(mixBuffer[0].begin(), mixBuffer[0].end(), double(0));
  auto wet1 = std::accumulate(mixBuffer[1].begin(), mixBuffer[1].end(), double(0));

  auto panned0 = double(0);
  auto panned1 = double(0);
  for (size_t idx = 0; idx < nShifter; ++idx) {
    auto gain0
      = lerp(idx / double(nShifter - 1), spreadGain[0][idx], lfoToPan.getValue());
    auto gain1 = lerp(
      (nShifter - 1 - idx) / double(nShifter - 1), spreadGain[1][idx],
      lfoToPan.getValue());
    panned0 += gain0 * (mixBuffer[0][idx] + mixBuffer[1][idx]);
    panned1 += gain1 * (mixBuffer[0][idx] + mixBuffer[1][idx]);
  }

  auto normalizeGain = double(1) + tremoloMix.getValue();
  in0 = dryGain.getValue() * in0
    + wetGain.getValue() * lerp(wet0, normalizeGain * panned0, panSpread.getValue());
  in1 = dryGain.getValue() * in1
    + wetGain.getValue() * lerp(wet1, normalizeGain * panned1, panSpread.getValue());

  return {in0, in1};
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  // When tempo-sync is off, use defaultTempo BPM.
  bool isTempoSyncing = pv[ID::lfoTempoSync]->getInt();
  synchronizer.prepare(
    upRate, isTempoSyncing ? tempo : defaultTempo, getTempoSyncInterval(), beatsElapsed,
    !isTempoSyncing || !isPlaying);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);
    for (size_t j = 0; j < 2; ++j) {
      auto frame = processFrame(upSampler[0].output[j], upSampler[1].output[j]);
      upSampler[0].output[j] = frame[0];
      upSampler[1].output[j] = frame[1];
    }
    out0[i] = halfbandIir[0].process(upSampler[0].output);
    out1[i] = halfbandIir[1].process(upSampler[1].output);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  notePitch.push(calcNotePitch(info.pitch));

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    notePitch.push(double(1));
  } else {
    notePitch.push(calcNotePitch(noteStack.back().pitch));
  }
}

double DSPCore::calcNotePitch(double note, double equalTemperament)
{
  // using ID = ParameterID::ID;
  // auto &pv = param.value;

  return std::exp2((note - 69) / equalTemperament);
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
