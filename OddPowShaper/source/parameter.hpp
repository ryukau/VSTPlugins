// (c) 2020 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

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

  drive,
  boost,
  outputGain,

  order,
  flip,
  inverse,

  oversample,

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
    , drive(1.0, 32.0, 0.5, 4.0)
    , boost(1.0, 32.0)
    , outputGain(0.0, 1.0, 0.5, 0.1)
    , order(15)
    , smoothness(0.0, 0.5, 0.1, 0.04)
    , limiterThreshold(0.01, 2.0, 0.5, 0.5)
    , limiterAttack(0.0001, 0.021328, 0.1, 0.002)
    , limiterRelease(0.0001, 0.2, 0.2, 0.01)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> drive;
  SomeDSP::LinearScale<double> boost;
  SomeDSP::LogScale<double> outputGain;
  SomeDSP::UIntScale<double> order;

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

    value[ID::drive]
      = std::make_unique<LogValue>(0.5, scale.drive, "drive", Info::kCanAutomate);
    value[ID::boost]
      = std::make_unique<LinearValue>(0.0, scale.boost, "boost", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, scale.outputGain, "outputGain", Info::kCanAutomate);

    value[ID::order]
      = std::make_unique<UIntValue>(0, scale.order, "order", Info::kCanAutomate);
    value[ID::flip]
      = std::make_unique<UIntValue>(true, scale.boolScale, "flip", Info::kCanAutomate);
    value[ID::inverse]
      = std::make_unique<UIntValue>(true, scale.boolScale, "inverse", Info::kCanAutomate);

    value[ID::oversample] = std::make_unique<UIntValue>(
      true, scale.boolScale, "oversample", Info::kCanAutomate);

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
