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

template<typename T> inline void syncPhase(T &phase0, T &phase1, T kp)
{
  auto d1 = phase1 - phase0;
  if (d1 < 0) {
    auto d2 = d1 + double(1);
    phase0 += kp * (d2 < -d1 ? d2 : d1);
  } else {
    auto d2 = d1 - double(1);
    phase0 += kp * (-d2 < d1 ? d2 : d1);
  }
}

template<typename T> inline T crossPhase(T phase0, T phase1, T ratio)
{
  auto d1 = phase1 - phase0;
  if (d1 < 0) {
    auto d2 = d1 + double(1);
    return phase0 + ratio * (d2 < -d1 ? d2 : d1);
  }
  auto d2 = d1 - double(1);
  return phase0 + ratio * (-d2 < d1 ? d2 : d1);
}

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);

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
                                                                                         \
  stereoPhaseLinkKp.METHOD(                                                              \
    EMAFilter<double>::cutoffToP(upRate, pv[ID::stereoPhaseLinkHz]->getDouble()));       \
  stereoPhaseCross.METHOD(pv[ID::stereoPhaseCross]->getDouble());                        \
  stereoPhaseOffset.METHOD(pv[ID::stereoPhaseOffset]->getDouble());                      \
  phaseWarp.METHOD(pv[ID::phaseWarp]->getDouble());                                      \
                                                                                         \
  auto phaseModCorrection = double(48000) / upRate;                                      \
  inputPhaseMod.METHOD(pv[ID::inputPhaseMod]->getDouble() * phaseModCorrection);         \
  sidePhaseMod.METHOD(pv[ID::sideChainPhaseMod]->getDouble() * phaseModCorrection);      \
                                                                                         \
  auto inPreAsym = pv[ID::inputPreAsymmetryAmount]->getDouble();                         \
  inputPreAsymmetry.METHOD(                                                              \
    pv[ID::inputPreAsymmetryHarsh]->getInt() ? freqToSvfG(inPreAsym / double(2))         \
                                             : inPreAsym);                               \
  auto sidePreAsym = pv[ID::sideChainPreAsymmetryAmount]->getDouble();                   \
  sidePreAsymmetry.METHOD(                                                               \
    pv[ID::sideChainPreAsymmetryHarsh]->getInt() ? freqToSvfG(sidePreAsym / double(2))   \
                                                 : sidePreAsym);                         \
                                                                                         \
  inputLowpassG.METHOD(freqToSvfG(pv[ID::inputLowpassHz]->getDouble() / upRate));        \
  sideLowpassG.METHOD(freqToSvfG(pv[ID::sideChainLowpassHz]->getDouble() / upRate));     \
  inputHighpassG.METHOD(freqToSvfG(pv[ID::inputHighpassHz]->getDouble() / upRate));      \
  sideHighpassG.METHOD(freqToSvfG(pv[ID::sideChainHighpassHz]->getDouble() / upRate));   \
                                                                                         \
  auto inPostAsym = pv[ID::inputPostAsymmetryAmount]->getDouble();                       \
  inputPostAsymmetry.METHOD(                                                             \
    pv[ID::inputPostAsymmetryHarsh]->getInt() ? freqToSvfG(inPostAsym / double(2))       \
                                              : inPostAsym);                             \
  auto sidePostAsym = pv[ID::sideChainPostAsymmetryAmount]->getDouble();                 \
  sidePostAsymmetry.METHOD(                                                              \
    pv[ID::sideChainPostAsymmetryHarsh]->getInt() ? freqToSvfG(sidePostAsym / double(2)) \
                                                  : sidePostAsym);                       \
                                                                                         \
  auto inputReleaseSamples = upRate * pv[ID::inputEnvelopeReleaseSecond]->getDouble();   \
  auto sideReleaseSamples                                                                \
    = upRate * pv[ID::sideChainEnvelopeReleaseSecond]->getDouble();                      \
  inputEnvelope[0].prepare(inputReleaseSamples);                                         \
  inputEnvelope[1].prepare(inputReleaseSamples);                                         \
  inputEnvelope[2].prepare(sideReleaseSamples);                                          \
  inputEnvelope[3].prepare(sideReleaseSamples);

