// (c) 2020 Takamitsu Endo
//
// This file is part of SoftClipper.
//
// SoftClipper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoftClipper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoftClipper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  inputGain,
  outputGain,
  clip,
  ratio,
  slope,
  orderInteger,
  orderFraction,

  oversample,

  smoothness,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , inputGain(0.0, 4.0, 0.5, 1.0)
    , outputGain(0.0, 2.0, 0.5, 0.2)
    , clip(0.0, 32.0, 0.5, 4.0)
    , orderInteger(16)
    , smoothness(0.0, 0.5, 0.1, 0.04)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> inputGain;
  SomeDSP::LogScale<double> outputGain;
  SomeDSP::LogScale<double> clip;
  SomeDSP::UIntScale<double> orderInteger;

  SomeDSP::LogScale<double> smoothness;
};

struct PlugParameter {
  Scales scale;
  std::vector<std::unique_ptr<ValueInterface>> value;

  PlugParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;
    // using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::inputGain]
      = std::make_unique<LogValue>(0.5, scale.inputGain, "inputGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      scale.outputGain.invmap(1.0), scale.outputGain, "outputGain", Info::kCanAutomate);
    value[ID::clip] = std::make_unique<LogValue>(
      scale.clip.invmap(1.0), scale.clip, "clip", Info::kCanAutomate);
    value[ID::ratio] = std::make_unique<LinearValue>(
      0.9, scale.defaultScale, "ratio", Info::kCanAutomate);
    value[ID::slope] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "slope", Info::kCanAutomate);

    value[ID::orderInteger] = std::make_unique<UIntValue>(
      2, scale.orderInteger, "orderInteger", Info::kCanAutomate);
    value[ID::orderFraction] = std::make_unique<LinearValue>(
      0, scale.defaultScale, "orderFraction", Info::kCanAutomate);

    value[ID::oversample] = std::make_unique<UIntValue>(
      true, scale.boolScale, "oversample", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);

    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
  }

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

  double getDefaultNormalized(int32_t tag)
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
};

} // namespace Synth
} // namespace Steinberg
