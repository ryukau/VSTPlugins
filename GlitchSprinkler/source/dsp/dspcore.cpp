// (c) 2023 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = sampleRate;

  SmootherCommon<double>::setTime(double(0.2));

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
                                                                                         \
  paramRng.seed(pv[ID::seed]->getInt());                                                 \
                                                                                         \
  for (size_t idx = 0; idx < nPolyOscControl; ++idx) {                                   \
    polynomial.polyX[idx + 1] = pv[ID::polynomialPointX0 + idx]->getDouble();            \
    polynomial.polyY[idx + 1] = pv[ID::polynomialPointY0 + idx]->getDouble();            \
  }

void DSPCore::reset()
{
  velocity = 0;

  ASSIGN_PARAMETER(reset);

  startup();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;
}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

std::array<double, 2> DSPCore::processFrame()
{
  const auto outGain = outputGain.process();

  constexpr double epsF32 = double(std::numeric_limits<float>::epsilon());
  if (decayGain < epsF32 || (noteStack.empty() && phaseCounter == 0)) {
    return {double(0), double(0)};
  }

  // Oscillator.
  const auto phase = oscSync * double(phaseCounter) / double(phasePeriod);
  auto sig = polynomial.evaluate(phase);

  // FM.
  auto modPhase = (fmIndex * sig + double(1)) * phase;
  modPhase -= std::floor(modPhase);
  sig = polynomial.evaluate(modPhase);

  // Envelope.
  decayGain *= decayRatio;
  const auto gain = decayGain * outGain;

  if (++phaseCounter >= phasePeriod) {
    phaseCounter = 0;
    if (scheduleUpdateNote) {
      scheduleUpdateNote = false;
      updateNote();
    }
  }

  return {gain * sig, gain * sig};
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    auto frame = processFrame();
    out0[i] = float(frame[0]);
    out1[i] = float(frame[1]);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  noteStack.push_back(info);

  if (noteStack.size() == 1 && phaseCounter == 0) {
    updateNote();
  } else {
    scheduleUpdateNote = true;
  }
}

void DSPCore::updateNote()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (noteStack.empty()) return;
  const auto &info = noteStack.back();

  velocity = velocityMap.map(info.velocity);

  auto freqHz = std::min(
    double(0.5) * sampleRate,
    double(440) * std::exp2((info.noteNumber - double(69)) / double(12)));
  phasePeriod = uint_fast32_t(sampleRate / freqHz);
  phaseCounter = 0;

  oscSync = pv[ID::oscSync]->getDouble();
  fmIndex = pv[ID::fmIndex]->getDouble();

  const auto decaySeconds = pv[ID::decaySeconds]->getDouble();
  decayRatio = std::pow(double(1e-3), double(1) / (decaySeconds * sampleRate));
  decayGain = double(1) / decayRatio;

  polynomial.updateCoefficients(true);
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
}
