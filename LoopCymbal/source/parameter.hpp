// (c) 2023 Takamitsu Endo
//
// This file is part of LoopCymbal.
//
// LoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr size_t nAllpass = 16;

constexpr size_t nReservedParameter = 64;
constexpr size_t nReservedGuiParameter = 16;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  overSampling,
  resetSeedAtNoteOn,
  release,

  stereoBalance,
  stereoMerge,

  useExternalInput,
  externalInputGain,

  notePitchAmount,
  tuningCent,
  pitchBend,
  pitchBendRange,
  noteSlideTimeSecond,

  seed,
  noiseDecaySeconds,

  delayTimeBaseSecond,
  delayTimeRandomSecond,
  delayTimeModAmount,
  allpassFeed1,
  allpassFeed2,
  allpassMixSpike,
  allpassMixAltSign,

  highShelfFrequencyHz,
  highShelfGain,
  lowShelfFrequencyHz,
  lowShelfGain,

  reservedParameter0,
  reservedGuiParameter0 = reservedParameter0 + nReservedParameter,

  ID_ENUM_LENGTH = reservedGuiParameter0 + nReservedGuiParameter,
  // ID_ENUM_GUI_START = reservedGuiParameter0,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::DecibelScale<double> noteSlideTimeSecond;

  static SomeDSP::DecibelScale<double> noiseDecaySeconds;
  static SomeDSP::DecibelScale<double> delayTimeSecond;
  static SomeDSP::DecibelScale<double> delayTimeModAmount;

  static SomeDSP::DecibelScale<double> cutoffFrequencyHz;
  static SomeDSP::DecibelScale<double> shelvingGain;

  static SomeDSP::DecibelScale<double> hihatDistance;
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
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);
    value[ID::resetSeedAtNoteOn] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "resetSeedAtNoteOn", Info::kCanAutomate);
    value[ID::release]
      = std::make_unique<UIntValue>(1, Scales::boolScale, "release", Info::kCanAutomate);

    value[ID::stereoBalance] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "stereoBalance",
      Info::kCanAutomate);
    value[ID::stereoMerge] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.75), Scales::defaultScale, "stereoMerge",
      Info::kCanAutomate);

    value[ID::useExternalInput] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "useExternalInput", Info::kCanAutomate);
    value[ID::externalInputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "externalInputGain", Info::kCanAutomate);

    value[ID::notePitchAmount] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "notePitchAmount",
      Info::kCanAutomate);
    value[ID::tuningCent] = std::make_unique<LinearValue>(
      Scales::cent.invmap(0.0), Scales::cent, "tuningCent", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::cent.invmap(1200.0), Scales::cent, "pitchBendRange", Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.0001), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::noiseDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(0.001), Scales::noiseDecaySeconds,
      "noiseDecaySeconds", Info::kCanAutomate);
    value[ID::delayTimeBaseSecond] = std::make_unique<DecibelValue>(
      Scales::delayTimeSecond.invmap(0.001), Scales::delayTimeSecond,
      "delayTimeBaseSecond", Info::kCanAutomate);
    value[ID::delayTimeRandomSecond] = std::make_unique<DecibelValue>(
      Scales::delayTimeSecond.invmap(0.001), Scales::delayTimeSecond,
      "delayTimeRandomSecond", Info::kCanAutomate);

    value[ID::delayTimeModAmount] = std::make_unique<DecibelValue>(
      Scales::delayTimeModAmount.invmap(0.0), Scales::delayTimeModAmount,
      "delayTimeModAmount", Info::kCanAutomate);
    value[ID::allpassFeed1] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.98), Scales::bipolarScale, "allpassFeed1",
      Info::kCanAutomate);
    value[ID::allpassFeed2] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.98), Scales::bipolarScale, "allpassFeed2",
      Info::kCanAutomate);
    value[ID::allpassMixSpike] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(2.0 / 3.0), Scales::defaultScale, "allpassMixSpike",
      Info::kCanAutomate);
    value[ID::allpassMixAltSign] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "allpassMixAltSign",
      Info::kCanAutomate);

    value[ID::highShelfFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::cutoffFrequencyHz.invmap(8000.0), Scales::cutoffFrequencyHz,
      "highShelfFrequencyHz", Info::kCanAutomate);
    value[ID::highShelfGain] = std::make_unique<DecibelValue>(
      Scales::shelvingGain.invmapDB(-1.0), Scales::shelvingGain, "highShelfGain",
      Info::kCanAutomate);
    value[ID::lowShelfFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::cutoffFrequencyHz.invmap(20.0), Scales::cutoffFrequencyHz,
      "lowShelfFrequencyHz", Info::kCanAutomate);
    value[ID::lowShelfGain] = std::make_unique<DecibelValue>(
      Scales::shelvingGain.invmapDB(-1.0), Scales::shelvingGain, "lowShelfGain",
      Info::kCanAutomate);

    for (size_t idx = 0; idx < nReservedParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedParameter" + indexStr).c_str(), Info::kIsHidden);
    }

    for (size_t idx = 0; idx < nReservedGuiParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedGuiParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedGuiParameter" + indexStr).c_str(), Info::kIsHidden);
    }

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