void DSPCore::updateUpRate()
{
  upRate = double(sampleRate) * fold[oversampling];

  SmootherCommon<double>::setSampleRate(upRate);

  for (auto &x : inputGate) x.setup(upRate, double(0.001));
}

void DSPCore::reset()
{
  oversampling = param.value[ParameterID::ID::oversampling]->getInt();
  updateUpRate();

  ASSIGN_PARAMETER(reset);

  enableInputEnvelope = pv[ID::inputEnvelopeEnable]->getInt();
  enableSideEnvelope = pv[ID::sideChainEnvelopeEnable]->getInt();

  phase.fill({});
  for (auto &x : inputLowpass) x.reset();
  for (auto &x : inputHighpass) x.reset();
  for (auto &x : inputGate) x.reset();
  for (auto &x : inputEnvelope) x.reset();

  for (auto &x : upSampler) x.reset();
  for (auto &x : decimationLowpass) x.reset();
  for (auto &x : halfbandIir) x.reset();

  startup();
}

void DSPCore::startup() {}

void DSPCore::setParameters()
{
  size_t newOversampling = param.value[ParameterID::ID::oversampling]->getInt();
  if (oversampling != newOversampling) {
    oversampling = newOversampling;
    updateUpRate();
  }

  ASSIGN_PARAMETER(push);

  auto inputGateThreshold = pv[ID::inputGateThreshold]->getDouble();
  inputGate[0].prepare(upRate, inputGateThreshold);
  inputGate[1].prepare(upRate, inputGateThreshold);
  auto sideGateThreshold = pv[ID::sideChainGateThreshold]->getDouble();
  inputGate[2].prepare(upRate, sideGateThreshold);
  inputGate[3].prepare(upRate, sideGateThreshold);

  enableInputEnvelope = pv[ID::inputEnvelopeEnable]->getInt();
  if (!enableInputEnvelope) {
    inputEnvelope[0].reset();
    inputEnvelope[1].reset();
  }
  enableSideEnvelope = pv[ID::sideChainEnvelopeEnable]->getInt();
  if (!enableSideEnvelope) {
    inputEnvelope[2].reset();
    inputEnvelope[3].reset();
  }
}

