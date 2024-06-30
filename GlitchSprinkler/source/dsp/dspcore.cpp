// (c) 2024 Takamitsu Endo
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
#include "tuning.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

void DSPCore::setup(double sampleRate)
{
  activeNote.reserve(1024);
  activeNote.resize(0);

  activeModifier.reserve(1024);
  activeModifier.resize(0);

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
  const auto samplesPerBeat = (double(60) * sampleRate) / tempo;                         \
  const auto arpeggioNotesPerBeat = pv[ID::arpeggioNotesPerBeat]->getInt() + 1;          \
  arpeggioDuration = int_fast32_t(samplesPerBeat / double(arpeggioNotesPerBeat));        \
  arpeggioLoopLength                                                                     \
    = (pv[ID::arpeggioLoopLengthInBeat]->getInt()) * arpeggioNotesPerBeat;               \
  if (arpeggioLoopLength == 0) {                                                         \
    arpeggioLoopLength = std::numeric_limits<decltype(arpeggioLoopLength)>::max();       \
  }                                                                                      \
                                                                                         \
  for (size_t idx = 0; idx < nPolyOscControl; ++idx) {                                   \
    polynomial.polyX[idx + 1] = pv[ID::polynomialPointX0 + idx]->getDouble();            \
    polynomial.polyY[idx + 1] = pv[ID::polynomialPointY0 + idx]->getDouble();            \
  }

void DSPCore::reset()
{
  velocity = 0;

  previousBeatsElapsed = 0;

  ASSIGN_PARAMETER(reset);

  rngParam.seed(pv[ID::seed]->getInt());
  rngArpeggio.seed(pv[ID::seed]->getInt());

  arpeggioDuration = std::numeric_limits<decltype(arpeggioDuration)>::max();
  arpeggioTie = 1;
  arpeggioTimer = 0;
  arpeggioLoopLength = std::numeric_limits<decltype(arpeggioLoopLength)>::max();
  arpeggioLoopCounter = 0;

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

  resetArpeggio();
}

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

std::array<double, 2> DSPCore::processFrame(double currentBeat)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  const auto outGain = outputGain.process();

  // Arpeggio.
  if (pv[ID::arpeggioSwitch]->getInt()) {
    if (arpeggioTimer < arpeggioTie * arpeggioDuration) {
      ++arpeggioTimer;
    } else {
      scheduleUpdateNote = true;

      arpeggioTimer = 0;

      if (++arpeggioLoopCounter >= arpeggioLoopLength) {
        arpeggioLoopCounter = 0;
        rngArpeggio.seed(pv[ID::seed]->getInt());
      }
    }
  }

  if (activeNote.empty() && phaseCounter == 0) {
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
  const auto gain = decayGain * outGain * velocity;

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

  if (previousBeatsElapsed > beatsElapsed) resetArpeggio();

  SmootherCommon<double>::setBufferSize(double(length));

  const auto beatPerSample = tempo / (double(60) * sampleRate);
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    const auto currentBeat = beatsElapsed + double(i) * beatPerSample;

    auto frame = processFrame(currentBeat);
    out0[i] = float(frame[0]);
    out1[i] = float(frame[1]);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  if (info.channel == pitchModifierChannel) {
    activeModifier.push_back(info);
    return;
  }

  activeNote.push_back(info);

  if (activeNote.size() == 1 && phaseCounter == 0) {
    arpeggioTimer = 0;
    arpeggioLoopCounter = 0;

    updateNote();
  } else if (!param.value[ParameterID::arpeggioSwitch]->getInt()) {
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

template<typename Rng, typename Scale, typename Tuning>
inline double getRandomPitch(Rng &rng, const Scale &scale, const Tuning &tuning)
{
  std::uniform_int_distribution<size_t> indexDist{0, scale.size() - 1};
  return tuning[scale[indexDist(rng)]];
}

void DSPCore::updateNote()
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (activeNote.empty()) return;

  const auto isArpeggioEnabled
    = pv[ID::arpeggioSwitch]->getInt() && arpeggioLoopCounter >= 1;
  std::uniform_int_distribution<size_t> indexDist{0, activeNote.size() - 1};
  const auto &info
    = isArpeggioEnabled ? activeNote[indexDist(rngArpeggio)] : activeNote.back();

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

  if (!activeModifier.empty()) {
    freqHz *= double(activeModifier.back().noteNumber + activeModifier.back().cent);
  }

  if (isArpeggioEnabled) {
    auto pitchDriftCent = pv[ID::arpeggioPicthDriftCent]->getDouble();
    auto octaveRange = pv[ID::arpeggioOctave]->getInt();
    using octaveType = decltype(octaveRange);
    std::uniform_int_distribution<uint32_t> octaveDist{0, octaveRange};
    std::uniform_real_distribution<double> centDist{-pitchDriftCent, pitchDriftCent};

    auto arpRatio
      = std::exp2(double(octaveDist(rngArpeggio)) + centDist(rngArpeggio) / double(1200));

    const auto arpeggioScale = pv[ID::arpeggioScale]->getInt();
    if (arpeggioScale == PitchScale::octave) {
      // Do nothing.
    } else if (arpeggioScale == PitchScale::et5Chromatic) {
      arpRatio *= getRandomPitch(rngArpeggio, scaleEt5, tuningRatioEt5);
    } else if (arpeggioScale == PitchScale::et12Major) {
      arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Major, tuningRatioEt12);
    } else if (arpeggioScale == PitchScale::et12Minor) {
      arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Minor, tuningRatioEt12);
    } else if (arpeggioScale == PitchScale::overtone32) {
      std::uniform_int_distribution<int> dist{1, 32};
      arpRatio *= double(dist(rngArpeggio));
    }

    freqHz *= arpRatio;

    auto durationVar = pv[ID::arpeggioDurationVariation]->getInt() + 1;
    std::uniform_int_distribution<decltype(durationVar)> durVarDist{1, durationVar};
    arpeggioTie = durVarDist(rngArpeggio);
  }

  phasePeriod = uint_fast32_t(sampleRate / freqHz);
  phaseCounter = 0;

  // Oscillator.
  polynomial.updateCoefficients(true);

  oscSync = pv[ID::oscSync]->getDouble();

  const auto rndFmIndexOct = pv[ID::randomizeFmIndex]->getDouble();
  std::uniform_real_distribution<double> fmIndexDist{-rndFmIndexOct, rndFmIndexOct};
  fmIndex = pv[ID::fmIndex]->getDouble() * std::exp2(fmIndexDist(rngArpeggio));
  saturationGain = pv[ID::saturationGain]->getDouble();

  // Envelope.
  const auto decaySeconds = pv[ID::decaySeconds]->getDouble();
  decayRatio = std::pow(double(1e-3), double(1) / (decaySeconds * sampleRate));

  const auto restChance = pv[ID::arpeggioRestChance]->getDouble();
  std::uniform_real_distribution<double> restDist{double(0), double(1)};
  decayGain = isArpeggioEnabled && restDist(rngParam) < restChance || freqHz == 0
    ? double(0)
    : double(1) / decayRatio;
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  { // Handling pitch modifiers.
    auto it = std::find_if(
      activeModifier.begin(), activeModifier.end(),
      [&](const NoteInfo &info) { return info.id == noteId; });
    if (it != activeModifier.end()) {
      activeModifier.erase(it);
      return;
    }
  }

  { // Handling notes.
    auto it
      = std::find_if(activeNote.begin(), activeNote.end(), [&](const NoteInfo &info) {
          return info.id == noteId;
        });
    if (it != activeNote.end()) {
      activeNote.erase(it);

      if (activeNote.empty()) resetArpeggio();
    }
  }
}

void DSPCore::resetArpeggio()
{
  rngArpeggio.seed(param.value[ParameterID::seed]->getInt());
  arpeggioTie = 0;
  arpeggioTimer = 0;
  arpeggioLoopCounter = 0;
}
