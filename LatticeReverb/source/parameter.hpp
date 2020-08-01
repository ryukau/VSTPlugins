// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

constexpr size_t nestingDepth = 16;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  time0 = 0,
  outerFeed0 = 16,
  innerFeed0 = 32,

  timeOffset0 = 48,
  outerFeedOffset0 = 64,
  innerFeedOffset0 = 80,

  timeLfoAmount0 = 96,
  lowpassCutoff0 = 112,

  timeMultiply = 128,
  outerFeedMultiply,
  innerFeedMultiply,

  timeOffsetMultiply,
  outerFeedOffsetMultiply,
  innerFeedOffsetMultiply,

  timeLfoLowpass,

  stereoCross,
  stereoSpread,

  dry,
  wet,

  smoothness,
  bypass,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> multiply;
  static SomeDSP::LogScale<double> time;
  static SomeDSP::LinearScale<double> feed;
  static SomeDSP::LinearScale<double> timeOffset;
  static SomeDSP::LinearScale<double> feedOffset;
  static SomeDSP::LogScale<double> timeLfoLowpas;
  static SomeDSP::LinearScale<double> stereoCross;
  static SomeDSP::LogScale<double> gain;
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

    value[ID::bypass] = std::make_unique<IntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string timeLabel("time");
    std::string outerFeedLabel("outerFeed");
    std::string innerFeedLabel("innerFeed");

    std::string timeOffsetLabel("timeOffset");
    std::string outerFeedOffsetLabel("outerFeedOffset");
    std::string innerFeedOffsetLabel("innerFeedOffset");

    std::string timeLfoAmountLabel("timeLfoAmount");
    std::string lowpassCutoffLabel("lowpassCutoff");

    for (size_t idx = 0; idx < nestingDepth; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::time0 + idx] = std::make_unique<LogValue>(
        Scales::time.invmap(0.1), Scales::time, (timeLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::outerFeed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (outerFeedLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::innerFeed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (innerFeedLabel + indexStr).c_str(), Info::kCanAutomate);

      value[ID::timeOffset0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::timeOffset, (timeOffsetLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::outerFeedOffset0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feedOffset, (outerFeedOffsetLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::innerFeedOffset0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feedOffset, (innerFeedOffsetLabel + indexStr).c_str(),
        Info::kCanAutomate);

      value[ID::timeLfoAmount0 + idx] = std::make_unique<LogValue>(
        0.0, Scales::time, (timeLfoAmountLabel + indexStr).c_str(), Info::kCanAutomate);

      value[ID::lowpassCutoff0 + idx] = std::make_unique<LinearValue>(
        1.0, Scales::defaultScale, (lowpassCutoffLabel + indexStr).c_str(),
        Info::kCanAutomate);
    }

    value[ID::timeMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::multiply, "timeMultiply", Info::kCanAutomate);
    value[ID::outerFeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::multiply, "outerFeedMultiply", Info::kCanAutomate);
    value[ID::innerFeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::multiply, "innerFeedMultiply", Info::kCanAutomate);

    value[ID::timeOffsetMultiply] = std::make_unique<LinearValue>(
      0.05, Scales::multiply, "timeOffsetMultiply", Info::kCanAutomate);
    value[ID::outerFeedOffsetMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::multiply, "outerFeedOffsetMultiply", Info::kCanAutomate);
    value[ID::innerFeedOffsetMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::multiply, "innerFeedOffsetMultiply", Info::kCanAutomate);

    value[ID::timeLfoLowpass] = std::make_unique<LogValue>(
      Scales::timeLfoLowpas.invmap(0.01), Scales::timeLfoLowpas, "timeLfoLowpass",
      Info::kCanAutomate);

    value[ID::stereoCross] = std::make_unique<LinearValue>(
      0.0, Scales::stereoCross, "stereoCross", Info::kCanAutomate);
    value[ID::stereoSpread] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "stereoSpread", Info::kCanAutomate);

    value[ID::dry]
      = std::make_unique<LogValue>(0.5, Scales::gain, "dry", Info::kCanAutomate);
    value[ID::wet]
      = std::make_unique<LogValue>(0.5, Scales::gain, "wet", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.5, Scales::smoothness, "smoothness", Info::kCanAutomate);

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
