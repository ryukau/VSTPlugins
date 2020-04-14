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

#include "../../common/parameterInterface.hpp"
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
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> inputGain;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::LogScale<double> clip;
  static SomeDSP::IntScale<double> orderInteger;

  static SomeDSP::LogScale<double> smoothness;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;
    // using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::inputGain] = std::make_unique<LogValue>(
      0.5, Scales::inputGain, "inputGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      Scales::outputGain.invmap(1.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::clip] = std::make_unique<LogValue>(
      Scales::clip.invmap(1.0), Scales::clip, "clip", Info::kCanAutomate);
    value[ID::ratio] = std::make_unique<LinearValue>(
      0.9, Scales::defaultScale, "ratio", Info::kCanAutomate);
    value[ID::slope] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "slope", Info::kCanAutomate);

    value[ID::orderInteger] = std::make_unique<IntValue>(
      2, Scales::orderInteger, "orderInteger", Info::kCanAutomate);
    value[ID::orderFraction] = std::make_unique<LinearValue>(
      0, Scales::defaultScale, "orderFraction", Info::kCanAutomate);

    value[ID::oversample] = std::make_unique<IntValue>(
      true, Scales::boolScale, "oversample", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

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
