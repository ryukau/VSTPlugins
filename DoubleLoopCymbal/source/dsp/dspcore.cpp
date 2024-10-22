// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numbers>
#include <numeric>

constexpr double defaultTempo = double(120);
constexpr double spreaderMaxTimeSecond = double(0.008);
constexpr double releaseSecondFixed = double(0.5);
constexpr double velToCutScaler = double(6);
constexpr uint32_t seedOffset = 65537;

// `value` in [0, 1].
template<typename T> inline T decibelMap(T value, T minDB, T maxDB, bool minToZero)
{
  if (minToZero && value <= T(0)) return T(0);
  T dB = std::clamp(value * (maxDB - minDB) + minDB, minDB, maxDB);
  return std::pow(T(10), dB / T(20));
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
  baseSampleRateKp = EMAFilter<double>::secondToP(sampleRate, double(0.2));

  releaseSmoother.setup(double(2) * upRate);
  envelopeRelease.setSmooth(EMAFilter<double>::secondToP(upRate, double(0.001)));
  envelopeClose.setup(EMAFilter<double>::secondToP(upRate, double(0.004)));

  const auto maxDelayTimeSamples = upRate * double(0.05);
  allpassLoop1.setup(maxDelayTimeSamples);
  allpassLoop2.setup(maxDelayTimeSamples);

  spreader.setup(spreaderMaxTimeSecond * upRate);

  reset();
  startup();
}

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  useExternalInput = pv[ID::useExternalInput]->getInt();                                 \
                                                                                         \
  pitchSmoothingKp                                                                       \
    = EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble());    \
  auto notePitch = calcNotePitch(noteNumber);                                            \
  interpPitch.METHOD(notePitch);                                                         \
                                                                                         \
  externalInputGain.METHOD(pv[ID::externalInputGain]->getDouble() * double(0.5));        \
  impactTextureMix.METHOD(pv[ID::impactTextureMix]->getDouble());                        \
  impactHighpassCutoff.METHOD(pv[ID::impactHighpassHz]->getDouble() / upRate);           \
  halfClosedGain.METHOD(pv[ID::halfClosedGain]->getDouble());                            \
  halfClosedSustain.METHOD(pv[ID::halfClosedSustainLevel]->getDouble());                 \
  halfClosedDensity.METHOD(pv[ID::halfClosedDensityHz]->getDouble() / upRate);           \
  halfClosedHighpassCutoff.METHOD(pv[ID::halfClosedHighpassHz]->getDouble() / upRate);   \
  closingHighpassCutoff.METHOD(pv[ID::closingHighpassHz]->getDouble() / upRate);         \
  delayTimeModAmount.METHOD(                                                             \
    pv[ID::delayTimeModAmount]->getDouble() * upRate / double(48000));                   \
  allpassFeed1.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed1]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassFeed2.METHOD(                                                                   \
    std::clamp(pv[ID::allpassFeed2]->getDouble(), double(-0.99999), double(0.99999)));   \
  allpassMixSpike.METHOD(pv[ID::allpassMixSpike]->getDouble());                          \
  allpassMixAltSign.METHOD(pv[ID::allpassMixAltSign]->getDouble());                      \
  highShelfCutoff.METHOD(EMAFilter<double>::cutoffToP(                                   \
    std::min(pv[ID::highShelfFrequencyHz]->getDouble() / upRate, double(0.5))));         \
  highShelfGain.METHOD(pv[ID::highShelfGain]->getDouble());                              \
  lowShelfCutoff.METHOD(EMAFilter<double>::cutoffToP(                                    \
    std::min(pv[ID::lowShelfFrequencyHz]->getDouble() / upRate, double(0.5))));          \
  lowShelfGain.METHOD(pv[ID::lowShelfGain]->getDouble());                                \
                                                                                         \
  outputGain.METHOD(double(0.25) * pv[ID::outputGain]->getDouble());                     \
                                                                                         \
  envelopeNoise.setTime(pv[ID::impactDecaySeconds]->getDouble() * upRate);               \
                                                                                         \
  envelopeHalfClosed.setTime(                                                            \
    pv[ID::halfClosedDecaySecond]->getDouble() * upRate, releaseSecondFixed * upRate);   \
                                                                                         \
  const auto closingAttackSecond = pv[ID::closingAttackSecond]->getDouble();             \
  if (!pv[ID::release]->getInt() & noteStack.empty()) {                                  \
    const auto releaseTime                                                               \
      = pv[ID::closingReleaseRatio]->getDouble() * closingAttackSecond;                  \
    envelopeRelease.setTime(releaseTime *upRate, false);                                 \
  }                                                                                      \
                                                                                         \
  envelopeClose.update(                                                                  \
    upRate, closingAttackSecond, releaseSecondFixed, pv[ID::closingGain]->getDouble());  \
                                                                                         \
  halfClosedNoise.setDecay(pv[ID::halfClosedPulseSecond]->getDouble() * upRate);         \
  closingNoise.setDecay(double(0.0002) * upRate);                                        \
                                                                                         \
  updateDelayTime();                                                                     \
                                                                                         \
  spreaderSplit.METHOD(pv[ID::spreaderSplitHz]->getDouble() / upRate);                   \
  spreaderSpread.METHOD(pv[ID::spreaderSpread]->getDouble());

