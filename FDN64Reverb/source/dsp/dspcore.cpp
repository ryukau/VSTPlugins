// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T calcNotePitch(T note)
{
  auto pitch = std::exp2((note - T(69)) / T(12));
  return T(1) / std::max(pitch, std::numeric_limits<float>::epsilon());
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(0.2f);

  for (size_t idx = 0; idx < nDelay; ++idx) {
    lowpassLfoTime[0][idx].setCutoff(sampleRate, 1.0f);
    lowpassLfoTime[1][idx].setCutoff(sampleRate, 1.0f);
  }

  gate.setup(sampleRate, 0.001f);

  for (auto &fdn : feedbackDelayNetwork) fdn.setup(sampleRate, 1.0f);

  reset();
  startup();
}

size_t DSPCore::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  for (auto &fdn : feedbackDelayNetwork) {                                               \
    fdn.rate = pv[ID::delayTimeInterpRate]->getFloat();                                  \
  }                                                                                      \
                                                                                         \
  auto timeMul = pv[ID::timeMultiplier]->getFloat() * notePitchMultiplier;               \
  for (size_t idx = 0; idx < nDelay; ++idx) {                                            \
    auto time = timeMul * sampleRate * pv[ID::delayTime0 + idx]->getFloat();             \
    auto timeLfo = sampleRate * pv[ID::timeLfoAmount0 + idx]->getFloat();                \
                                                                                         \
    feedbackDelayNetwork[0].delayTimeSample[idx].METHOD(                                 \
      time + timeLfo * lowpassLfoTime[0][idx].value);                                    \
    feedbackDelayNetwork[1].delayTimeSample[idx].METHOD(                                 \
      time + timeLfo * lowpassLfoTime[1][idx].value);                                    \
                                                                                         \
    auto &&lowpassCutoffHz = pv[ID::lowpassCutoffHz0 + idx]->getFloat();                 \
    interpLowpassCutoff[idx].METHOD(                                                     \
      lowpassCutoffHz >= Scales::lowpassCutoffHz.getMax()                                \
        ? 1.0f                                                                           \
        : float(EMAFilter<double>::cutoffToP(sampleRate, lowpassCutoffHz)));             \
    auto &&highpassCutoffHz = pv[ID::highpassCutoffHz0 + idx]->getFloat();               \
    interpHighpassCutoff[idx].METHOD(                                                    \
      float(EMAFilter<double>::cutoffToP(sampleRate, highpassCutoffHz)));                \
  }                                                                                      \
  interpSplitPhaseOffset.METHOD(pv[ID::splitPhaseOffset]->getFloat());                   \
  interpSplitSkew.METHOD(std::pow(2.0f, pv[ID::splitSkew]->getFloat()) - 1.0f);          \
  interpStereoCross.METHOD(pv[ID::stereoCross]->getFloat());                             \
  interpFeedback.METHOD(pv[ID::feedback]->getFloat());                                   \
  interpDry.METHOD(pv[ID::dry]->getFloat());                                             \
  interpWet.METHOD(pv[ID::wet]->getFloat());                                             \
                                                                                         \
  gate.prepare(std::max(0.0f, pv[ID::gateThreshold]->getFloat()));