std::array<double, 2> DSPCore::processFrame(const std::array<double, 4> &frame)
{
  outputGain.process();
  mix.process();

  stereoPhaseLinkKp.process();
  stereoPhaseCross.process();
  stereoPhaseOffset.process();
  phaseWarp.process();

  inputPhaseMod.process();
  inputPreAsymmetry.process();
  inputLowpassG.process();
  inputHighpassG.process();
  inputPostAsymmetry.process();

  sidePhaseMod.process();
  sidePreAsymmetry.process();
  sideLowpassG.process();
  sideHighpassG.process();
  sidePostAsymmetry.process();

  auto sig0 = frame[0]; // Main L.
  auto sig1 = frame[1]; // Main R.
  auto sig2 = frame[2]; // Side L.
  auto sig3 = frame[3]; // Side R.
  sig0 = lerp(sig0, std::abs(sig0), inputPreAsymmetry.getValue());
  sig1 = lerp(sig1, std::abs(sig1), inputPreAsymmetry.getValue());
  sig2 = lerp(sig2, std::abs(sig2), sidePreAsymmetry.getValue());
  sig3 = lerp(sig3, std::abs(sig3), sidePreAsymmetry.getValue());
  sig0 = inputLowpass[0].lowpass(sig0, inputLowpassG.getValue());
  sig1 = inputLowpass[1].lowpass(sig1, inputLowpassG.getValue());
  sig2 = inputLowpass[2].lowpass(sig2, sideLowpassG.getValue());
  sig3 = inputLowpass[3].lowpass(sig3, sideLowpassG.getValue());
  sig0 = inputHighpass[0].highpass(sig0, inputHighpassG.getValue());
  sig1 = inputHighpass[1].highpass(sig1, inputHighpassG.getValue());
  sig2 = inputHighpass[2].highpass(sig2, sideHighpassG.getValue());
  sig3 = inputHighpass[3].highpass(sig3, sideHighpassG.getValue());
  sig0 *= inputGate[0].process(std::abs(sig0));
  sig1 *= inputGate[1].process(std::abs(sig1));
  sig2 *= inputGate[2].process(std::abs(sig2));
  sig3 *= inputGate[3].process(std::abs(sig3));
  sig0 = lerp(sig0, std::abs(sig0), inputPostAsymmetry.getValue());
  sig1 = lerp(sig1, std::abs(sig1), inputPostAsymmetry.getValue());
  sig2 = lerp(sig2, std::abs(sig2), sidePostAsymmetry.getValue());
  sig3 = lerp(sig3, std::abs(sig3), sidePostAsymmetry.getValue());

  if (enableInputEnvelope) {
    sig0 = inputEnvelope[0].process(sig0);
    sig1 = inputEnvelope[1].process(sig1);
  }
  if (enableSideEnvelope) {
    sig2 = inputEnvelope[2].process(sig2);
    sig3 = inputEnvelope[3].process(sig3);
  }

  syncPhase(phase[0], phase[1], stereoPhaseLinkKp.getValue());

  phase[0] += inputPhaseMod.getValue() * sig0 + sidePhaseMod.getValue() * sig2;
  phase[1] += inputPhaseMod.getValue() * sig1 + sidePhaseMod.getValue() * sig3;
  phase[0] -= std::floor(phase[0]);
  phase[1] -= std::floor(phase[1]);
  if (phaseWarp.getValue() > std::numeric_limits<double>::epsilon()) {
    phase[0] *= lerp(double(1), std::pow(phase[0], 48000 / upRate), phaseWarp.getValue());
    phase[1] *= lerp(double(1), std::pow(phase[1], 48000 / upRate), phaseWarp.getValue());
  }

  auto mod0 = std::cos(double(twopi) * (phase[0] + stereoPhaseOffset.getValue()));
  auto mod1 = std::cos(double(twopi) * (phase[1] - stereoPhaseOffset.getValue()));

  auto rot0 = frame[0] * lerp(mod0, mod1, stereoPhaseCross.getValue());
  auto rot1 = frame[1] * lerp(mod1, mod0, stereoPhaseCross.getValue());

  return {
    outputGain.process() * lerp(frame[0], rot0, mix.getValue()),
    outputGain.process() * lerp(frame[1], rot1, mix.getValue()),
  };
}

void DSPCore::process(
  const size_t length,
  const float *in0,
  const float *in1,
  const float *in2,
  const float *in3,
  float *out0,
  float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);
    upSampler[2].process(in2[i]);
    upSampler[3].process(in3[i]);

    if (oversampling == 2) { // 16x.
      for (size_t j = 0; j < upFold; ++j) {
        auto frame = processFrame({
          upSampler[0].output[j],
          upSampler[1].output[j],
          upSampler[2].output[j],
          upSampler[3].output[j],
        });
        decimationLowpass[0].push(frame[0]);
        decimationLowpass[1].push(frame[1]);
        upSampler[0].output[j] = decimationLowpass[0].output();
        upSampler[1].output[j] = decimationLowpass[1].output();
      }
      out0[i] = halfbandIir[0].process(
        {upSampler[0].output[0], upSampler[0].output[upFold / 2]});
      out1[i] = halfbandIir[1].process(
        {upSampler[1].output[0], upSampler[1].output[upFold / 2]});
    } else if (oversampling == 1) { // 2x.
      constexpr size_t mid = upFold / 2;
      for (size_t j = 0; j < upFold; j += mid) {
        auto frame = processFrame({
          upSampler[0].output[j],
          upSampler[1].output[j],
          upSampler[2].output[j],
          upSampler[3].output[j],
        });
        upSampler[0].output[j] = frame[0];
        upSampler[1].output[j] = frame[1];
      }
      out0[i]
        = halfbandIir[0].process({upSampler[0].output[0], upSampler[0].output[mid]});
      out1[i]
        = halfbandIir[1].process({upSampler[1].output[0], upSampler[1].output[mid]});
    } else { // 1x.
      auto frame = processFrame({
        upSampler[0].output[0],
        upSampler[1].output[0],
        upSampler[2].output[0],
        upSampler[3].output[0],
      });
      out0[i] = frame[0];
      out1[i] = frame[1];
    }
  }
}
