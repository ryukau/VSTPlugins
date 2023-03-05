// (c) 2022 Takamitsu Endo
//
// This file is part of ComplexRotation.
//
// ComplexRotation is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ComplexRotation is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ComplexRotation.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <complex>
#include <limits>
#include <numeric>

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

template<typename T> inline T freqToSvfG(T normalizedFreq)
{
  static constexpr T minCutoff = T(0.00001);
  static constexpr T nyquist = T(0.49998);

  return std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * T(pi));
}

template<typename T> inline T addAsymmetry(T value, T asym)
{
  auto mult = value * asym;
  return mult > 0 ? std::copysign(mult, value) : value;
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

  pitchSmoothingKp = EMAFilter<double>::secondToP(upRate, double(0.05));
  pitchReleaseKp = EMAFilter<double>::secondToP(upRate, double(4));

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
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
  mix.METHOD(pv[ID::mix]->getDouble());                                                  \
  inputPhaseMod.METHOD(pv[ID::inputPhaseMod]->getDouble());                              \
  inputLowpassG.METHOD(freqToSvfG(pv[ID::inputLowpassHz]->getDouble() / upRate));

void DSPCore::updateUpRate()
{
  auto fold = oversampling ? upFold : size_t(1);
  upRate = double(sampleRate) * fold;

  SmootherCommon<double>::setSampleRate(upRate);
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  midiNotes.clear();
  noteStack.clear();

  notePitchToDelayTime.reset(double(1));
  notePitchToAllpassCutoff.reset(double(1));
  notePitchToDelayTimeRelease.reset(double(1));
  notePitchToAllpassCutoffRelease.reset(double(1));

  phase.fill({});
  for (auto &x : inputLowpass) x.reset();

  previousInput.fill({});
  upsampleBuffer.fill({});
  for (auto &x : halfbandIir) x.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  bool newOversampling = param.value[ParameterID::ID::oversampling]->getInt();
  if (oversampling != newOversampling) {
    oversampling = newOversampling;
    updateUpRate();
  }

  ASSIGN_PARAMETER(push);
}

void DSPCore::processFrame(std::array<double, 2> &frame)
{
  outputGain.process();
  mix.process();
  inputPhaseMod.process();
  inputLowpassG.process();

  constexpr double eps = (double)std::numeric_limits<float>::epsilon();

  auto inLp0 = inputLowpass[0].lowpass(frame[0], inputLowpassG.getValue());
  auto inLp1 = inputLowpass[1].lowpass(frame[1], inputLowpassG.getValue());

  phase[0] += inputPhaseMod.getValue() * inLp0;
  phase[1] += inputPhaseMod.getValue() * inLp1;
  phase[0] -= std::floor(phase[0]);
  phase[1] -= std::floor(phase[1]);

  auto rot0 = frame[0] * std::cos(double(twopi) * phase[0]);
  auto rot1 = frame[1] * std::cos(double(twopi) * phase[1]);

  frame[0] = outputGain.process() * lerp(frame[0], rot0, mix.getValue());
  frame[1] = outputGain.process() * lerp(frame[1], rot1, mix.getValue());
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (oversampling) {
      // Crude up-sampling with linear interpolation.
      upsampleBuffer[0] = {
        double(0.5) * (previousInput[0] + in0[i]),
        double(0.5) * (previousInput[1] + in1[i])};
      upsampleBuffer[1] = {double(in0[i]), double(in1[i])};

      for (size_t j = 0; j < upFold; ++j) processFrame(upsampleBuffer[j]);

      out0[i] = halfbandIir[0].process({upsampleBuffer[0][0], upsampleBuffer[1][0]});
      out1[i] = halfbandIir[1].process({upsampleBuffer[0][1], upsampleBuffer[1][1]});
    } else {
      upsampleBuffer[0] = {double(in0[i]), double(in1[i])};

      processFrame(upsampleBuffer[0]);

      out0[i] = upsampleBuffer[0][0];
      out1[i] = upsampleBuffer[0][1];
    }

    previousInput[0] = in0[i];
    previousInput[1] = in1[i];
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto pitchToTime
    = calcNotePitch(info.pitch, -pv[ID::notePitchToDelayTime]->getDouble());
  notePitchToDelayTime.push(pitchToTime);
  notePitchToDelayTimeRelease.reset(pitchToTime);

  auto pitchToCutoff
    = calcNotePitch(info.pitch, pv[ID::notePitchToAllpassCutoff]->getDouble());
  notePitchToAllpassCutoff.push(pitchToCutoff);
  notePitchToAllpassCutoffRelease.reset(pitchToCutoff);

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    notePitchToDelayTime.push(double(1));
    notePitchToAllpassCutoff.push(double(1));
  } else {
    notePitchToDelayTime.push(
      calcNotePitch(noteStack.back().pitch, pv[ID::notePitchToDelayTime]->getDouble()));
    notePitchToAllpassCutoff.push(calcNotePitch(
      noteStack.back().pitch, pv[ID::notePitchToAllpassCutoff]->getDouble()));
  }
}

double DSPCore::calcNotePitch(double note, double scale, double equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto center = pv[ID::notePitchOrigin]->getDouble();
  return std::max(
    std::exp2(scale * (note - center) / equalTemperament),
    std::numeric_limits<double>::epsilon());
}
