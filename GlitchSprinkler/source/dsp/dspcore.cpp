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

static constexpr bool isPolyphonic__ = true; // TODO
static constexpr bool doRelease = true;      // TODO

void DSPCore::setup(double sampleRate)
{
  activeNote.reserve(1024);
  activeNote.resize(0);

  activeModifier.reserve(1024);
  activeModifier.resize(0);

  sampleRate = sampleRate;

  SmootherCommon<double>::setTime(double(0.2));

  reset();
  startup();
}

#define ASSIGN_PARAMETER_CORE(METHOD)                                                    \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  isPolyphonic = isPolyphonic__;                                                         \
  noteOffState = doRelease ? Voice::State::release : Voice::State::terminate;            \
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
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
                                                                                         \
  for (size_t idx = 0; idx < nPolyOscControl; ++idx) {                                   \
    polynomial.polyX[idx + 1] = pv[ID::polynomialPointX0 + idx]->getDouble();            \
    polynomial.polyY[idx + 1] = pv[ID::polynomialPointY0 + idx]->getDouble();            \
  }

#define ASSIGN_PARAMETER_VOICE(METHOD)                                                   \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = core.param.value;

void Voice::reset()
{
  ASSIGN_PARAMETER_VOICE(reset);

  state = State::rest;

  noteId = -1;
  noteNumber = 0;
  noteCent = 0;
  noteVelocity = 0;

  rngArpeggio.seed(pv[ID::seed]->getInt());

  arpeggioTie = 1;
  arpeggioTimer = 0;
  arpeggioLoopCounter = 0;

  scheduleUpdateNote = false;
  phasePeriod = 0;
  phaseCounter = 0;
  oscSync = double(1);
  fmIndex = double(0);
  saturationGain = double(1);
  decayGain = double(0);
  decayRatio = double(1);
  polynomialCoefficients.fill({});
}

void DSPCore::reset()
{
  ASSIGN_PARAMETER_CORE(reset);

  previousBeatsElapsed = 0;

  polynomial.updateCoefficients(true);
  isPolynomialUpdated = true;

  for (auto &x : voices) x.reset();

  startup();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  for (auto &x : voices) x.resetArpeggio(pv[ID::seed]->getInt());
}

void Voice::setParameters() { ASSIGN_PARAMETER_VOICE(push); }

void DSPCore::setParameters()
{
  ASSIGN_PARAMETER_CORE(push);

  for (auto &x : voices) x.setParameters();
}

