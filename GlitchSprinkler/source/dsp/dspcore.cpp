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

void DSPCore::setup(double sampleRate_)
{
  activeNote.reserve(1024);
  activeNote.resize(0);

  activeModifier.reserve(1024);
  activeModifier.resize(0);

  sampleRate = sampleRate_;

  SmootherCommon<double>::setTime(double(0.2));

  terminationLength = int_fast32_t(double(0.002) * sampleRate);
  lowpassInterpRate = sampleRate / double(48000 * 64);
  softAttackEmaRatio = EMAFilter<double>::cutoffToP(double(50) / sampleRate);

  for (auto &x : safetyFilter) x.setup(sampleRate);

  reset();
  startup();
}

#define ASSIGN_PARAMETER_CORE(METHOD)                                                    \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  const auto newNoteOffState                                                             \
    = pv[ID::release]->getInt() ? Voice::State::release : Voice::State::terminate;       \
  if (noteOffState != newNoteOffState) {                                                 \
    for (auto &x : voices) {                                                             \
      if (x.state == noteOffState) x.state = newNoteOffState;                            \
    }                                                                                    \
  }                                                                                      \
  noteOffState = newNoteOffState;                                                        \
                                                                                         \
  const auto newIsPolyphonic = pv[ID::polyphonic]->getInt() != 0;                        \
  if (isPolyphonic != newIsPolyphonic) {                                                 \
    for (size_t idx = 1; idx < voices.size(); ++idx) {                                   \
      voices[idx].state = noteOffState;                                                  \
    }                                                                                    \
  }                                                                                      \
  isPolyphonic = newIsPolyphonic;                                                        \
                                                                                         \
  const auto samplesPerBeat = (double(60) * sampleRate) / tempo;                         \
  const auto arpeggioNotesPerBeat = pv[ID::arpeggioNotesPerBeat]->getInt() + 1;          \
  arpeggioNoteDuration = int_fast32_t(samplesPerBeat / double(arpeggioNotesPerBeat));    \
  arpeggioLoopLength                                                                     \
    = (pv[ID::arpeggioLoopLengthInBeat]->getInt()) * arpeggioNotesPerBeat;               \
  if (arpeggioLoopLength == 0) {                                                         \
    arpeggioLoopLength = std::numeric_limits<decltype(arpeggioLoopLength)>::max();       \
  }                                                                                      \
                                                                                         \
  arpeggioSwitch = pv[ID::arpeggioSwitch]->getInt();                                     \
  filterSwitch = pv[ID::filterSwitch]->getInt();                                         \
  softEnvelopeSwitch = pv[ID::softEnvelopeSwitch]->getInt();                             \
  pwmSwitch = pv[ID::pulseWidthModulation]->getInt();                                    \
  pwmBidrection = pv[ID::pulseWidthModBidirectionSwitch]->getInt();                      \
  bitmaskSwitch = pv[ID::pulseWidthBitwiseAnd]->getInt();                                \
  pwmRatio = double(1) - pv[ID::pulseWidthRatio]->getDouble();                           \
                                                                                         \
  const auto pulseWidthModRate = pv[ID::pulseWidthModRate]->getInt();                    \
  if (pulseWidthModRate < pwmOneCyclePoint) {                                            \
    pwmChangeCycle = 1;                                                                  \
    pwmAmount = pwmOneCyclePoint - pulseWidthModRate + 1;                                \
  } else {                                                                               \
    pwmChangeCycle = 1 + pulseWidthModRate - pwmOneCyclePoint;                           \
    pwmAmount = 1;                                                                       \
  }                                                                                      \
                                                                                         \
  safetyFilterMix.METHOD(pv[ID::safetyFilterMix]->getDouble());                          \
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

  unisonIndex = 1;
  unisonRatio = double(0);
  unisonPan = double(0.5);
  unisonGain = double(1);

  rngArpeggio.seed(pv[ID::seed]->getInt() + unisonIndex);

  arpeggioTie = 1;
  arpeggioTimer = 0;
  arpeggioLoopCounter = 0;
  terminationCounter = 0;

  scheduleUpdateNote = false;
  pwmLower = 0;
  pwmPoint = 0;
  pwmBitMask = 0;
  pwmDirection = 1;
  pwmChangeCounter = 1;
  phasePeriod = 0;
  phaseCounter = 0;
  oscSync = double(1);
  fmIndex = double(0);
  saturationGain = double(1);
  decayRatio = double(1);
  decayGain = double(0);
  decayEmaRatio = double(1);
  decayEmaValue = double(0);
  polynomialCoefficients.fill({});

  filterDecayRatio = double(1);
  filterDecayGain = double(0);
  cutoffBase = double(1);
  cutoffMod = double(0);
  resonanceBase = double(0);
  resonanceMod = double(0);
  notchBase = double(1);
  notchMod = double(0);
  lowpass.reset();
}

