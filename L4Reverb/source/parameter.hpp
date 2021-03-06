// (c) 2020 Takamitsu Endo
//
// This file is part of L4Reverb.
//
// L4Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// L4Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with L4Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

constexpr uint16_t nDepth1 = 256;
constexpr uint16_t nDepth2 = 64;
constexpr uint16_t nDepth3 = 16;
constexpr uint16_t nDepth4 = 4;
constexpr uint16_t nDepth = 4;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  time0 = 0,
  innerFeed0 = nDepth1,
  d1Feed0 = 2 * nDepth1,
  d2Feed0 = 3 * nDepth1,
  d3Feed0 = 3 * nDepth1 + nDepth2,
  d4Feed0 = 3 * nDepth1 + nDepth2 + nDepth3,

  timeMultiply = 3 * nDepth1 + nDepth2 + nDepth3 + nDepth4,
  innerFeedMultiply,
  d1FeedMultiply,
  d2FeedMultiply,
  d3FeedMultiply,
  d4FeedMultiply,

  timeOffsetRange,
  innerFeedOffsetRange,
  d1FeedOffsetRange,
  d2FeedOffsetRange,
  d3FeedOffsetRange,
  d4FeedOffsetRange,

  timeModulation,
  innerFeedModulation,
  d1FeedModulation,
  d2FeedModulation,
  d3FeedModulation,
  d4FeedModulation,

  seed,

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
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LogScale<double> time;
  static SomeDSP::LinearScale<double> feed;
  static SomeDSP::LogScale<double> timeMultiply;
  static SomeDSP::LogScale<double> timeOffsetRange;
  static SomeDSP::UIntScale<double> seed;
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
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using LogValue = DoubleValue<SomeDSP::LogScale<double>>;

    std::string timeLabel("time");
    std::string innerFeedLabel("innerFeed");
    std::string d1FeedLabel("d1Feed");
    for (size_t idx = 0; idx < nDepth1; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::time0 + idx] = std::make_unique<LogValue>(
        Scales::time.invmap(0.1), Scales::time, (timeLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::innerFeed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (innerFeedLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::d1Feed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (d1FeedLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string d2FeedLabel("d2Feed");
    for (size_t idx = 0; idx < nDepth2; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::d2Feed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (d2FeedLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string d3FeedLabel("d3Feed");
    for (size_t idx = 0; idx < nDepth3; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::d3Feed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (d3FeedLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string d4FeedLabel("d4Feed");
    for (size_t idx = 0; idx < nDepth4; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::d4Feed0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::feed, (d4FeedLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::timeMultiply] = std::make_unique<LogValue>(
      1.0, Scales::timeMultiply, "timeMultiply", Info::kCanAutomate);
    value[ID::innerFeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "innerFeedMultiply", Info::kCanAutomate);
    value[ID::d1FeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "d1FeedMultiply", Info::kCanAutomate);
    value[ID::d2FeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "d2FeedMultiply", Info::kCanAutomate);
    value[ID::d3FeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "d3FeedMultiply", Info::kCanAutomate);
    value[ID::d4FeedMultiply] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "d4FeedMultiply", Info::kCanAutomate);

    value[ID::timeOffsetRange] = std::make_unique<LogValue>(
      Scales::timeOffsetRange.invmap(0.05), Scales::timeOffsetRange, "timeOffsetRange",
      Info::kCanAutomate);
    value[ID::innerFeedOffsetRange] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "innerFeedOffsetRange", Info::kCanAutomate);
    value[ID::d1FeedOffsetRange] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "d1FeedOffsetRange", Info::kCanAutomate);
    value[ID::d2FeedOffsetRange] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "d2FeedOffsetRange", Info::kCanAutomate);
    value[ID::d3FeedOffsetRange] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "d3FeedOffsetRange", Info::kCanAutomate);
    value[ID::d4FeedOffsetRange] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "d4FeedOffsetRange", Info::kCanAutomate);

    value[ID::timeModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "timeModulation", Info::kCanAutomate);
    value[ID::innerFeedModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "innerFeedModulation", Info::kCanAutomate);
    value[ID::d1FeedModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "d1FeedModulation", Info::kCanAutomate);
    value[ID::d2FeedModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "d2FeedModulation", Info::kCanAutomate);
    value[ID::d3FeedModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "d3FeedModulation", Info::kCanAutomate);
    value[ID::d4FeedModulation] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "d4FeedModulation", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);

    value[ID::stereoCross] = std::make_unique<LinearValue>(
      Scales::stereoCross.invmap(0.0), Scales::stereoCross, "stereoCross",
      Info::kCanAutomate);
    value[ID::stereoSpread] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "stereoSpread", Info::kCanAutomate);

    value[ID::dry]
      = std::make_unique<LogValue>(0.5, Scales::gain, "dry", Info::kCanAutomate);
    value[ID::wet]
      = std::make_unique<LogValue>(0.5, Scales::gain, "wet", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.5, Scales::smoothness, "smoothness", Info::kCanAutomate);
    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

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