void DSPCore::updateUpRate()
{
  upRate = sampleRate * fold[overSampling];
  SmootherCommon<double>::setSampleRate(upRate);
  spreader.updateBaseTime(spreaderMaxTimeSecond * upRate);
}

void DSPCore::updateDelayTime()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  rngParam.seed(pv[ID::seed]->getInt());
  const auto randomRatio = pv[ID::delayTimeRandomRatio]->getDouble();
  const auto delayTimeBase = double(0.002) * upRate * (double(1) - randomRatio);
  const auto delayTimeRandom = double(0.002) * upRate * randomRatio;
  const auto pitchRatio
    = std::exp2(pv[ID::delayTimeLoopRatio]->getDouble() / double(-12));
  std::uniform_real_distribution<double> timeDist{double(0), double(delayTimeRandom)};
  for (size_t idx = 0; idx < nAllpass; ++idx) {
    static_assert(nAllpass >= 1);
    const auto harmonics = delayTimeBase / double(idx + 1);
    allpassLoop1.timeInSamples[idx] = harmonics + timeDist(rngParam);
    allpassLoop2.timeInSamples[idx] = pitchRatio * (harmonics + timeDist(rngParam));
  }

  const size_t nDelay1 = 1 + pv[ID::allpassDelayCount1]->getInt();
  const size_t nDelay2 = 1 + pv[ID::allpassDelayCount2]->getInt();
  allpassLoop1.nDelay = nDelay1;
  allpassLoop2.nDelay = nDelay2;
}

void DSPCore::reset()
{
  midiNotes.resize(0);
  noteStack.resize(0);

  overSampling = param.value[ParameterID::ID::overSampling]->getInt();
  updateUpRate();

  noteNumber = 60.0;
  noteOnVelocity = 0;

  ASSIGN_PARAMETER(reset);

  startup();

  impactHighpassScaler = double(1);
  halfClosedDensityScaler = double(1);
  halfClosedHighpassScaler = double(1);
  closingHighpassScaler = double(1);
  delayTimeModOffset = 0;

  impulse = 0;
  releaseSmoother.reset();
  envelopeNoise.reset();
  envelopeHalfClosed.reset();
  envelopeRelease.reset();
  envelopeClose.reset();
  impactHighpass.reset();
  halfClosedNoise.reset();
  closingNoise.reset();
  feedbackBuffer1 = 0;
  feedbackBuffer2 = 0;
  allpassLoop1.reset();
  allpassLoop2.reset();

  spreader.reset();

  prevExtIn.fill({});
  halfbandIir.reset();
}

void DSPCore::startup()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  const auto seed = pv[ID::seed]->getInt();
  rngNoisePinned.seed(seed + seedOffset);
  rngNoiseRolling.seed(seed);
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