void DSPCore::reset()
{
  ASSIGN_PARAMETER_CORE(reset);

  previousBeatsElapsed = 0;

  currentBeat = 0;
  pitchModifier = double(1);

  polynomial.updateCoefficients(true);
  isPolynomialUpdated = true;

  nextSteal = 0;
  for (auto &x : voices) x.reset();

  for (auto &x : safetyFilter) x.reset();

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
  if (core.arpeggioSwitch) {
    if (arpeggioTimer < arpeggioTie * core.arpeggioNoteDuration) {
      ++arpeggioTimer;
    } else if (state == State::active) {
      scheduleUpdateNote = true;

      arpeggioTimer = 0;

      if (++arpeggioLoopCounter >= core.arpeggioLoopLength) {
        arpeggioLoopCounter = 0;
        rngArpeggio.seed(pv[ID::seed]->getInt() + unisonIndex);
      }
    }
  }

  constexpr double epsf32 = double(std::numeric_limits<float>::epsilon());
  if (
    (state == State::terminate && phaseCounter == 0)
    || (state == State::release && lastGain <= epsf32) || (terminationCounter > 0))
  {
    if (
      terminationCounter < core.terminationLength
      && (core.filterSwitch || core.softEnvelopeSwitch))
    {
      ++terminationCounter;
    } else {
      terminationCounter = core.terminationLength;
      state = State::rest;
      return {double(0), double(0)};
    }
  }

  // Oscillator.
  auto oscFunc = [&](double phase) {
    if (phaseCounter > (pwmPoint | ~pwmBitMask)) return double(0);
    return computePolynomial<double, PolySolver::nPolynomialPoint>(
      phase, polynomialCoefficients);
  };

  const auto phase = oscSync * double(phaseCounter & pwmBitMask) / double(phasePeriod);
  auto sig = oscFunc(phase);

  // FM.
  auto modPhase = (fmIndex * sig + double(1)) * phase;
  modPhase -= std::floor(modPhase);
  sig = oscFunc(modPhase);

  // Saturation.
  sig = std::clamp(saturationGain * sig, double(-1), double(1));

  // Lowpass.
  if (core.filterSwitch) {
    sig = lowpass.process(
      sig, core.lowpassInterpRate, cutoffBase + cutoffMod * filterDecayRatio,
      resonanceBase + resonanceMod * filterDecayRatio,
      notchBase + notchMod * filterDecayRatio);
    filterDecayRatio *= filterDecayGain;
  }

  // Envelope.
  decayGain *= decayRatio;
  decayEmaValue += decayEmaRatio * (decayGain - decayEmaValue);
  const auto terminationDecay = double(core.terminationLength - terminationCounter)
    / double(core.terminationLength);
  lastGain = decayEmaValue * noteVelocity * terminationDecay;
  sig *= lastGain;

  if (++phaseCounter >= phasePeriod) {
    phaseCounter = 0;

    if (pwmChangeCounter <= 0) {
      pwmChangeCounter = core.pwmChangeCycle;

      if (core.pwmSwitch) {
        pwmPoint = std::clamp(
          pwmPoint + pwmDirection, int_fast32_t(0), int_fast32_t(phasePeriod));

        if (pwmPoint <= pwmLower) {
          if (core.pwmBidrection) {
            pwmDirection = core.pwmAmount;
          } else {
            pwmPoint = phasePeriod;
          }
        } else if (pwmPoint >= phasePeriod) {
          pwmDirection = -core.pwmAmount;
        }
      } else {
        pwmPoint = int_fast32_t(phasePeriod * core.pwmRatio);
      }

      pwmBitMask
        = core.bitmaskSwitch ? pwmPoint : std::numeric_limits<uint_fast32_t>::max();
    } else {
      --pwmChangeCounter;
    }

    if (scheduleUpdateNote) {
      scheduleUpdateNote = false;
      if (state == State::active) updateNote();
    }
  }

  return {(double(1) - unisonPan) * sig, unisonPan * sig};
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

    const auto safetyFiltMix = safetyFilterMix.process();
    frame[0] = std::lerp(frame[0], safetyFilter[0].process(frame[0]), safetyFiltMix);
    frame[1] = std::lerp(frame[1], safetyFilter[1].process(frame[1]), safetyFiltMix);

    const auto outGain = outputGain.process();
    out0[i] = float(outGain * frame[0]);
    out1[i] = float(outGain * frame[1]);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  if (info.channel == pitchModifierChannel) {
    activeModifier.push_back(info);
    return;
  }
  activeNote.push_back(info);

  const size_t nUnison = 1 + pv[ID::unisonVoice]->getInt();
  noteIndices.resize(nUnison);
  if (isPolyphonic) {
    for (auto &idx : noteIndices) {
      idx = nextSteal;
      if (++nextSteal >= voices.size()) nextSteal = 0;
    }
  } else {
    std::iota(noteIndices.begin(), noteIndices.end(), size_t(0));
  }

  for (unsigned idx = 0; idx < noteIndices.size(); ++idx) {
    auto &vc = voices[noteIndices[idx]];

    if (isPolyphonic) {
      vc.noteId = info.id;
      vc.noteNumber = info.noteNumber;
      vc.noteCent = info.cent;
      vc.noteVelocity = velocityMap.map(info.velocity);
    } else {
      vc.noteId = -1;
    }

    if (nUnison <= 1) {
      vc.unisonIndex = 0;
      vc.unisonRatio = double(0);
    } else {
      auto scatterArp = pv[ID::unisonScatterArpeggio]->getInt();
      vc.unisonIndex = scatterArp ? idx + 1 : 1;
      vc.unisonRatio = double(idx) / double(nUnison - 1);
    }

    vc.unisonGain = pv[ID::unisonGainSqrt]->getInt()
      ? std::sqrt(double(1) / double(nUnison))
      : double(1) / double(nUnison);
    vc.rngArpeggio.seed(pv[ID::seed]->getInt() + vc.unisonIndex);

    if (vc.state == Voice::State::rest) {
      vc.arpeggioTimer = 0;
      vc.arpeggioLoopCounter = 0;
      vc.pwmChangeCounter = pwmChangeCycle;
      vc.decayEmaRatio = softEnvelopeSwitch ? softAttackEmaRatio : double(1);
      vc.decayEmaValue = 0;
      vc.lowpass.reset();
      vc.updateNote();
    } else if (!param.value[ParameterID::arpeggioSwitch]->getInt()) {
      vc.scheduleUpdateNote = true;
    }

    vc.terminationCounter = 0;
    vc.state = Voice::State::active;
  }
}

