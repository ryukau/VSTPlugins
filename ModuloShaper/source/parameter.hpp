// (c) 2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

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
  clipGain,
  outputGain,
  add,
  mul,
  moreAdd,
  moreMul,
  type,
  hardclip,

  lowpass,
  lowpassCutoff,

  smoothness,

  limiter,
  limiterThreshold,
  limiterAttack,
  limiterRelease,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> inputGain;
  static SomeDSP::LinearScale<double> add;
  static SomeDSP::LinearScale<double> mul;
  static SomeDSP::LinearScale<double> moreAdd;
  static SomeDSP::LinearScale<double> moreMul;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::UIntScale<double> type;

  static SomeDSP::LogScale<double> lowpassCutoff;

  static SomeDSP::LogScale<double> smoothness;

  static SomeDSP::LogScale<double> limiterThreshold;
  static SomeDSP::LogScale<double> limiterAttack;
  static SomeDSP::LogScale<double> limiterRelease;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using LogValue = DoubleValue<SomeDSP::LogScale<double>>;
    // using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::inputGain] = std::make_unique<LogValue>(
      0.5, Scales::inputGain, "inputGain", Info::kCanAutomate);
    value[ID::add]
      = std::make_unique<LinearValue>(1.0, Scales::add, "add", Info::kCanAutomate);
    value[ID::mul]
      = std::make_unique<LinearValue>(1.0, Scales::mul, "mul", Info::kCanAutomate);
    value[ID::moreAdd] = std::make_unique<LinearValue>(
      0.0, Scales::moreAdd, "moreAdd", Info::kCanAutomate);
    value[ID::moreMul] = std::make_unique<LinearValue>(
      0.0, Scales::moreMul, "moreMul", Info::kCanAutomate);
    value[ID::clipGain] = std::make_unique<LogValue>(
      1.0, Scales::outputGain, "clipGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, Scales::outputGain, "outputGain", Info::kCanAutomate);
    value[ID::type]
      = std::make_unique<UIntValue>(1, Scales::type, "type", Info::kCanAutomate);
    value[ID::hardclip] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "hardclip", Info::kCanAutomate);

    value[ID::lowpass] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "lowpass", Info::kCanAutomate);
    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      1.0, Scales::lowpassCutoff, "lowpassCutoff", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

    value[ID::limiter]
      = std::make_unique<UIntValue>(1, Scales::boolScale, "limiter", Info::kCanAutomate);
    value[ID::limiterThreshold] = std::make_unique<LogValue>(
      Scales::limiterThreshold.invmap(1.0), Scales::limiterThreshold, "limiterThreshold",
      Info::kCanAutomate);
    value[ID::limiterAttack] = std::make_unique<LogValue>(
      Scales::limiterAttack.invmap(0.002), Scales::limiterAttack, "limiterAttack",
      Info::kCanAutomate);
    value[ID::limiterRelease] = std::make_unique<LogValue>(
      Scales::limiterRelease.invmap(0.005), Scales::limiterRelease, "limiterRelease",
      Info::kCanAutomate);

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

  double getDefaultNormalized(int32_t tag) override
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
};

} // namespace Synth
} // namespace Steinberg
