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
  noiseAllpass.setup(maxDelayTimeSamples);
  wireAllpass.setup(maxDelayTimeSamples);
  wireEnergyDecay.setup(upRate * double(0.001));
  membrane1EnergyDecay.setup(upRate * double(0.001));
  membrane2EnergyDecay.setup(upRate * double(0.001));
  membrane1.setup(maxDelayTimeSamples);
  membrane2.setup(maxDelayTimeSamples);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
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
  outputGain.METHOD(pv[ID::outputGain]->getDouble());                                    \
                                                                                         \
  safetyHighpass.METHOD(                                                                 \
    pv[ID::safetyHighpassHz]->getDouble() / sampleRate,                                  \
    std::numbers::sqrt2_v<double> / double(2));                                          \
                                                                                         \
  noiseLowpass.METHOD(pv[ID::noiseLowpassHz]->getDouble() / upRate);                     \
  paramRng.seed(pv[ID::seed]->getInt());                                                 \
  noiseAllpass.timeInSamples.METHOD(                                                     \
    prepareSerialAllpassTime<decltype(noiseAllpass)::size>(                              \
      upRate, pv[ID::noiseAllpassMaxTimeHz]->getDouble(), paramRng));                    \
  wireAllpass.timeInSamples.METHOD(                                                      \
    prepareSerialAllpassTime<decltype(wireAllpass)::size>(                               \
      upRate, pv[ID::wireFrequencyHz]->getDouble(), paramRng));                          \
                                                                                         \
  for (size_t idx = 0; idx < maxFdnSize; ++idx) {                                        \
    const auto crossFeedbackRatio = pv[ID::crossFeedbackRatio0 + idx]->getDouble();      \
    feedbackMatrix.seed[idx] = crossFeedbackRatio * crossFeedbackRatio;                  \
  }                                                                                      \
  feedbackMatrix.constructHouseholder();                                                 \
                                                                                         \
  const auto secondaryPitchOffset = pv[ID::secondaryPitchOffset]->getDouble();           \
  const auto delayTimeFreq1 = pv[ID::delayTimeHz]->getDouble() / upRate;                 \
  const auto delayTimeFreq2 = delayTimeFreq1 * std::exp2(secondaryPitchOffset);          \
  const auto bandpassCutRatio = std::exp2(pv[ID::bandpassCutRatio]->getDouble());        \
  const auto secondaryQOffset = pv[ID::secondaryQOffset]->getDouble();                   \
  const auto delayTimeSpread = pv[ID::delayTimeSpread]->getDouble();                     \
  const auto bandpassCutSpread = pv[ID::bandpassCutSpread]->getDouble();                 \
  const auto pitchRandomCent = pv[ID::pitchRandomCent]->getDouble();                     \
  const size_t pitchType = pv[ID::pitchType]->getInt();                                  \
  for (size_t idx = 0; idx < maxFdnSize; ++idx) {                                        \
    const auto pitch = pitchFunc(pitchType, idx);                                        \
                                                                                         \
    const auto delayCutRatio1                                                            \
      = pitchRatio(pitch, delayTimeSpread, pitchRandomCent, paramRng);                   \
    membrane1.delayTimeSamples[idx] = double(1) / (delayTimeFreq1 * delayCutRatio1);     \
                                                                                         \
    const auto bpCutRatio1 = bandpassCutRatio                                            \
      * pitchRatio(pitch, bandpassCutSpread, pitchRandomCent, paramRng);                 \
    membrane1.bandpassCutoff.METHOD##At(idx, delayTimeFreq1 *bpCutRatio1);               \
                                                                                         \
    const auto delayCutRatio2                                                            \
      = pitchRatio(pitch, delayTimeSpread, pitchRandomCent, paramRng);                   \
    membrane2.delayTimeSamples[idx] = double(1) / (delayTimeFreq2 * delayCutRatio2);     \
                                                                                         \
    const auto bpCutRatio2 = bandpassCutRatio                                            \
      * pitchRatio(pitch, bandpassCutSpread, pitchRandomCent, paramRng);                 \
    membrane2.bandpassCutoff.METHOD##At(idx, delayTimeFreq2 *bpCutRatio2);               \
  }                                                                                      \
  const auto bandpassQ = pv[ID::bandpassQ]->getDouble();                                 \
  membrane1.bandpassQ.METHOD(bandpassQ);                                                 \
  membrane2.bandpassQ.METHOD(                                                            \
    std::clamp(bandpassQ *std::exp2(secondaryQOffset), double(0.1), double(100)));

void DSPCore::updateUpRate()
{
  upRate = sampleRate * fold[overSampling];
  SmootherCommon<double>::setSampleRate(upRate);
  membrane1.onSampleRateChange(upRate);
  membrane2.onSampleRateChange(upRate);
}

