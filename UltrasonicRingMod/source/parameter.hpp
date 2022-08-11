// (c) 2021-2022 Takamitsu Endo
//
// This file is part of UltrasonicRingMod.
//
// UltrasonicRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltrasonicRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltrasonicRingMod.  If not, see <https://www.gnu.org/licenses/>.

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

  mix,
  preClipGain,
  outputGain,
  frequencyHz,
  dcOffset,
  feedbackGain,
  modFrequencyScaling,
  modWrapMix,
  hardclipMix,

  noteScaling,
  noteScalingNegative,
  noteOffset,
  noteSlideTimeSecond,

  parameterSmoothingSecond,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> frequencyHz;
  static SomeDSP::DecibelScale<double> feedbackGain;
  static SomeDSP::DecibelScale<double> modFrequencyScaling;
  static SomeDSP::NegativeDecibelScale<double> modWrapMix;

  static SomeDSP::LinearScale<double> noteScaling;
  static SomeDSP::LinearScale<double> noteOffset;
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

    value[ID::mix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "mix", Info::kCanAutomate);
    value[ID::preClipGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "preClipGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::frequencyHz] = std::make_unique<DecibelValue>(
      Scales::frequencyHz.invmap(10000.0), Scales::frequencyHz, "frequencyHz",
      Info::kCanAutomate);
    value[ID::dcOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "dcOffset", Info::kCanAutomate);
    value[ID::feedbackGain] = std::make_unique<DecibelValue>(
      Scales::feedbackGain.invmap(1.0), Scales::feedbackGain, "feedbackGain",
      Info::kCanAutomate);

    value[ID::modFrequencyScaling] = std::make_unique<DecibelValue>(
      0.0, Scales::modFrequencyScaling, "modFrequencyScaling", Info::kCanAutomate);
    value[ID::modWrapMix] = std::make_unique<NegativeDecibelValue>(
      0.0, Scales::modWrapMix, "modWrapMix", Info::kCanAutomate);
    value[ID::hardclipMix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "hardclipMix", Info::kCanAutomate);

    value[ID::noteScaling] = std::make_unique<LinearValue>(
      Scales::noteScaling.invmap(1.0), Scales::noteScaling, "noteScaling",
      Info::kCanAutomate);
    value[ID::noteScalingNegative] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "noteScalingNegative", Info::kCanAutomate);
    value[ID::noteOffset] = std::make_unique<LinearValue>(
      Scales::noteOffset.invmap(0.0), Scales::noteOffset, "noteOffset",
      Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.02), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.2), Scales::noteSlideTimeSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);

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
