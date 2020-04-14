// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

constexpr size_t nOvertone = 64;

namespace ParameterID {
enum ID {
  bypass,

  mix,
  frequency,
  freqSpread,
  feedback,
  range,
  min,
  phase,
  stereoOffset,
  cascadeOffset,
  stage,

  smoothness,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> frequency;
  static SomeDSP::LinearScale<double> feedback;
  static SomeDSP::LogScale<double> range;
  static SomeDSP::LinearScale<double> phase;
  static SomeDSP::LinearScale<double> cascadeOffset;
  static SomeDSP::IntScale<double> stage;

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
    using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::mix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "mix", Info::kCanAutomate);
    value[ID::frequency] = std::make_unique<LogValue>(
      0.5, Scales::frequency, "frequency", Info::kCanAutomate);
    value[ID::freqSpread] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "freqSpread", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.5, Scales::feedback, "feedback", Info::kCanAutomate);
    value[ID::range]
      = std::make_unique<LogValue>(1.0, Scales::range, "range", Info::kCanAutomate);
    value[ID::min]
      = std::make_unique<LogValue>(0.0, Scales::range, "min", Info::kCanAutomate);
    value[ID::phase]
      = std::make_unique<LinearValue>(0.0, Scales::phase, "phase", Info::kCanAutomate);
    value[ID::stereoOffset] = std::make_unique<LinearValue>(
      0.5, Scales::phase, "stereoOffset", Info::kCanAutomate);
    value[ID::cascadeOffset] = std::make_unique<LinearValue>(
      Scales::cascadeOffset.invmap(SomeDSP::twopi / 16.0), Scales::cascadeOffset,
      "cascadeOffset", Info::kCanAutomate);
    value[ID::stage]
      = std::make_unique<IntValue>(15, Scales::stage, "stage", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      Scales::smoothness.invmap(0.35), Scales::smoothness, "smoothness",
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
