// (c) 2023 Takamitsu Endo
//
// This file is part of GenericDrum.
//
// GenericDrum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenericDrum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenericDrum.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

constexpr double defaultTempo = double(120);

inline double calcOscillatorPitch(double octave, double cent)
{
  return std::exp2(octave - octaveOffset + cent / 1200.0);
}

inline double calcPitch(double semitone, double equalTemperament = 12.0)
{
  return std::exp2(semitone / equalTemperament);
}

template<size_t nAllpass, typename Rng>
inline auto prepareSerialAllpassTime(double upRate, double allpassMaxTimeHz, Rng &rng)
{
  std::array<double, nAllpass> delaySamples{};
  const auto scaler = std::max(
    double(0), std::ceil(upRate * nAllpass / allpassMaxTimeHz) - double(2) * nAllpass);
  double sumSamples = 0;
  std::uniform_real_distribution<double> dist{double(0), double(1)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    delaySamples[idx] = dist(rng);
    sumSamples += delaySamples[idx];
  }
  double sumFraction = 0;
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    const auto samples = double(2) + scaler * delaySamples[idx] / sumSamples;
    delaySamples[idx] = std::floor(samples);
    sumFraction += samples - delaySamples[idx];
  }
  delaySamples[0] += std::round(sumFraction);
  return delaySamples;
}

constexpr size_t nPitch = 8;
constexpr std::array<double, nPitch> pitchHarmonicPlus12{
  double(1),  double(4),  double(5),  double(12),
  double(13), double(15), double(16), double(24),
};
constexpr std::array<double, nPitch> pitchHarmonicTimes5{
  double(1),  double(5),  double(8),  double(10),
  double(15), double(16), double(20), double(24),
};
constexpr std::array<double, nPitch> pitchSemitone1_2_7_9{
  double(1), double(8) / double(7), double(3) / double(2), double(5) / double(3),
  double(1), double(8) / double(7), double(3) / double(2), double(5) / double(3),
};
constexpr std::array<double, nPitch> pitchCircularMembraneMode{
  double(1.00000000000000),  double(1.593340505695112),  double(2.1355487866494034),
  double(2.295417267427694), double(2.6530664045492145), double(2.9172954551172228),
  double(3.155464815408362), double(3.5001474903090264),
};
constexpr std::array<double, nPitch> pitchPrimeNumber{
  double(2) / double(2),  double(3) / double(2),  double(5) / double(2),
  double(7) / double(2),  double(11) / double(2), double(13) / double(2),
  double(17) / double(2), double(19) / double(2),
};

enum PitchTypeName : size_t {
  harmonic,
  harmonicPlus12,
  harmonicTimes5,
  harmonicCycle1_5,
  harmonicOdd,
  semitone1_2_7_9,
  circularMembraneMode,
  primeNumber,
  octave,
};

inline double pitchFunc(size_t pitchType, size_t index)
{
  index %= size_t(8);
  if (pitchType == harmonic) {
    return double(index + 1);
  } else if (pitchType == harmonicPlus12) {
    return pitchHarmonicPlus12[index];
  } else if (pitchType == harmonicTimes5) {
    return pitchHarmonicTimes5[index];
  } else if (pitchType == harmonicCycle1_5) {
    return (index & 1) == 0 ? double(1) : double(5);
  } else if (pitchType == harmonicOdd) {
    return double(2 * index + 1);
  } else if (pitchType == semitone1_2_7_9) {
    return pitchSemitone1_2_7_9[index];
  } else if (pitchType == circularMembraneMode) {
    return pitchCircularMembraneMode[index];
  } else if (pitchType == primeNumber) {
    return pitchPrimeNumber[index];
  }
  return double(size_t(1) << index); // `pitchType == octave`.
}

template<typename Rng>
inline double pitchRatio(double pitch, double spread, double rndCent, Rng &rng)
{
  const auto rndRange = rndCent / double(1200);
  std::uniform_real_distribution<double> dist{-rndRange, rndRange};
  return std::lerp(double(1), pitch, spread) * std::exp2(dist(rng));
}

