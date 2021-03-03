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
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , inputGain(0.0, 16.0, 0.5, 2.0)
    , outputGain(0.0, 1.0, 0.5, 0.1)
    , add(0.0, 1.0)
    , mul(0.0, 1.0)
    , moreAdd(1.0, 2.0)
    , moreMul(1.0, 2.0)
    , type(3)
    , lowpassCutoff(20.0, 20000.0, 0.5, 200.0)
    , smoothness(0.0, 0.5, 0.1, 0.04)
    , limiterThreshold(0.01, 2.0, 0.5, 0.5)
    , limiterAttack(0.0001, 0.021328, 0.1, 0.002)
    , limiterRelease(0.0001, 0.2, 0.2, 0.01)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> inputGain;
  SomeDSP::LinearScale<double> add;
  SomeDSP::LinearScale<double> mul;
  SomeDSP::LinearScale<double> moreAdd;
  SomeDSP::LinearScale<double> moreMul;
  SomeDSP::LogScale<double> outputGain;
  SomeDSP::UIntScale<double> type;

  SomeDSP::LogScale<double> lowpassCutoff;

  SomeDSP::LogScale<double> smoothness;

  SomeDSP::LogScale<double> limiterThreshold;
  SomeDSP::LogScale<double> limiterAttack;
  SomeDSP::LogScale<double> limiterRelease;
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
    value[ID::add]
      = std::make_unique<LinearValue>(1.0, scale.add, "add", Info::kCanAutomate);
    value[ID::mul]
      = std::make_unique<LinearValue>(1.0, scale.mul, "mul", Info::kCanAutomate);
    value[ID::moreAdd]
      = std::make_unique<LinearValue>(0.0, scale.moreAdd, "moreAdd", Info::kCanAutomate);
    value[ID::moreMul]
      = std::make_unique<LinearValue>(0.0, scale.moreMul, "moreMul", Info::kCanAutomate);
    value[ID::clipGain]
      = std::make_unique<LogValue>(1.0, scale.outputGain, "clipGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, scale.outputGain, "outputGain", Info::kCanAutomate);
    value[ID::type]
      = std::make_unique<UIntValue>(1, scale.type, "type", Info::kCanAutomate);
    value[ID::hardclip] = std::make_unique<UIntValue>(
      false, scale.boolScale, "hardclip", Info::kCanAutomate);

    value[ID::lowpass]
      = std::make_unique<UIntValue>(true, scale.boolScale, "lowpass", Info::kCanAutomate);
    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      1.0, scale.lowpassCutoff, "lowpassCutoff", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);

    value[ID::limiter]
      = std::make_unique<UIntValue>(1, scale.boolScale, "limiter", Info::kCanAutomate);
    value[ID::limiterThreshold] = std::make_unique<LogValue>(
      scale.limiterThreshold.invmap(1.0), scale.limiterThreshold, "limiterThreshold",
      Info::kCanAutomate);
    value[ID::limiterAttack] = std::make_unique<LogValue>(
      scale.limiterAttack.invmap(0.002), scale.limiterAttack, "limiterAttack",
      Info::kCanAutomate);
    value[ID::limiterRelease] = std::make_unique<LogValue>(
      scale.limiterRelease.invmap(0.005), scale.limiterRelease, "limiterRelease",
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

  double getDefaultNormalized(int32_t tag)
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
};

} // namespace Synth
} // namespace Steinberg