template<size_t length>
double getPitchRatioFromArray(
  int semitone,
  int rootShift,
  double octave,
  double cent,
  const std::array<double, length> &tuningTable)
{
  int size = int(tuningTable.size());

  int rootIndex = rootShift % size;
  if (rootIndex < 0) rootIndex += size;

  semitone += rootIndex;
  int stIndex = semitone % size;
  if (stIndex < 0) stIndex += size;

  octave += double((semitone - stIndex) / size);

  return std::exp2(octave + cent / double(1200)) * tuningTable[stIndex]
    / tuningTable[rootIndex];
}

inline double
getPitchRatio(int semitone, int rootShift, double octave, double cent, Tuning tuningIndex)
{
  switch (tuningIndex) {
    default:
    case Tuning::et12:
      return getPitchRatioFromArray(semitone, rootShift, octave, cent, tuningRatioEt12);
    case Tuning::et5:
      return getPitchRatioFromArray(semitone, rootShift, octave, cent, tuningRatioEt5);
    case Tuning::et10:
      return getPitchRatioFromArray(semitone, rootShift, octave, cent, tuningRatioEt10);
    case Tuning::just5Major:
      return getPitchRatioFromArray(
        semitone, rootShift, octave, cent, tuningRatioJust5Major);
    case Tuning::just5Minor:
      return getPitchRatioFromArray(
        semitone, rootShift, octave, cent, tuningRatioJust5Minor);
    case Tuning::just7:
      return getPitchRatioFromArray(semitone, rootShift, octave, cent, tuningRatioJust7);
    case Tuning::mtPythagorean:
      return getPitchRatioFromArray(
        semitone, rootShift, octave, cent, tuningRatioMtPythagorean);
    case Tuning::mtThirdComma:
      return getPitchRatioFromArray(
        semitone, rootShift, octave, cent, tuningRatioMtThirdComma);
    case Tuning::mtQuarterComma:
      return getPitchRatioFromArray(
        semitone, rootShift, octave, cent, tuningRatioMtQuarterComma);
  }
  // Shouldn't reach here.
}