double DSPCore::processFrame(const std::array<double, 2> &externalInput)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  const auto envRelease = envelopeRelease.process();

  const auto extGain = externalInputGain.process();
  const auto imTexture = impactTextureMix.process();
  const auto imCutoff = impactHighpassCutoff.process();
  const auto hcGain
    = halfClosedGain.process() * envelopeHalfClosed.process(halfClosedSustain.process());
  const auto hcDensity = halfClosedDensityScaler * halfClosedDensity.process();
  const auto hcCutoff = halfClosedHighpassCutoff.process();
  const auto clCutoff = closingHighpassCutoff.process();
  const auto timeModAmt = delayTimeModOffset + delayTimeModAmount.process();
  const auto apGain1 = allpassFeed1.process();
  const auto apGain2 = allpassFeed2.process();
  const auto apMixSpike = allpassMixSpike.process();
  const auto apMixSign = allpassMixAltSign.process();
  const auto hsCut = highShelfCutoff.process();
  const auto hsGain = highShelfGain.process();
  const auto lsCut = lowShelfCutoff.process();
  const auto lsGain = lowShelfGain.process();
  const auto outGain = outputGain.process() * envRelease;

  auto noiseEnv = releaseSmoother.process() + envelopeNoise.process();

  double excitation = -apGain1 * feedbackBuffer1;
  if (impulse != 0) {
    excitation += impulse;
    impulse = 0;
  } else {
    std::uniform_real_distribution<double> dist{double(-1), double(1)};
    const auto ipow = [](double v) { return v * v * v; };
    auto normalizeGain = double(2) - double(1) * std::abs(imTexture - double(0.5));
    auto impactNoise = std::lerp(dist(rngNoiseRolling), dist(rngNoisePinned), imTexture);
    excitation += impactHighpass.process(
      noiseEnv * normalizeGain * ipow(impactNoise), impactHighpassScaler * imCutoff);

    excitation += noteOnVelocity * hcGain
      * halfClosedNoise.process(
        hcDensity, double(1), halfClosedHighpassScaler * hcCutoff, rngNoiseRolling);
  }

  if (!pv[ID::release]->getInt() && noteStack.empty()) {
    std::uniform_real_distribution<double> dist{double(-1), double(1)};
    excitation += envelopeClose.process()
      * closingNoise.process(
        double(0.0008) * upRate, closingHighpassScaler * clCutoff, rngNoiseRolling);
  }

  if (useExternalInput) {
    excitation += (externalInput[0] + externalInput[1]) * extGain;
  }

  const auto pitchRatio = interpPitch.process(pitchSmoothingKp);
  const auto normalizeGain = nAllpass
    * std::lerp(double(1) / std::sqrt(hsCut / (double(2) - hsCut)), hsGain, hsGain);

  auto ap1
    = std::lerp(allpassLoop1.sum(apMixSign), feedbackBuffer1, apMixSpike) * normalizeGain;
  feedbackBuffer1 = allpassLoop1.process(
    excitation, hsCut, hsGain, lsCut, lsGain, apGain1, double(1), pitchRatio, timeModAmt);

  auto ap2
    = std::lerp(allpassLoop2.sum(apMixSign), feedbackBuffer2, apMixSpike) * normalizeGain;
  feedbackBuffer2 = allpassLoop2.process(
    ap1 - apGain2 * feedbackBuffer2, hsCut, hsGain, lsCut, lsGain, apGain2, double(1),
    pitchRatio, timeModAmt);

  return outGain * ap2;
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));
  SmootherCommon<double>::setSampleRate(upRate);

  double frame = 0;
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    const double extIn0 = in0 == nullptr ? 0 : in0[i];
    const double extIn1 = in1 == nullptr ? 0 : in1[i];

    if (overSampling) {
      const auto sig0 = processFrame(
        {double(0.5) * (prevExtIn[0] + extIn0), double(0.5) * (prevExtIn[1] + extIn1)});
      const auto sig1 = processFrame({extIn0, extIn1});
      frame = halfbandIir.process({sig0, sig1});
    } else {
      frame = processFrame({extIn0, extIn1});
    }

    const auto spSplit = spreaderSplit.process(baseSampleRateKp);
    const auto spSpread = spreaderSpread.process(baseSampleRateKp);
    auto sig = spreader.process(frame, spSplit, spSpread);

    out0[i] = float(sig[0]);
    out1[i] = float(sig[1]);

    prevExtIn = {extIn0, extIn1};
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto eps = std::numeric_limits<double>::epsilon();

  noteNumber = info.noteNumber;
  auto notePitch = calcNotePitch(info.noteNumber);
  interpPitch.push(notePitch);

  const double velocityLin = info.velocity;
  noteOnVelocity = decibelMap(
    double(info.velocity), pv[ID::velocityToImpactGain]->getDouble(), double(0), false);

  const auto seed = pv[ID::seed]->getInt();
  if (pv[ID::resetSeedAtNoteOn]->getInt()) rngNoiseRolling.seed(seed);
  rngNoisePinned.seed(seed + seedOffset);

  const auto lossGain = noteStack.empty() ? envelopeRelease.value : double(1);
  allpassLoop1.applyGain(lossGain);
  allpassLoop2.applyGain(lossGain);

  releaseSmoother.prepare(
    double(0.1) * envelopeNoise.process(),
    upRate * pv[ID::impactDecaySeconds]->getDouble());

  const auto oscGain = noteOnVelocity * pv[ID::impactGain]->getDouble();
  impulse = oscGain;
  envelopeNoise.trigger(oscGain);

  impactHighpassScaler = std::exp2(
    velToCutScaler * velocityLin * pv[ID::velocityToImpactHighpass]->getDouble());
  halfClosedDensityScaler = std::exp2(
    double(4) * velocityLin * pv[ID::velocityToHalfClosedDensity]->getDouble());
  halfClosedHighpassScaler = std::exp2(
    velToCutScaler * velocityLin * pv[ID::velocityToHalfClosedHighpass]->getDouble());

  envelopeHalfClosed.trigger(halfClosedSustain.getValue(), halfClosedDensityScaler);

  envelopeRelease.trigger();
  envelopeRelease.setTime(double(1), true);

  delayTimeModOffset = pv[ID::velocityToDelayTimeMod]->getDouble();

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId, double noteOffVelocity)
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
  } else {
    envelopeHalfClosed.release();
    if (!pv[ID::release]->getInt()) {
      const auto closingVelocity = pv[ID::useNoteOffVelocityForClosing]->getInt()
        ? noteOffVelocity
        : noteOnVelocity;

      const auto velToDur = pv[ID::noteOffVelocityToClosingReleaseRatio]->getDouble();
      const auto closeDurationRatio = std::exp2(double(4) * closingVelocity * velToDur);
      const auto releaseTime = pv[ID::closingReleaseRatio]->getDouble()
        * pv[ID::closingAttackSecond]->getDouble();
      envelopeRelease.setTime(releaseTime * closeDurationRatio * upRate, false);

      const auto minDecibel = pv[ID::noteOffVelocityToClosingGain]->getDouble();
      const auto gain = decibelMap(closingVelocity, minDecibel, double(0), true);
      envelopeClose.trigger(
        upRate, pv[ID::closingAttackSecond]->getDouble(), releaseSecondFixed,
        pv[ID::closingGain]->getDouble(), gain);

      closingHighpassScaler = std::exp2(
        velToCutScaler * noteOffVelocity
        * pv[ID::noteOffVelocityToClosingHighpass]->getDouble());
    }
    noteOnVelocity = 0;
  }
}

double DSPCore::calcNotePitch(double note)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  constexpr auto centerNote = double(60);
  auto pitchBendSemitone
    = pv[ID::pitchBendRange]->getDouble() * pv[ID::pitchBend]->getDouble();
  auto cent = pv[ID::transposeSemitone]->getDouble();
  auto notePitchAmount = pv[ID::notePitchAmount]->getDouble();
  return std::exp2(
    (pitchBendSemitone + notePitchAmount * (note - centerNote) + cent) / double(12));
}
