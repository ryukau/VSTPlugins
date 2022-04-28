// (c) 2020-2022 Takamitsu Endo
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
  limiterAttack, // Deprecated but not removed for backward compatibility.
  limiterRelease,

  guiInputGain,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = guiInputGain,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> drive;
  static SomeDSP::LinearScale<double> boost;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::UIntScale<double> order;

  static SomeDSP::LogScale<double> smoothness;

  static SomeDSP::LogScale<double> limiterThreshold;
  static SomeDSP::LogScale<double> limiterAttack; // Deprecated.
  static SomeDSP::LogScale<double> limiterRelease;

  static SomeDSP::LinearScale<double> guiInputGainScale;
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

    value[ID::drive]
      = std::make_unique<LogValue>(0.5, Scales::drive, "drive", Info::kCanAutomate);
    value[ID::boost]
      = std::make_unique<LinearValue>(0.0, Scales::boost, "boost", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, Scales::outputGain, "outputGain", Info::kCanAutomate);

    value[ID::order]
      = std::make_unique<UIntValue>(0, Scales::order, "order", Info::kCanAutomate);
    value[ID::flip]
      = std::make_unique<UIntValue>(true, Scales::boolScale, "flip", Info::kCanAutomate);
    value[ID::inverse] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "inverse", Info::kCanAutomate);

    value[ID::oversample] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "oversample", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

    value[ID::limiter]
      = std::make_unique<UIntValue>(1, Scales::boolScale, "limiter", Info::kCanAutomate);
    value[ID::limiterThreshold] = std::make_unique<LogValue>(
      Scales::limiterThreshold.invmap(1.0), Scales::limiterThreshold, "limiterThreshold",
      Info::kCanAutomate);
    value[ID::limiterAttack] = std::make_unique<LogValue>(
      Scales::limiterAttack.invmap(0.002), Scales::limiterAttack,
      "limiterAttack (deprecated)", Info::kIsHidden);
    value[ID::limiterRelease] = std::make_unique<LogValue>(
      Scales::limiterRelease.invmap(0.005), Scales::limiterRelease, "limiterRelease",
      Info::kCanAutomate);

    value[ID::guiInputGain] = std::make_unique<LinearValue>(
      0.0, Scales::guiInputGainScale, "guiInputGain", Info::kIsReadOnly);

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