void DSPCore::reset()
{
  overSampling = param.value[ParameterID::ID::overSampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  startup();

  noteNumber = 69.0;
  velocity = 0;

  noiseGain = 0;
  noiseDecay = 0;
  noiseAllpass.reset();

  wireAllpass.reset();
  wireEnergyDecay.reset();
  wireEnergyNoise.reset();
  wirePosition = 0;
  wireVelocity = 0;
  wireGain = 0;
  wireDecay = 0;

  envelope.reset();
  releaseSmoother.reset();

  membrane1Position = 0;
  membrane1Velocity = 0;
  membrane2Position = 0;
  membrane2Velocity = 0;
  membrane1EnergyDecay.reset();
  membrane2EnergyDecay.reset();
  membrane1.reset();
  membrane2.reset();

  halfbandIir.reset();
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
}

// Overwrites `p0` and `p1`.
inline void solveCollision(double &p0, double &p1, double v0, double v1, double distance)
{
  const auto diff = p0 - p1 + distance;
  if (diff >= 0) {
    p0 = 0;
    p1 = 0;
    return;
  }

  auto sum = -diff;
  const auto r0 = std::abs(v0);
  const auto r1 = std::abs(v1);
  if (r0 + r1 >= std::numeric_limits<double>::epsilon()) sum /= r0 + r1;
  p0 = sum * r1;
  p1 = -sum * r0;
}

double DSPCore::processSample()
{
  wireDistance.process();
  wireCollisionTypeMix.process();
  impactWireMix.process();
  secondaryDistance.process();
  const auto crossGain = crossFeedbackGain.process();
  const auto timeModAmt = delayTimeModAmount.process();
  secondaryFdnMix.process();
  membraneWireMix.process();
  outputGain.process();

  constexpr auto eps = std::numeric_limits<double>::epsilon();
  double sig = 0;
  if (noiseGain > eps) {
    std::uniform_real_distribution<double> dist{double(-1), double(1)};
    const auto noise = noiseGain * dist(noiseRng);
    noiseGain *= noiseDecay;
    sig += noiseLowpass.process(noise);
  }
  sig = std::tanh(noiseAllpass.process(sig, double(0.95)));

  solveCollision(
    wirePosition, membrane1Position, wireVelocity, membrane1Velocity,
    wireDistance.getValue());

  // TODO: Send wire-membrane1 collision status to GUI.

  auto wireCollision = std::lerp(
    wireEnergyNoise.process(wirePosition, noiseRng),
    wireEnergyDecay.process(wirePosition), wireCollisionTypeMix.getValue());
  wireCollision = double(8) * std::tanh(double(0.125) * wireCollision);
  const auto wireIn = double(0.995) * (sig + wireCollision);
  const auto wirePos = wireAllpass.process(wireIn, double(0.5)) * wireGain;
  wireGain *= wireDecay;
  wireVelocity = wirePos - wirePosition;
  wirePosition = wirePos;

  const auto wireOut = std::lerp(sig, wirePosition, impactWireMix.getValue());
  sig = wireOut;

  solveCollision(
    membrane1Position, membrane2Position, membrane1Velocity, membrane2Velocity,
    secondaryDistance.getValue());

  // TODO: Send membrane1-membrane2 collision status to GUI.

  const auto env = std::exp2(envelope.process() + releaseSmoother.process());
  const auto pitch = env * interpPitch.process(pitchSmoothingKp);

  const auto collision1 = membrane1EnergyDecay.process(membrane1Position);
  const auto p1 = membrane1.process(sig, crossGain, pitch, timeModAmt, feedbackMatrix);
  membrane1Velocity = p1 - membrane1Position;
  membrane1Position = p1;

  const auto collision2 = membrane2EnergyDecay.process(membrane2Position);
  const auto p2 = membrane2.process(sig, crossGain, pitch, timeModAmt, feedbackMatrix);
  membrane2Velocity = p2 - membrane2Position;
  membrane2Position = p2;

  sig = std::lerp(p1, p2, secondaryFdnMix.getValue());
  sig = std::lerp(sig, wireOut, membraneWireMix.getValue());

  return outputGain.getValue() * sig;
}

void DSPCore::process(const size_t length, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  bool isSafetyHighpassEnabled = pv[ID::safetyHighpassEnable]->getInt();

  std::array<double, 2> halfbandInput{};
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    if (overSampling) {
      for (size_t j = 0; j < upFold; ++j) halfbandInput[j] = processSample();
      auto sig = float(halfbandIir.process(halfbandInput));
      if (isSafetyHighpassEnabled) sig = safetyHighpass.process(sig);
      out0[i] = sig;
      out1[i] = sig;
    } else {
      auto sig = float(processSample());
      if (isSafetyHighpassEnabled) sig = safetyHighpass.process(sig);
      out0[i] = sig;
      out1[i] = sig;
    }
  }
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
  if (pv[ID::slideAtNoteOn]->getInt()) {
    interpPitch.push(notePitch);
  } else {
    interpPitch.reset(notePitch);
  }

  velocity = velocityMap.map(info.velocity);

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

  const auto crossFeedbackGain = pv[ID::crossFeedbackGain]->getDouble();
  membrane1.noteOn();
  membrane2.noteOn();
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

  if (!noteStack.empty() && pv[ID::slideAtNoteOff]->getInt()) {
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
  auto equalTemperament = pv[ID::tuningET]->getInt() + 1;
  return std::exp2((note + semitone + cent) / equalTemperament);
}
