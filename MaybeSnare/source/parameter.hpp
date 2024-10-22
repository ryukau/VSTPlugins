// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr int octaveOffset = 8;
constexpr int semitoneOffset = 96;
constexpr int a4HzOffset = 100;
constexpr size_t fdnSize = 16;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  fdnMix,
  gainNormalization,
  overSampling,

  impactAmplitude,
  impactDecaySeconds,
  impactCenterRimPosition,
  impactNoiseMix,
  impactNoiseDecaySeconds,
  impactNoiseLowpassHz,

  snareSideOctaveOffset,
  couplingAmount,
  couplingDecaySeconds,
  couplingSafetyReduction,

  batterFdnMatrixIdentityAmount,
  batterFdnFeedback,
  batterFdnShape,
  batterFdnModulationAmount,
  batterFdnInterpRate,
  batterFdnMaxModulationRatio,
  batterModEnvSustainSeconds,
  batterModEnvReleaseSeconds,
  batterFdnLowpassCutoffHz,
  batterFdnHighpassCutoffHz,
  batterFdnLowpassQ0,
  batterFdnHighpassQ0 = 41, // batterFdnLowpassQ0 + fdnSize,

  snareFdnMatrixIdentityAmount = 57, // batterFdnHighpassQ0 + fdnSize,
  snareFdnFeedback,
  snareFdnShape,
  snareFdnModulationAmount,
  snareFdnInterpRate,
  snareFdnMaxModulationRatio,
  snareModEnvSustainSeconds,
  snareModEnvReleaseSeconds,
  snareFdnLowpassCutoffHz,
  snareFdnHighpassCutoffHz,
  snareFdnLowpassQ0,
  snareFdnHighpassQ0 = 83, // snareFdnLowpassQ0 + fdnSize,

  fdnSeed = 99, // snareFdnHighpassQ0 + fdnSize,
  fdnRandomMatrix,
  fdnRandomOvertone,

  enableBatterModEnv,
  enableSnareModEnv,

  tuningSemitone,
  tuningCent,
  tuningET,
  tuningA4Hz,
  pitchBend,
  pitchBendRange,
  noteSlideTimeSecond,
  slideAtNoteOn,
  slideAtNoteOff,

  ID_ENUM_LENGTH,
  // ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> outputGain;

  static SomeDSP::DecibelScale<double> impactAmplitude;
  static SomeDSP::DecibelScale<double> impactDecaySeconds;

  static SomeDSP::LinearScale<double> snareSideOctaveOffset;
  static SomeDSP::DecibelScale<double> couplingAmount;
  static SomeDSP::DecibelScale<double> couplingDecaySeconds;

  static SomeDSP::DecibelScale<double> fdnMatrixIdentityAmount;
  static SomeDSP::NegativeDecibelScale<double> fdnFeedback;
  static SomeDSP::DecibelScale<double> fdnInterpRate;
  static SomeDSP::LinearScale<double> fdnMaxModulationRatio;
  static SomeDSP::DecibelScale<double> filterCutoffHz;
  static SomeDSP::LinearScale<double> filterQ;

  static SomeDSP::DecibelScale<double> envelopeSeconds;

  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::UIntScale<double> equalTemperament;
  static SomeDSP::UIntScale<double> a4Hz;
  static SomeDSP::LinearScale<double> pitchBendRange;

  static SomeDSP::DecibelScale<double> noteSlideTimeSecond;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using NegativeDecibelValue = DoubleValue<SomeDSP::NegativeDecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::outputGain.invmap(1.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::fdnMix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "fdnMix", Info::kCanAutomate);
    value[ID::gainNormalization] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "gainNormalization", Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);

    value[ID::impactAmplitude] = std::make_unique<DecibelValue>(
      Scales::impactAmplitude.invmap(10.0), Scales::impactAmplitude, "impactAmplitude",
      Info::kCanAutomate);
    value[ID::impactDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::impactDecaySeconds.invmap(0.01), Scales::impactDecaySeconds,
      "impactDecaySeconds", Info::kCanAutomate);
    value[ID::impactCenterRimPosition] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "impactCenterRimPosition", Info::kCanAutomate);
    value[ID::impactNoiseMix] = std::make_unique<LinearValue>(
      0.05, Scales::defaultScale, "impactNoiseMix", Info::kCanAutomate);
    value[ID::impactNoiseDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::impactDecaySeconds.invmap(0.01), Scales::impactDecaySeconds,
      "impactNoiseDecaySeconds", Info::kCanAutomate);
    value[ID::impactNoiseLowpassHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(500.0), Scales::filterCutoffHz,
      "impactNoiseLowpassHz", Info::kCanAutomate);

    value[ID::snareSideOctaveOffset] = std::make_unique<LinearValue>(
      Scales::snareSideOctaveOffset.invmap(-1.6), Scales::snareSideOctaveOffset,
      "snareSideOctaveOffset", Info::kCanAutomate);
    value[ID::couplingAmount] = std::make_unique<DecibelValue>(
      Scales::couplingAmount.invmap(2.0), Scales::couplingAmount, "couplingAmount",
      Info::kCanAutomate);
    value[ID::couplingDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::couplingDecaySeconds.invmap(1.0), Scales::couplingDecaySeconds,
      "couplingDecaySeconds", Info::kCanAutomate);
    value[ID::couplingSafetyReduction] = std::make_unique<NegativeDecibelValue>(
      Scales::fdnFeedback.invmap(0.99), Scales::fdnFeedback, "couplingSafetyReduction",
      Info::kCanAutomate);

    value[ID::batterFdnMatrixIdentityAmount] = std::make_unique<DecibelValue>(
      Scales::fdnMatrixIdentityAmount.invmap(0.5), Scales::fdnMatrixIdentityAmount,
      "batterFdnMatrixIdentityAmount", Info::kCanAutomate);
    value[ID::batterFdnFeedback] = std::make_unique<NegativeDecibelValue>(
      Scales::fdnFeedback.invmap(0.92), Scales::fdnFeedback, "batterFdnFeedback",
      Info::kCanAutomate);
    value[ID::batterFdnShape] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "batterFdnShape", Info::kCanAutomate);
    value[ID::batterFdnModulationAmount] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "batterFdnModulationAmount", Info::kCanAutomate);
    value[ID::batterFdnInterpRate] = std::make_unique<DecibelValue>(
      Scales::fdnInterpRate.invmap(1.0), Scales::fdnInterpRate, "batterFdnInterpRate",
      Info::kCanAutomate);
    value[ID::batterFdnMaxModulationRatio] = std::make_unique<LinearValue>(
      Scales::fdnMaxModulationRatio.invmap(0.33), Scales::fdnMaxModulationRatio,
      "batterFdnMaxModulationRatio", Info::kCanAutomate);
    value[ID::batterModEnvSustainSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.0), Scales::envelopeSeconds,
      "batterModEnvSustainSeconds", Info::kCanAutomate);
    value[ID::batterModEnvReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(1.0), Scales::envelopeSeconds,
      "batterModEnvReleaseSeconds", Info::kCanAutomate);
    value[ID::batterFdnLowpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(4000), Scales::filterCutoffHz,
      "batterFdnLowpassCutoffHz", Info::kCanAutomate);
    value[ID::batterFdnHighpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(90), Scales::filterCutoffHz,
      "batterFdnHighpassCutoffHz", Info::kCanAutomate);
    std::string batterFdnLowpassQLabel("batterFdnLowpassQ");
    std::string batterFdnHighpassQLabel("batterFdnHighpassQ");
    for (size_t idx = 0; idx < fdnSize; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::batterFdnLowpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (batterFdnLowpassQLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::batterFdnHighpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (batterFdnHighpassQLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::snareFdnMatrixIdentityAmount] = std::make_unique<DecibelValue>(
      Scales::fdnMatrixIdentityAmount.invmap(3.3), Scales::fdnMatrixIdentityAmount,
      "snareFdnMatrixIdentityAmount", Info::kCanAutomate);
    value[ID::snareFdnFeedback] = std::make_unique<NegativeDecibelValue>(
      Scales::fdnFeedback.invmap(0.96), Scales::fdnFeedback, "snareFdnFeedback",
      Info::kCanAutomate);
    value[ID::snareFdnShape] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "snareFdnShape", Info::kCanAutomate);
    value[ID::snareFdnModulationAmount] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "snareFdnModulationAmount", Info::kCanAutomate);
    value[ID::snareFdnInterpRate] = std::make_unique<DecibelValue>(
      Scales::fdnInterpRate.invmap(5.2), Scales::fdnInterpRate, "snareFdnInterpRate",
      Info::kCanAutomate);
    value[ID::snareFdnMaxModulationRatio] = std::make_unique<LinearValue>(
      Scales::fdnMaxModulationRatio.invmap(0.25), Scales::fdnMaxModulationRatio,
      "snareFdnMaxModulationRatio", Info::kCanAutomate);
    value[ID::snareModEnvSustainSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.0), Scales::envelopeSeconds,
      "snareModEnvSustainSeconds", Info::kCanAutomate);
    value[ID::snareModEnvReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(1.0), Scales::envelopeSeconds,
      "snareModEnvReleaseSeconds", Info::kCanAutomate);
    value[ID::snareFdnLowpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(10000), Scales::filterCutoffHz,
      "snareFdnLowpassCutoffHz", Info::kCanAutomate);
    value[ID::snareFdnHighpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(60), Scales::filterCutoffHz,
      "snareFdnHighpassCutoffHz", Info::kCanAutomate);
    std::string snareFdnLowpassQLabel("snareFdnLowpassQ");
    std::string snareFdnHighpassQLabel("snareFdnHighpassQ");
    for (size_t idx = 0; idx < fdnSize; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::snareFdnLowpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (snareFdnLowpassQLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::snareFdnHighpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (snareFdnHighpassQLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::fdnSeed]
      = std::make_unique<UIntValue>(0, Scales::seed, "fdnSeed", Info::kCanAutomate);
    value[ID::fdnRandomMatrix] = std::make_unique<LinearValue>(
      0.001, Scales::defaultScale, "fdnRandomMatrix", Info::kCanAutomate);
    value[ID::fdnRandomOvertone] = std::make_unique<LinearValue>(
      0.001, Scales::defaultScale, "fdnRandomOvertone", Info::kCanAutomate);

    value[ID::enableBatterModEnv] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "enableBatterModEnv", Info::kCanAutomate);
    value[ID::enableSnareModEnv] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "enableSnareModEnv", Info::kCanAutomate);

    value[ID::tuningSemitone] = std::make_unique<UIntValue>(
      semitoneOffset - 24, Scales::semitone, "tuningSemitone", Info::kCanAutomate);
    value[ID::tuningCent] = std::make_unique<LinearValue>(
      Scales::cent.invmap(0.0), Scales::cent, "tuningCent", Info::kCanAutomate);
    value[ID::tuningET] = std::make_unique<UIntValue>(
      11, Scales::equalTemperament, "tuningET", Info::kCanAutomate);
    value[ID::tuningA4Hz] = std::make_unique<UIntValue>(
      440 - a4HzOffset, Scales::a4Hz, "tuningA4Hz", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::pitchBendRange.invmap(2.0), Scales::pitchBendRange, "pitchBendRange",
      Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.0), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);
    value[ID::slideAtNoteOn] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "slideAtNoteOn", Info::kCanAutomate);
    value[ID::slideAtNoteOff] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "slideAtNoteOff", Info::kCanAutomate);

    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
  }

#ifdef TEST_DSP
  // Not used in DSP test.
  double getDefaultNormalized(int32_t) { return 0.0; }

#else
  tresult setState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->setState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult getState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->getState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters)
  {
    for (auto &val : value)
      if (val->addParameter(parameters)) return kResultFalse;
    return kResultOk;
  }

  double getDefaultNormalized(int32_t tag) override
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
#endif
};

} // namespace Synth
} // namespace Steinberg