void DSPCore::reset()
{
  rng.seed(9999991);

  midiNotes.clear();
  noteStack.clear();
  notePitchMultiplier = float(1);

  std::uniform_real_distribution<float> timeLfoDist(0.0f, 1.0f);
  for (size_t idx = 0; idx < nDelay; ++idx) {
    lowpassLfoTime[0][idx].reset(timeLfoDist(rng));
    lowpassLfoTime[1][idx].reset(timeLfoDist(rng));
  }

  ASSIGN_PARAMETER(reset);

  crossBuffer.fill(0);
  gate.reset();
  for (auto &fdn : feedbackDelayNetwork) fdn.reset();
  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  std::uniform_real_distribution<float> timeLfoDist(0.0f, 1.0f);
  for (size_t idx = 0; idx < nDelay; ++idx) {
    lowpassLfoTime[0][idx].process(timeLfoDist(rng));
    lowpassLfoTime[1][idx].process(timeLfoDist(rng));
  }

  ASSIGN_PARAMETER(push);

  auto &&splitRotationHz = pv[ID::splitRotationHz]->getFloat();
  for (auto &fdn : feedbackDelayNetwork) fdn.prepare(sampleRate, splitRotationHz);

  unsigned seed = pv[ID::seed]->getInt();
  unsigned matrixType = pv[ID::matrixType]->getInt();
  if (
    prepareRefresh || (!isMatrixRefeshed && pv[ID::refreshMatrix]->getInt())
    || seed != previousSeed || previousMatrixType != matrixType)
  {
    previousSeed = seed;
    previousMatrixType = matrixType;

    pcg64 matrixRng{seed};
    std::uniform_int_distribution<unsigned> seedDist{
      0, std::numeric_limits<unsigned>::max()};
    feedbackDelayNetwork[0].randomizeMatrix(matrixType, seedDist(matrixRng));
    feedbackDelayNetwork[1].randomizeMatrix(matrixType, seedDist(matrixRng));
  }
  isMatrixRefeshed = pv[ID::refreshMatrix]->getInt();
  prepareRefresh = false;
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    for (size_t idx = 0; idx < nDelay; ++idx) {
      auto lowpassCutoff = interpLowpassCutoff[idx].process();
      auto highpassCutoff = interpHighpassCutoff[idx].process();
      for (auto &fdn : feedbackDelayNetwork) {
        fdn.lowpassKp[idx] = lowpassCutoff;
        fdn.highpassKp[idx] = highpassCutoff;
      }
    }

    auto splitPhaseOffset = interpSplitPhaseOffset.process();
    auto splitSkew = interpSplitSkew.process();
    auto stereoCross = interpStereoCross.process();
    auto feedback = interpFeedback.process();

    auto gateOut = gate.process(std::max(std::fabs(in0[i]), std::fabs(in1[i])));
    stereoCross = std::min(1.0f, stereoCross + (1.0f - stereoCross) * gateOut);

    auto fdnBuf0 = feedbackDelayNetwork[0].preProcess(splitPhaseOffset, splitSkew);
    auto fdnBuf1 = feedbackDelayNetwork[1].preProcess(splitPhaseOffset, splitSkew);
    crossBuffer[0]
      = feedbackDelayNetwork[0].process(in0[i], fdnBuf1, stereoCross, feedback);
    crossBuffer[1]
      = feedbackDelayNetwork[1].process(in1[i], fdnBuf0, stereoCross, feedback);

    auto dry = interpDry.process();
    auto wet = interpWet.process();
    out0[i] = dry * in0[i] + wet * crossBuffer[0];
    out1[i] = dry * in1[i] + wet * crossBuffer[1];
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  notePitchMultiplier = calcNotePitch(info.pitch);
  updateDelayTime();

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
    notePitchMultiplier = float(1);
  } else {
    notePitchMultiplier = calcNotePitch(noteStack.back().pitch);
  }
  updateDelayTime();
}

void DSPCore::updateDelayTime()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto timeMul = pv[ID::timeMultiplier]->getFloat() * notePitchMultiplier;
  for (size_t idx = 0; idx < nDelay; ++idx) {
    auto time = timeMul * sampleRate * pv[ID::delayTime0 + idx]->getFloat();
    auto timeLfo = sampleRate * pv[ID::timeLfoAmount0 + idx]->getFloat();

    feedbackDelayNetwork[0].delayTimeSample[idx].push(
      time + timeLfo * lowpassLfoTime[0][idx].value);
    feedbackDelayNetwork[1].delayTimeSample[idx].push(
      time + timeLfo * lowpassLfoTime[1][idx].value);
  }
}