template<typename Rng, typename Scale, typename Tuning>
inline double getRandomPitchFixed(Rng &rng, const Scale &scale, const Tuning &tuningTable)
{
  std::uniform_int_distribution<size_t> indexDist{0, scale.size() - 1};
  const size_t index = scale[indexDist(rng)];
  const size_t octave = size_t(1) << (index / tuningTable.size());
  return octave * tuningTable[index % tuningTable.size()];
}

template<typename Rng, typename Scale>
inline double getRandomPitch(Rng &rng, const Scale &scale, const Tuning tuningIndex)
{
  switch (tuningIndex) {
    default:
    case Tuning::et12:
      return getRandomPitchFixed(rng, scale, tuningRatioEt12);
    case Tuning::just5Major:
      return getRandomPitchFixed(rng, scale, tuningRatioJust5Major);
    case Tuning::just5Minor:
      return getRandomPitchFixed(rng, scale, tuningRatioJust5Minor);
    case Tuning::just7:
      return getRandomPitchFixed(rng, scale, tuningRatioJust7);
    case Tuning::mtPythagorean:
      return getRandomPitchFixed(rng, scale, tuningRatioMtPythagorean);
    case Tuning::mtThirdComma:
      return getRandomPitchFixed(rng, scale, tuningRatioMtThirdComma);
    case Tuning::mtQuarterComma:
      return getRandomPitchFixed(rng, scale, tuningRatioMtQuarterComma);
  }
  // Shouldn't reach here.
}