std::array<double, 2> Voice::processFrame()
{
  using ID = ParameterID::ID;
  const auto &pv = core.param.value;

  // Arpeggio.
  if (pv[ID::arpeggioSwitch]->getInt()) {
    if (arpeggioTimer < arpeggioTie * core.arpeggioDuration) {
      ++arpeggioTimer;
    } else if (state != State::release && state != State::terminate) {
      scheduleUpdateNote = true;

      arpeggioTimer = 0;

      if (++arpeggioLoopCounter >= core.arpeggioLoopLength) {
        arpeggioLoopCounter = 0;
        rngArpeggio.seed(pv[ID::seed]->getInt());
      }
    }
  }

  constexpr double epsf32 = double(std::numeric_limits<float>::epsilon());
  if (
    (state == State::terminate && phaseCounter == 0)
    || (state == State::release && lastGain <= epsf32))
  {
    state = State::rest;
    return {double(0), double(0)};
  }

  // Oscillator.
  auto oscFunc = [&](double phase) {
    return computePolynomial<double, PolySolver::nPolynomialPoint>(
      phase, polynomialCoefficients);
  };

  const auto phase = oscSync * double(phaseCounter) / double(phasePeriod);
  auto sig = oscFunc(phase);

  // FM.
  auto modPhase = (fmIndex * sig + double(1)) * phase;
  modPhase -= std::floor(modPhase);
  sig = oscFunc(modPhase);

  // Saturation.
  sig = std::clamp(saturationGain * sig, double(-1), double(1));

  // Envelope.
  decayGain *= decayRatio;
  lastGain = decayGain * noteVelocity;
  sig *= lastGain;

  if (++phaseCounter >= phasePeriod) {
    phaseCounter = 0;
    if (scheduleUpdateNote) {
      scheduleUpdateNote = false;
      if (state != State::release && state != State::terminate) updateNote();
    }
  }

  return {sig, sig};
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  if (previousBeatsElapsed > beatsElapsed) {
    for (auto &x : voices) x.resetArpeggio(pv[ID::seed]->getInt());
  }

  isPolynomialUpdated = false;

  SmootherCommon<double>::setBufferSize(double(length));

  const auto beatPerSample = tempo / (double(60) * sampleRate);
  std::array<double, 2> frame{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    currentBeat = beatsElapsed + double(i) * beatPerSample;
    pitchModifier = activeModifier.empty()
      ? double(1)
      : double(activeModifier.back().noteNumber + activeModifier.back().cent);

    frame.fill({});

    for (auto &vc : voices) {
      if (vc.state == Voice::State::rest) continue;
      auto voiceOut = vc.processFrame();
      frame[0] += voiceOut[0];
      frame[1] += voiceOut[1];
    }

    const auto outGain = outputGain.process();
    out0[i] = float(outGain * frame[0]);
    out1[i] = float(outGain * frame[1]);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  if (info.channel == pitchModifierChannel) {
    activeModifier.push_back(info);
    return;
  }

  activeNote.push_back(info);

  size_t replaceIndex = 0;
  if (isPolyphonic) {
    double mostQuiet = std::numeric_limits<double>::max();
    for (size_t idx = 0; idx < voices.size(); ++idx) {
      if (voices[idx].state == Voice::State::rest) {
        replaceIndex = idx;
        break;
      }
      if (mostQuiet > voices[idx].lastGain) {
        mostQuiet = voices[idx].lastGain;
        replaceIndex = idx;
      }
    }
    voices[replaceIndex].noteId = info.id;
    voices[replaceIndex].noteNumber = info.noteNumber;
    voices[replaceIndex].noteCent = info.cent;
    voices[replaceIndex].noteVelocity = velocityMap.map(info.velocity);
    voices[replaceIndex].state = Voice::State::active;
  } else {
    voices[replaceIndex].noteId = -1;
    voices[replaceIndex].state = Voice::State::active;
  }

  auto &newVoice = voices[replaceIndex];
  if (newVoice.state == Voice::State::rest && newVoice.phaseCounter == 0) {
    newVoice.arpeggioTimer = 0;
    newVoice.arpeggioLoopCounter = 0;

    newVoice.updateNote();
  } else if (!param.value[ParameterID::arpeggioSwitch]->getInt()) {
    newVoice.scheduleUpdateNote = true;
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

void Voice::updateNote()
{
  using ID = ParameterID::ID;
  auto &pv = core.param.value;

  if (core.activeNote.empty()) return;

  const auto useArpeggiator
    = pv[ID::arpeggioSwitch]->getInt() && arpeggioLoopCounter >= 1;

  if (!core.isPolyphonic) {
    std::uniform_int_distribution<size_t> indexDist{0, core.activeNote.size() - 1};
    const auto &note
      = useArpeggiator ? core.activeNote[indexDist(rngArpeggio)] : core.activeNote.back();

    noteVelocity = core.velocityMap.map(note.velocity);
    noteNumber = note.noteNumber;
    noteCent = note.cent;
  }

  // Pitch & phase.
  auto transposeOctave = int(pv[ID::transposeOctave]->getInt()) - transposeOctaveOffset;
  auto transposeSemitone
    = int(pv[ID::transposeSemitone]->getInt()) - transposeSemitoneOffset;
  auto transposeCent = pv[ID::transposeCent]->getDouble();
  auto transposeRatio = getPitchRatio(
    static_cast<Tuning>(pv[ID::tuning]->getInt()), transposeSemitone + noteNumber - 69,
    transposeOctave, transposeCent + noteCent);
  auto freqHz = core.pitchModifier * transposeRatio * double(440);

  if (useArpeggiator) {
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

  freqHz = std::min(double(0.5) * core.sampleRate, freqHz);
  phasePeriod = uint_fast32_t(core.sampleRate / freqHz);
  phaseCounter = 0;

  // Oscillator.
  if (!core.isPolynomialUpdated) {
    core.polynomial.updateCoefficients(true);
    core.isPolynomialUpdated = true;
  }
  polynomialCoefficients = core.polynomial.coefficients;

  oscSync = pv[ID::oscSync]->getDouble();

  const auto rndFmIndexOct = pv[ID::randomizeFmIndex]->getDouble();
  std::uniform_real_distribution<double> fmIndexDist{-rndFmIndexOct, rndFmIndexOct};
  fmIndex = pv[ID::fmIndex]->getDouble() * std::exp2(fmIndexDist(rngArpeggio));
  saturationGain = pv[ID::saturationGain]->getDouble();

  // Envelope.
  const auto decaySeconds = pv[ID::decaySeconds]->getDouble();
  decayRatio = std::pow(double(1e-3), double(1) / (decaySeconds * core.sampleRate));

  const auto restChance = pv[ID::arpeggioRestChance]->getDouble();
  std::uniform_real_distribution<double> restDist{double(0), double(1)};
  decayGain = useArpeggiator && restDist(rngArpeggio) < restChance || freqHz == 0
    ? double(0)
    : double(1) / decayRatio;
  lastGain = decayGain;
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
    auto itNote
      = std::find_if(activeNote.begin(), activeNote.end(), [&](const NoteInfo &info) {
          return info.id == noteId;
        });
    if (itNote == activeNote.end()) return;
    activeNote.erase(itNote);

    if (isPolyphonic) {
      auto itVoice = std::find_if(voices.begin(), voices.end(), [&](const Voice &vc) {
        return vc.noteId == noteId;
      });
      if (itVoice != voices.end()) {
        auto &voice = *itVoice;
        voice.state = noteOffState;
        voice.resetArpeggio(pv[ID::seed]->getInt());
      }
    } else if (activeNote.empty()) {
      voices[0].state = noteOffState;
      voices[0].resetArpeggio(pv[ID::seed]->getInt());
    }
  }
}

void Voice::resetArpeggio(unsigned seed)
{
  rngArpeggio.seed(seed);
  arpeggioTie = 0;
  arpeggioTimer = 0;
  arpeggioLoopCounter = 0;
}
