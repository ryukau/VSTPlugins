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

// `for (let i = 0; i < 12; ++i) console.log(2**(i / 12));`
std::array<double, 12> tuningRatioEt12 = {
  double(1.00000000000000),   double(1.0594630943592953), double(1.122462048309373),
  double(1.189207115002721),  double(1.2599210498948732), double(1.3348398541700344),
  double(1.4142135623730951), double(1.4983070768766815), double(1.5874010519681994),
  double(1.681792830507429),  double(1.7817974362806785), double(1.887748625363387),
};

// ```javascript
// var cents = [0, 20, 240, 260, 360, 480, 500, 720, 740, 960, 980, 1180];
// console.log(cents.map(v => 2**(v / 1200)));
// ```
std::array<double, 12> tuningRatioEt5 = {
  double(1.0000000000000),    double(1.0116194403019225), double(1.148698354997035),
  double(1.1620455869578397), double(1.2311444133449163), double(1.3195079107728942),
  double(1.3348398541700344), double(1.515716566510398),  double(1.5333283446696007),
  double(1.7411011265922482), double(1.761331747192297),  double(1.9770280407057923),
};

// cents = [0, 120, 240, 300, 360, 480, 600, 720, 840, 960, 1020, 1080];
std::array<double, 12> tuningRatioEt10Minor = {
  double(1.0000000000000),    double(1.0717734625362931), double(1.148698354997035),
  double(1.189207115002721),  double(1.2311444133449163), double(1.3195079107728942),
  double(1.4142135623730951), double(1.515716566510398),  double(1.624504792712471),
  double(1.7411011265922482), double(1.8025009252216604), double(1.8660659830736148),
};

std::array<double, 12> tuningRatioJust5Major = {
  double(1) / double(1),   double(16) / double(15), double(9) / double(8),
  double(6) / double(5),   double(5) / double(4),   double(4) / double(3),
  double(45) / double(32), double(3) / double(2),   double(8) / double(5),
  double(5) / double(3),   double(16) / double(9),  double(15) / double(8),
};

std::array<double, 12> tuningRatioJust5Minor = {
  double(1) / double(1),   double(16) / double(15), double(10) / double(9),
  double(6) / double(5),   double(5) / double(4),   double(4) / double(3),
  double(64) / double(45), double(3) / double(2),   double(8) / double(5),
  double(5) / double(3),   double(9) / double(5),   double(15) / double(8),
};

// TODO: remove 1 element if possible.
std::array<double, 13> tuningRatioJust7 = {
  double(1) / double(1), double(8) / double(7),  double(7) / double(6),
  double(6) / double(5), double(5) / double(4),  double(4) / double(3),
  double(7) / double(5), double(10) / double(7), double(3) / double(2),
  double(8) / double(5), double(5) / double(3),  double(12) / double(7),
  double(7) / double(4),
};

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

  scheduleUpdateNote = false;
  phaseCounter = 0;
  decayGain = double(0);
  polynomial.updateCoefficients(true);

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

  // Saturation.
  sig = std::clamp(saturationGain * sig, double(-1), double(1));

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

  constexpr double epsF32 = double(std::numeric_limits<float>::epsilon());
  if (decayGain < epsF32 || (noteStack.size() == 1 && phaseCounter == 0)) {
    updateNote();
  } else {
    scheduleUpdateNote = true;
  }
}

inline double getPitchRatio(Tuning tuning, int semitone, double octave, double cent)
{
  auto getTuning = [&](Tuning tuning) {
    switch (tuning) {
      default:
      case Tuning::equalTemperament12:
        return tuningRatioEt12;
      case Tuning::equalTemperament5:
        return tuningRatioEt5;
      case Tuning::justIntonation5LimitMajor:
        return tuningRatioJust5Major;
    }
  };
  auto tuningTable = getTuning(tuning);

  int size = int(tuningTable.size());
  int index = semitone % size;
  if (index < 0) index += size;

  octave += double((semitone - index) / size);

  return tuningTable[index] * std::exp2(octave + cent / double(1200));
}

void DSPCore::updateNote()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (noteStack.empty()) return;
  const auto &info = noteStack.back();

  velocity = velocityMap.map(info.velocity);

  // Pitch & phase.
  auto transposeOctave = int(pv[ID::transposeOctave]->getInt()) - transposeOctaveOffset;
  auto transposeSemitone
    = int(pv[ID::transposeSemitone]->getInt()) - transposeSemitoneOffset;
  auto transposeCent = pv[ID::transposeCent]->getDouble();
  auto transposeRatio = getPitchRatio(
    static_cast<Tuning>(pv[ID::tuning]->getInt()),
    transposeSemitone + info.noteNumber - 69, transposeOctave, transposeCent);
  auto freqHz = std::min(double(0.5) * sampleRate, transposeRatio * double(440));

  phasePeriod = uint_fast32_t(sampleRate / freqHz);
  phaseCounter = 0;

  // Oscillator.
  polynomial.updateCoefficients(true);

  oscSync = pv[ID::oscSync]->getDouble();
  fmIndex = pv[ID::fmIndex]->getDouble();
  saturationGain = pv[ID::saturationGain]->getDouble();

  // Envelope.
  const auto decaySeconds = pv[ID::decaySeconds]->getDouble();
  decayRatio = std::pow(double(1e-3), double(1) / (decaySeconds * sampleRate));
  decayGain = double(1) / decayRatio;
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