void Voice::updateNote()
{
  using ID = ParameterID::ID;
  auto &pv = core.param.value;

  if (core.activeNote.empty()) return;

  const auto useArpeggiator = core.arpeggioSwitch;

  if (!core.isPolyphonic) {
    std::uniform_int_distribution<size_t> indexDist{0, core.activeNote.size() - 1};
    const auto &note
      = useArpeggiator ? core.activeNote[indexDist(rngArpeggio)] : core.activeNote.back();

    noteVelocity = core.velocityMap.map(note.velocity);
    noteNumber = note.noteNumber;
    noteCent = note.cent;
  }

  if (unisonIndex <= 0) {
    unisonPan = double(0.5);
  } else {
    const auto unisonPanSpread = pv[ID::unisonPanSpread]->getDouble();
    const auto unisonPanOffset = (double(1) - unisonPanSpread) / double(2);
    unisonPan = unisonRatio * unisonPanSpread + unisonPanOffset;
  }

  // Pitch & phase.
  const auto tuning = static_cast<Tuning>(pv[ID::tuningType]->getInt());
  const auto transposeOctave
    = int(pv[ID::transposeOctave]->getInt()) - transposeOctaveOffset;
  const auto transposeSemitone
    = int(pv[ID::transposeSemitone]->getInt()) - transposeSemitoneOffset;
  const auto transposeCent = pv[ID::transposeCent]->getDouble();
  const auto unisonDetuneCent = unisonRatio * pv[ID::unisonDetuneCent]->getDouble();
  const auto tuningRootSemitone = int(pv[ID::tuningRootSemitone]->getInt());

  auto getNaturalFreq = [&]() {
    auto transposeRatio = getPitchRatio(
      transposeSemitone + noteNumber - 69, tuningRootSemitone, transposeOctave,
      transposeCent + noteCent + unisonDetuneCent, tuning);
    return core.pitchModifier * transposeRatio * double(440);
  };
  auto getDiscreteFreq = [&]() {
    auto semitones = double(transposeSemitone + noteNumber - 127) / double(12);
    auto cents = (transposeCent + noteCent + unisonDetuneCent) / double(1200);
    auto transposeRatio = std::exp2(transposeOctave + semitones + cents);
    auto freqHz = core.pitchModifier * transposeRatio * core.sampleRate;
    switch (tuning) {
      default:
      case Tuning::discrete2:
        return freqHz / double(2);
      case Tuning::discrete3:
        return freqHz / double(3);
      case Tuning::discrete5:
        return freqHz / double(5);
      case Tuning::discrete7:
        return freqHz / double(7);
    }
  };

  auto freqHz = tuning >= Tuning::discrete2 ? getDiscreteFreq() : getNaturalFreq();

  if (useArpeggiator) {
    auto pitchDriftCent = pv[ID::arpeggioPicthDriftCent]->getDouble();
    auto octaveRange = pv[ID::arpeggioOctave]->getInt();
    using octaveType = decltype(octaveRange);
    std::uniform_int_distribution<uint32_t> octaveDist{0, octaveRange};
    std::uniform_real_distribution<double> centDist{-pitchDriftCent, pitchDriftCent};

    auto arpRatio
      = std::exp2(double(octaveDist(rngArpeggio)) + centDist(rngArpeggio) / double(1200));

    const auto arpeggioStartFromRoot = bool(pv[ID::arpeggioStartFromRoot]->getInt());
    if (!(arpeggioStartFromRoot && arpeggioTimer == 0 && arpeggioLoopCounter == 0)) {
      const auto arpeggioScale = pv[ID::arpeggioScale]->getInt();
      if (arpeggioScale == PitchScale::et5Chromatic) {
        arpRatio *= getRandomPitchFixed(rngArpeggio, scaleEt5, tuningRatioEt5);
      } else if (arpeggioScale == PitchScale::et12ChurchC) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchC, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchD) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchD, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchE) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchE, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchF) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchF, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchG) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchG, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchA) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchA, tuning);
      } else if (arpeggioScale == PitchScale::et12ChurchB) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12ChurchB, tuning);
      } else if (arpeggioScale == PitchScale::et12Sus2) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Sus2, tuning);
      } else if (arpeggioScale == PitchScale::et12Sus4) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Sus4, tuning);
      } else if (arpeggioScale == PitchScale::et12Maj7) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Maj7, tuning);
      } else if (arpeggioScale == PitchScale::et12Min7) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Min7, tuning);
      } else if (arpeggioScale == PitchScale::et12MajExtended) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12MajExtended, tuning);
      } else if (arpeggioScale == PitchScale::et12MinExtended) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12MinExtended, tuning);
      } else if (arpeggioScale == PitchScale::et12WholeTone2) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12WholeTone2, tuning);
      } else if (arpeggioScale == PitchScale::et12WholeTone3) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12WholeTone3, tuning);
      } else if (arpeggioScale == PitchScale::et12WholeTone4) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12WholeTone4, tuning);
      } else if (arpeggioScale == PitchScale::et12Blues) {
        arpRatio *= getRandomPitch(rngArpeggio, scaleEt12Blues, tuning);
      } else if (arpeggioScale == PitchScale::overtone32) {
        std::uniform_int_distribution<int> dist{1, 32};
        arpRatio *= double(dist(rngArpeggio));
      } else if (arpeggioScale == PitchScale::overtone42Melody) {
        constexpr std::array<double, 7> melody{2, 8, 10, 32, 34, 40, 42};
        std::uniform_int_distribution<size_t> dist{0, melody.size() - 1};
        arpRatio *= melody[dist(rngArpeggio)];
      } else if (arpeggioScale == PitchScale::overtoneOdd16) {
        constexpr std::array<double, 8> melody{1, 3, 5, 7, 9, 11, 13, 15};
        std::uniform_int_distribution<size_t> dist{0, melody.size() - 1};
        arpRatio *= melody[dist(rngArpeggio)];
      }
      // Defaults to `arpeggioScale == PitchScale::octave`.
    }

    freqHz *= arpRatio;

    auto durationVar = pv[ID::arpeggioDurationVariation]->getInt() + 1;
    std::uniform_int_distribution<decltype(durationVar)> durVarDist{1, durationVar};
    arpeggioTie = durVarDist(rngArpeggio);
  } else {
    arpeggioTie = 1;
  }

  freqHz = std::min(double(0.5) * core.sampleRate, freqHz);
  phasePeriod = uint_fast32_t(std::ceil(core.sampleRate / freqHz));
  phaseCounter = 0;

  // Oscillator.
  if (!core.isPolynomialUpdated) {
    core.polynomial.updateCoefficients(true);
    core.isPolynomialUpdated = true;
  }
  polynomialCoefficients = core.polynomial.coefficients;

  pwmLower = int_fast32_t(phasePeriod * core.pwmRatio);
  if (
    state == State::rest || core.pwmRatio >= 1
    || pv[ID::arpeggioResetModulation]->getInt())
  {
    pwmPoint = core.pwmSwitch ? phasePeriod : pwmLower;
    pwmDirection = core.pwmAmount;

    pwmBitMask
      = core.bitmaskSwitch ? pwmPoint : std::numeric_limits<uint_fast32_t>::max();
  }

  oscSync = pv[ID::oscSync]->getDouble();

  const auto rndFmIndexOct = pv[ID::randomizeFmIndex]->getDouble();
  std::uniform_real_distribution<double> fmIndexDist{-rndFmIndexOct, rndFmIndexOct};
  fmIndex = pv[ID::fmIndex]->getDouble() * std::exp2(fmIndexDist(rngArpeggio));
  saturationGain = pv[ID::saturationGain]->getDouble();

  // Envelope.
  const auto decayTargetGain = pv[ID::decayTargetGain]->getDouble();
  decayRatio
    = std::pow(decayTargetGain, double(1) / (arpeggioTie * core.arpeggioNoteDuration));

  const auto restChance = arpeggioTimer == 0 && arpeggioLoopCounter == 0
    ? double(0)
    : pv[ID::arpeggioRestChance]->getDouble();
  std::uniform_real_distribution<double> restDist{double(0), double(1)};
  decayGain = useArpeggiator && restDist(rngArpeggio) < restChance || freqHz == 0
    ? double(0)
    : unisonGain / decayRatio;
  lastGain = decayGain;

  // Filter.
  if (core.filterSwitch) {
    const auto cutoffBaseOctave = pv[ID::filterCutoffBaseOctave]->getDouble();
    const auto cutoffKeyFollow = pv[ID::filterCutoffKeyFollow]->getDouble();
    const auto baseFreq
      = std::lerp(double(20), freqHz, cutoffKeyFollow) / core.sampleRate;
    cutoffBase
      = std::clamp(baseFreq * std::exp2(cutoffBaseOctave), double(0), double(0.4999));

    const auto cutoffModOctave = pv[ID::filterCutoffModOctave]->getDouble();
    cutoffMod = cutoffModOctave <= Scales::filterCutoffModOctave.getMin()
      ? double(0)
      : std::clamp(
          cutoffBase * std::exp2(cutoffModOctave), double(0),
          double(0.4999) - cutoffBase);

    resonanceBase = pv[ID::filterResonanceBase]->getDouble();
    resonanceMod = pv[ID::filterResonanceMod]->getDouble() * (double(1) - resonanceBase);

    notchBase = std::exp2(pv[ID::filterNotchBaseOctave]->getDouble());
    const auto notchModOctave = std::exp2(pv[ID::filterNotchModOctave]->getDouble());
    notchMod = notchModOctave <= Scales::filterNotchModOctave.getMin()
      ? double(0)
      : std::exp2(notchModOctave);
    filterDecayRatio = double(1);
    filterDecayGain
      = std::pow(decayRatio, std::exp2(-pv[ID::filterDecayRatio]->getDouble()));
  }
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

    if (!isPolyphonic && !activeNote.empty()) return;
    const auto targetId = isPolyphonic ? noteId : -1;
    for (auto &vc : voices) {
      if (vc.noteId != targetId) continue;
      vc.noteId = -1;
      vc.state = noteOffState;
      vc.resetArpeggio(pv[ID::seed]->getInt() + vc.unisonIndex);
    }
  }
}

void Voice::resetArpeggio(unsigned seed)
{
  rngArpeggio.seed(seed + unisonIndex);
  arpeggioTie = 0;
  arpeggioTimer = 0;
  arpeggioLoopCounter = 0;
}
