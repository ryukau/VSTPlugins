// (c) 2021-2022 Takamitsu Endo
//
// This file is part of MembraneSynth.
//
// MembraneSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MembraneSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MembraneSynth.  If not, see <https://www.gnu.org/licenses/>.

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
  gainNormalization,
  overSampling,

  pulseAmplitude,
  pulseDecaySeconds,
  impactCenterRimPosition,

  fdnMatrixIdentityAmount,
  fdnFeedback,
  fdnModulationAmount,
  fdnInterpRate,
  fdnMaxModulationRatio,
  fdnShape,

  fdnSeed,
  fdnRandomMatrix,
  fdnRandomOvertone,

  fdnLowpassCutoffHz,
  fdnHighpassCutoffHz,
  fdnLowpassQ0,
  fdnHighpassQ0 = 34, // fdnLowpassQ0 + fdnSize,

  enableModEnv = 50, // fdnHighpassQ0 + fdnSize,
  modEnvSustainSeconds,
  modEnvReleaseSeconds,

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
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> outputGain;

  static SomeDSP::DecibelScale<double> pulseAmplitude;
  static SomeDSP::DecibelScale<double> pulseDecaySeconds;

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
    value[ID::gainNormalization] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "gainNormalization", Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);

    value[ID::pulseAmplitude] = std::make_unique<DecibelValue>(
      Scales::pulseAmplitude.invmap(8.0), Scales::pulseAmplitude, "pulseAmplitude",
      Info::kCanAutomate);
    value[ID::pulseDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::pulseDecaySeconds.invmap(0.01), Scales::pulseDecaySeconds,
      "pulseDecaySeconds", Info::kCanAutomate);
    value[ID::impactCenterRimPosition] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "impactCenterRimPosition", Info::kCanAutomate);

    value[ID::fdnMatrixIdentityAmount] = std::make_unique<DecibelValue>(
      Scales::fdnMatrixIdentityAmount.invmap(0.5), Scales::fdnMatrixIdentityAmount,
      "fdnMatrixIdentityAmount", Info::kCanAutomate);
    value[ID::fdnFeedback] = std::make_unique<NegativeDecibelValue>(
      Scales::fdnFeedback.invmap(0.96), Scales::fdnFeedback, "fdnFeedback",
      Info::kCanAutomate);
    value[ID::fdnModulationAmount] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "fdnModulationAmount", Info::kCanAutomate);
    value[ID::fdnInterpRate] = std::make_unique<DecibelValue>(
      0.5, Scales::fdnInterpRate, "fdnInterpRate", Info::kCanAutomate);
    value[ID::fdnMaxModulationRatio] = std::make_unique<LinearValue>(
      Scales::fdnMaxModulationRatio.invmap(0.33), Scales::fdnMaxModulationRatio,
      "fdnMaxModulationRatio", Info::kCanAutomate);
    value[ID::fdnShape] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "fdnShape", Info::kCanAutomate);

    value[ID::fdnSeed]
      = std::make_unique<UIntValue>(0, Scales::seed, "fdnSeed", Info::kCanAutomate);
    value[ID::fdnRandomMatrix] = std::make_unique<LinearValue>(
      0.001, Scales::defaultScale, "fdnRandomMatrix", Info::kCanAutomate);
    value[ID::fdnRandomOvertone] = std::make_unique<LinearValue>(
      0.001, Scales::defaultScale, "fdnRandomOvertone", Info::kCanAutomate);

    value[ID::fdnLowpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(2000), Scales::filterCutoffHz, "fdnLowpassCutoffHz",
      Info::kCanAutomate);
    value[ID::fdnHighpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::filterCutoffHz.invmap(20), Scales::filterCutoffHz, "fdnHighpassCutoffHz",
      Info::kCanAutomate);

    std::string fdnLowpassQLabel("fdnLowpassQ");
    std::string fdnHighpassQLabel("fdnHighpassQ");
    for (size_t idx = 0; idx < fdnSize; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::fdnLowpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (fdnLowpassQLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::fdnHighpassQ0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQ.invmap(0.7), Scales::filterQ,
        (fdnHighpassQLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::enableModEnv] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "enableModEnv", Info::kCanAutomate);
    value[ID::modEnvSustainSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.001), Scales::envelopeSeconds,
      "modEnvSustainSeconds", Info::kCanAutomate);
    value[ID::modEnvReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(1.0), Scales::envelopeSeconds,
      "modEnvReleaseSeconds", Info::kCanAutomate);

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
