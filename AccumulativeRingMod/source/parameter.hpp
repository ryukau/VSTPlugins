// (c) 2023 Takamitsu Endo
//
// This file is part of AccumulativeRingMod.
//
// AccumulativeRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AccumulativeRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with AccumulativeRingMod.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  mix,

  stereoPhaseLinkHz,
  stereoPhaseCross,
  stereoPhaseOffset,
  phaseWarp,

  inputPhaseMod,
  inputLowpassHz,
  inputHighpassHz,
  inputGateThreshold,
  inputEnvelopeEnable,
  inputEnvelopeReleaseSecond,
  inputPreAsymmetryAmount,
  inputPostAsymmetryAmount,
  inputPreAsymmetryHarsh,
  inputPostAsymmetryHarsh,

  sideChainPhaseMod,
  sideChainLowpassHz,
  sideChainHighpassHz,
  sideChainGateThreshold,
  sideChainEnvelopeEnable,
  sideChainEnvelopeReleaseSecond,
  sideChainPreAsymmetryAmount,
  sideChainPostAsymmetryAmount,
  sideChainPreAsymmetryHarsh,
  sideChainPostAsymmetryHarsh,

  parameterSmoothingSecond,
  oversampling,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> outputGain;
  static SomeDSP::DecibelScale<double> stereoPhaseLinkHz;
  static SomeDSP::DecibelScale<double> modulation;
  static SomeDSP::DecibelScale<double> cutoffHz;
  static SomeDSP::DecibelScale<double> gateThreshold;
  static SomeDSP::DecibelScale<double> envelopeSecond;

  static SomeDSP::UIntScale<double> oversampling;
  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
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
      Scales::outputGain.invmapDB(0.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::mix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "mix", Info::kCanAutomate);

    value[ID::stereoPhaseLinkHz] = std::make_unique<DecibelValue>(
      Scales::stereoPhaseLinkHz.invmap(1.0), Scales::stereoPhaseLinkHz, "stereoLink",
      Info::kCanAutomate);
    value[ID::stereoPhaseCross] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "stereoPhaseCross", Info::kCanAutomate);
    value[ID::stereoPhaseOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "stereoPhaseOffset", Info::kCanAutomate);
    value[ID::phaseWarp] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "phaseWarp", Info::kCanAutomate);

    value[ID::inputPhaseMod] = std::make_unique<DecibelValue>(
      Scales::modulation.invmap(0.01), Scales::modulation, "inputPhaseMod",
      Info::kCanAutomate);
    value[ID::inputLowpassHz] = std::make_unique<DecibelValue>(
      1.0, Scales::cutoffHz, "inputLowpassHz", Info::kCanAutomate);
    value[ID::inputHighpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(5.0), Scales::cutoffHz, "inputHighpassHz",
      Info::kCanAutomate);
    value[ID::inputGateThreshold] = std::make_unique<DecibelValue>(
      0.0, Scales::gateThreshold, "inputGateThreshold", Info::kCanAutomate);
    value[ID::inputEnvelopeEnable] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "inputEnvelopeEnable", Info::kCanAutomate);
    value[ID::inputEnvelopeReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.5), Scales::envelopeSecond,
      "inputEnvelopeReleaseSecond", Info::kCanAutomate);
    value[ID::inputPreAsymmetryAmount] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "inputPreAsymmetryAmount", Info::kCanAutomate);
    value[ID::inputPostAsymmetryAmount] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "inputPostAsymmetryAmount", Info::kCanAutomate);
    value[ID::inputPreAsymmetryHarsh] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "inputPreAsymmetryHarsh", Info::kCanAutomate);
    value[ID::inputPostAsymmetryHarsh] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "inputPostAsymmetryHarsh", Info::kCanAutomate);

    value[ID::sideChainPhaseMod] = std::make_unique<DecibelValue>(
      Scales::modulation.invmap(0.0), Scales::modulation, "sideChainPhaseMod",
      Info::kCanAutomate);
    value[ID::sideChainLowpassHz] = std::make_unique<DecibelValue>(
      1.0, Scales::cutoffHz, "sideChainLowpassHz", Info::kCanAutomate);
    value[ID::sideChainHighpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(5.0), Scales::cutoffHz, "sideChainHighpassHz",
      Info::kCanAutomate);
    value[ID::sideChainGateThreshold] = std::make_unique<DecibelValue>(
      0.0, Scales::gateThreshold, "sideChainGateThreshold", Info::kCanAutomate);
    value[ID::sideChainEnvelopeEnable] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "sideChainEnvelopeEnable", Info::kCanAutomate);
    value[ID::sideChainEnvelopeReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.5), Scales::envelopeSecond,
      "sideChainEnvelopeReleaseSecond", Info::kCanAutomate);
    value[ID::sideChainPreAsymmetryAmount] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "sideChainPreAsymmetryAmount", Info::kCanAutomate);
    value[ID::sideChainPostAsymmetryAmount] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "sideChainPostAsymmetryAmount", Info::kCanAutomate);
    value[ID::sideChainPreAsymmetryHarsh] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "sideChainPreAsymmetryHarsh", Info::kCanAutomate);
    value[ID::sideChainPostAsymmetryHarsh] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "sideChainPostAsymmetryHarsh", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.2), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);
    value[ID::oversampling] = std::make_unique<UIntValue>(
      2, Scales::oversampling, "oversampling", Info::kCanAutomate);

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