void DSPCore::setup(double sampleRate)
{
  noteStack.reserve(1024);
  noteStack.resize(0);

  this->sampleRate = sampleRate;
  upRate = sampleRate * upFold;

  SmootherCommon<double>::setTime(double(0.2));

  const auto maxDelayTimeSamples = upRate;
  for (auto &x : noiseAllpass) x.setup(maxDelayTimeSamples);
  for (auto &x : wireAllpass) x.setup(maxDelayTimeSamples);
  for (auto &x : wireEnergyDecay) x.setup(upRate * double(0.001));
  for (auto &x : membrane1EnergyDecay) x.setup(upRate * double(0.001));
  for (auto &x : membrane2EnergyDecay) x.setup(upRate * double(0.001));
  for (auto &x : membrane1) x.setup(maxDelayTimeSamples);
  for (auto &x : membrane2) x.setup(maxDelayTimeSamples);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  preventBlowUp = pv[ID::preventBlowUp]->getInt();                                       \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto pitchBend                                                                         \
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());   \
  auto notePitch = calcNotePitch(pitchBend * noteNumber);                                \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  wireDistance.METHOD(pv[ID::wireDistance]->getDouble());                                \
  wireCollisionTypeMix.METHOD(pv[ID::wireCollisionTypeMix]->getDouble());                \
  impactWireMix.METHOD(pv[ID::impactWireMix]->getDouble());                              \
  secondaryDistance.METHOD(pv[ID::secondaryDistance]->getDouble());                      \
  crossFeedbackGain.METHOD(pv[ID::crossFeedbackGain]->getDouble());                      \
  delayTimeModAmount.METHOD(                                                             \
    pv[ID::delayTimeModAmount]->getDouble() * upRate / double(48000));                   \
  secondaryFdnMix.METHOD(pv[ID::secondaryFdnMix]->getDouble());                          \
  membraneWireMix.METHOD(pv[ID::membraneWireMix]->getDouble());                          \
  stereoBalance.METHOD(pv[ID::stereoBalance]->getDouble());                              \
  stereoMerge.METHOD(pv[ID::stereoMerge]->getDouble() / double(2));                      \
                                                                                         \
  constexpr auto highpassQ = std::numbers::sqrt2_v<double> / double(2);                  \
  const auto highpassCut = pv[ID::safetyHighpassHz]->getDouble() / sampleRate;           \
  for (size_t ch = 0; ch < 2; ++ch) {                                                    \
    safetyHighpass[ch].METHOD(highpassCut, highpassQ);                                   \
  }                                                                                      \
                                                                                         \
  paramRng.seed(pv[ID::seed]->getInt());                                                 \
                                                                                         \
  for (size_t idx = 0; idx < maxFdnSize; ++idx) {                                        \
    const auto crossFeedbackRatio = pv[ID::crossFeedbackRatio0 + idx]->getDouble();      \
    feedbackMatrix.seed[idx] = crossFeedbackRatio * crossFeedbackRatio;                  \
  }                                                                                      \
  feedbackMatrix.constructHouseholder();                                                 \
                                                                                         \
  const auto noiseLowpassFreq = pv[ID::noiseLowpassHz]->getDouble() / upRate;            \
  const auto noiseAllpassMaxTimeHz = pv[ID::noiseAllpassMaxTimeHz]->getDouble();         \
  const auto wireFrequencyHz = pv[ID::wireFrequencyHz]->getDouble();                     \
  const auto secondaryPitchOffset = pv[ID::secondaryPitchOffset]->getDouble();           \
  const auto delayTimeFreq1 = pv[ID::delayTimeHz]->getDouble() / upRate;                 \
  const auto delayTimeFreq2 = delayTimeFreq1 * std::exp2(secondaryPitchOffset);          \
  const auto bandpassCutRatio = std::exp2(pv[ID::bandpassCutRatio]->getDouble());        \
  const auto secondaryQOffset = pv[ID::secondaryQOffset]->getDouble();                   \
  const auto delayTimeSpread = pv[ID::delayTimeSpread]->getDouble();                     \
  const auto bandpassCutSpread = pv[ID::bandpassCutSpread]->getDouble();                 \
  const auto pitchRandomCent = pv[ID::pitchRandomCent]->getDouble();                     \
  const size_t pitchType = pv[ID::pitchType]->getInt();                                  \
                                                                                         \
  auto gain = pv[ID::outputGain]->getDouble();                                           \
  if (pv[ID::normalizeGainWrtNoiseLowpassHz]->getInt()) {                                \
    gain *= approxNormalizeGain(noiseLowpassFreq);                                       \
  }                                                                                      \
  outputGain.METHOD(gain);                                                               \
                                                                                         \
  for (size_t drm = 0; drm < nDrum; ++drm) {                                             \
    noiseLowpass[drm].METHOD(noiseLowpassFreq);                                          \
    noiseAllpass[drm].timeInSamples.METHOD(                                              \
      prepareSerialAllpassTime<nAllpass>(upRate, noiseAllpassMaxTimeHz, paramRng));      \
    wireAllpass[drm].timeInSamples.METHOD(                                               \
      prepareSerialAllpassTime<nAllpass>(upRate, wireFrequencyHz, paramRng));            \
                                                                                         \
    for (size_t idx = 0; idx < maxFdnSize; ++idx) {                                      \
      const auto pitch = pitchFunc(pitchType, idx);                                      \
                                                                                         \
      const auto delayCutRatio1                                                          \
        = pitchRatio(pitch, delayTimeSpread, pitchRandomCent, paramRng);                 \
      membrane1[drm].delayTimeSamples[idx]                                               \
        = double(1) / (delayTimeFreq1 * delayCutRatio1);                                 \
                                                                                         \
      const auto bpCutRatio1 = bandpassCutRatio                                          \
        * pitchRatio(pitch, bandpassCutSpread, pitchRandomCent, paramRng);               \
      membrane1[drm].bandpassCutoff.METHOD##At(idx, delayTimeFreq1 *bpCutRatio1);        \
                                                                                         \
      const auto delayCutRatio2                                                          \
        = pitchRatio(pitch, delayTimeSpread, pitchRandomCent, paramRng);                 \
      membrane2[drm].delayTimeSamples[idx]                                               \
        = double(1) / (delayTimeFreq2 * delayCutRatio2);                                 \
                                                                                         \
      const auto bpCutRatio2 = bandpassCutRatio                                          \
        * pitchRatio(pitch, bandpassCutSpread, pitchRandomCent, paramRng);               \
      membrane2[drm].bandpassCutoff.METHOD##At(idx, delayTimeFreq2 *bpCutRatio2);        \
    }                                                                                    \
    const auto bandpassQ = pv[ID::bandpassQ]->getDouble();                               \
    membrane1[drm].bandpassQ.METHOD(bandpassQ);                                          \
    membrane2[drm].bandpassQ.METHOD(                                                     \
      std::clamp(bandpassQ *std::exp2(secondaryQOffset), double(0.1), double(100)));     \
  }

void DSPCore::updateUpRate()
{
  upRate = sampleRate * fold[overSampling];
  SmootherCommon<double>::setSampleRate(upRate);
  for (auto &x : membrane1) x.onSampleRateChange(upRate);
  for (auto &x : membrane2) x.onSampleRateChange(upRate);
}

void DSPCore::resetCollision()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  pv[ID::isWireCollided]->setFromInt(0);
  isWireCollided = false;

  pv[ID::isSecondaryCollided]->setFromInt(0);
  isSecondaryCollided = false;
}

void DSPCore::reset()
{
  overSampling = param.value[ParameterID::ID::overSampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  startup();
  resetCollision();

  noteNumber = 69.0;
  velocity = 0;

  noiseGain = 0;
  noiseDecay = 0;
  for (auto &x : noiseAllpass) x.reset();

  for (auto &x : wireAllpass) x.reset();
  for (auto &x : wireEnergyDecay) x.reset();
  for (auto &x : wireEnergyNoise) x.reset();
  wirePosition.fill({});
  wireVelocity.fill({});
  wireGain = 0;
  wireDecay = 0;

  envelope.reset();
  releaseSmoother.reset();

  membrane1Position.fill({});
  membrane1Velocity.fill({});
  membrane2Position.fill({});
  membrane2Velocity.fill({});
  for (auto &x : membrane1EnergyDecay) x.reset();
  for (auto &x : membrane2EnergyDecay) x.reset();
  for (auto &x : membrane1) x.reset();
  for (auto &x : membrane2) x.reset();

  for (auto &x : halfbandIir) x.reset();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;
  noiseRng.seed(pv[ID::seed]->getInt());
}

void DSPCore::setParameters()
{
  size_t newOverSampling = param.value[ParameterID::ID::overSampling]->getInt();
  if (overSampling != newOverSampling) {
    overSampling = newOverSampling;
    updateUpRate();
  }
  ASSIGN_PARAMETER(push);

  isWireCollided = pv[ID::isWireCollided]->getInt();
  isSecondaryCollided = pv[ID::isSecondaryCollided]->getInt();
}

// Overwrites `p0` and `p1`.
inline void solveCollision(double &p0, double &p1, double v0, double v1, double distance)
{
  auto diff = p0 - p1 + distance;
  if (diff >= 0) {
    p0 = 0;
    p1 = 0;
    return;
  }

  const auto r0 = std::abs(v0);
  const auto r1 = std::abs(v1);
  if (r0 + r1 > std::numeric_limits<double>::epsilon()) diff /= r0 + r1;
  p0 = -diff * r1;
  p1 = diff * r0;
}

double DSPCore::processDrum(
  size_t index, double noise, double wireGain, double crossGain, double timeModAmt)
{
  // Impact.
  constexpr auto eps = std::numeric_limits<double>::epsilon();
  double sig = 0;
  sig += noiseLowpass[index].process(noise);
  sig = std::tanh(noiseAllpass[index].process(sig, double(0.95)));

  // Wire.
  solveCollision(
    wirePosition[index], membrane1Position[index], wireVelocity[index],
    membrane1Velocity[index], wireDistance.getValue());

  if (!isWireCollided && wirePosition[index] != 0) isWireCollided = true;

  auto wireCollision = std::lerp(
    wireEnergyNoise[index].process(wirePosition[index], preventBlowUp, noiseRng),
    wireEnergyDecay[index].process(wirePosition[index], preventBlowUp),
    wireCollisionTypeMix.getValue());
  wireCollision = double(8) * std::tanh(double(0.125) * wireCollision);
  const auto wireIn = double(0.995) * (sig + wireCollision);
  const auto wirePos = wireAllpass[index].process(wireIn, double(0.5)) * wireGain;
  wireVelocity[index] = wirePos - wirePosition[index];
  wirePosition[index] = wirePos;

  const auto wireOut = std::lerp(sig, wirePosition[index], impactWireMix.getValue());
  sig = wireOut;

  // Membranes.
  solveCollision(
    membrane1Position[index], membrane2Position[index], membrane1Velocity[index],
    membrane2Velocity[index], secondaryDistance.getValue());

  if (!isSecondaryCollided && membrane2Position[index] != 0) isSecondaryCollided = true;

  const auto env = std::exp2(envelope.process() + releaseSmoother.process());
  const auto pitch = env * interpPitch.process(pitchSmoothingKp);
  feedbackMatrix.process();

  const auto collision1
    = membrane1EnergyDecay[index].process(membrane1Position[index], false)
    / double(maxFdnSize);
  const auto p1 = membrane1[index].process(
    sig + collision1, crossGain, pitch, timeModAmt, feedbackMatrix);
  membrane1Velocity[index] = p1 - membrane1Position[index];
  membrane1Position[index] = p1;

  const auto collision2
    = membrane2EnergyDecay[index].process(membrane2Position[index], false)
    / double(maxFdnSize);
  const auto p2 = membrane2[index].process(
    sig + collision2, crossGain, pitch, timeModAmt, feedbackMatrix);
  membrane2Velocity[index] = p2 - membrane2Position[index];
  membrane2Position[index] = p2;

  // Mix.
  sig = std::lerp(p1, p2, secondaryFdnMix.getValue());
  sig = std::lerp(sig, wireOut, membraneWireMix.getValue());
  return sig;
}

#define PROCESS_COMMON                                                                   \
  wireDistance.process();                                                                \
  wireCollisionTypeMix.process();                                                        \
  impactWireMix.process();                                                               \
  secondaryDistance.process();                                                           \
  const auto crossGain = crossFeedbackGain.process();                                    \
  const auto timeModAmt = delayTimeModAmount.process();                                  \
  secondaryFdnMix.process();                                                             \
  membraneWireMix.process();                                                             \
  const auto balance = stereoBalance.process();                                          \
  const auto merge = stereoMerge.process();                                              \
  const auto outGain = outputGain.process();                                             \
                                                                                         \
  std::uniform_real_distribution<double> dist{double(-0.5), double(0.5)};                \
  const auto noise = noiseGain * (dist(noiseRng) + dist(noiseRng));                      \
  noiseGain *= noiseDecay;                                                               \
  wireGain *= wireDecay;

double DSPCore::processSample()
{
  PROCESS_COMMON;

  return outGain * processDrum(0, noise, wireGain, crossGain, timeModAmt);
}

std::array<double, 2> DSPCore::processFrame()
{
  PROCESS_COMMON;

  auto drum0 = processDrum(0, noise, wireGain, crossGain, timeModAmt);
  auto drum1 = processDrum(1, noise, wireGain, crossGain, timeModAmt);

  constexpr auto eps = std::numeric_limits<double>::epsilon();
  if (balance < -eps) {
    drum0 *= double(1) + balance;
  } else if (balance > eps) {
    drum1 *= double(1) - balance;
  }
  return {
    outGain * std::lerp(drum0, drum1, merge),
    outGain * std::lerp(drum1, drum0, merge),
  };
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  bool isStereo = pv[ID::stereoEnable]->getInt();
  bool isSafetyHighpassEnabled = pv[ID::safetyHighpassEnable]->getInt();

  std::array<double, 2> frame{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (isStereo) {
      if (overSampling) {
        for (size_t j = 0; j < upFold; ++j) {
          frame = processFrame();
          halfbandInput[0][j] = frame[0];
          halfbandInput[1][j] = frame[1];
        }
        frame[0] = halfbandIir[0].process(halfbandInput[0]);
        frame[1] = halfbandIir[1].process(halfbandInput[1]);
        if (isSafetyHighpassEnabled) {
          frame[0] = safetyHighpass[0].process(frame[0]);
          frame[1] = safetyHighpass[1].process(frame[1]);
        }
        out0[i] = float(frame[0]);
        out1[i] = float(frame[1]);
      } else {
        frame = processFrame();
        if (isSafetyHighpassEnabled) {
          frame[0] = safetyHighpass[0].process(frame[0]);
          frame[1] = safetyHighpass[1].process(frame[1]);
        }
        out0[i] = float(frame[0]);
        out1[i] = float(frame[1]);
      }
    } else {
      if (overSampling) {
        for (size_t j = 0; j < upFold; ++j) halfbandInput[0][j] = processSample();
        frame[0] = halfbandIir[0].process(halfbandInput[0]);
        if (isSafetyHighpassEnabled) frame[0] = safetyHighpass[0].process(frame[0]);
        out0[i] = float(frame[0]);
        out1[i] = float(frame[0]);
      } else {
        frame[0] = processSample();
        if (isSafetyHighpassEnabled) frame[0] = safetyHighpass[0].process(frame[0]);
        out0[i] = float(frame[0]);
        out1[i] = float(frame[0]);
      }
    }
  }

  // Send a value to GUI.
  if (isWireCollided) pv[ID::isWireCollided]->setFromInt(1);
  if (isSecondaryCollided) pv[ID::isSecondaryCollided]->setFromInt(1);
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto eps = std::numeric_limits<double>::epsilon();

  noteStack.push_back(info);

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  auto pitchBend
    = calcPitch(pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble());
  interpPitch.push(notePitch);

  velocity = velocityMap.map(info.velocity);

  if (pv[ID::resetSeedAtNoteOn]->getInt()) noiseRng.seed(pv[ID::seed]->getInt());

  noiseGain = velocity;
  noiseDecay = std::pow(
    double(1e-3), double(1) / (upRate * pv[ID::noiseDecaySeconds]->getDouble()));

  wireGain = double(2);
  wireDecay = std::pow(eps, double(1) / (upRate * pv[ID::wireDecaySeconds]->getDouble()));

  releaseSmoother.prepare(envelope.process(), double(0.002) * upRate);
  envelope.noteOn(
    pv[ID::envelopeModAmount]->getDouble(),
    pv[ID::envelopeAttackSeconds]->getDouble() * upRate,
    pv[ID::envelopeDecaySeconds]->getDouble() * upRate);

  // membrane1.noteOn();
  // membrane2.noteOn();

  resetCollision();
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

  if (!noteStack.empty()) {
    noteNumber = noteStack.back().noteNumber;
    interpPitch.push(calcNotePitch(noteNumber));
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto semitone = pv[ID::tuningSemitone]->getInt() - double(semitoneOffset + 57);
  auto cent = pv[ID::tuningCent]->getDouble() / double(100);
  auto notePitchAmount = pv[ID::notePitchAmount]->getDouble();
  return std::exp2(notePitchAmount * (note + semitone + cent) / double(12));
}
