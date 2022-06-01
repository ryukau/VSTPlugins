// (c) 2021-2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t nCombTaps = 4;
constexpr float maxDelayTime = 4.0f;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  delayTime0,

  timeMultiplier = delayTime0 + nCombTaps,
  feedback,
  feedbackHighpassCutoffHz,
  feedbackLimiterRelease,
  delayTimeInterpRate,
  delayTimeInterpLowpassSeconds,
  stereoLean,
  stereoCross,
  feedbackToDelayTime,

  dry,
  wet,
  channelType,
  overSampling,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> delayTime;
  static SomeDSP::LogScale<double> feedback;
  static SomeDSP::SemitoneScale<double> feedbackHighpassCutoffHz;
  static SomeDSP::LogScale<double> feedbackLimiterRelease;
  static SomeDSP::LogScale<double> delayTimeInterpRate;
  static SomeDSP::DecibelScale<double> delayTimeInterpLowpassSeconds;
  static SomeDSP::LinearScale<double> stereoLean;
  static SomeDSP::DecibelScale<double> feedbackToDelayTime;

  static SomeDSP::DecibelScale<double> dry;
  static SomeDSP::DecibelScale<double> wet;
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
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using SemitoneValue = DoubleValue<SomeDSP::SemitoneScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string delayTimeLabel("delayTime");

    for (size_t idx = 0; idx < nCombTaps; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::delayTime0 + idx] = std::make_unique<LogValue>(
        Scales::delayTime.invmap(0.1), Scales::delayTime,
        (delayTimeLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::timeMultiplier] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "timeMultiplier", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LogValue>(
      Scales::feedback.invmap(1.0 / double(nCombTaps)), Scales::feedback, "feedback",
      Info::kCanAutomate);
    value[ID::feedbackHighpassCutoffHz] = std::make_unique<SemitoneValue>(
      Scales::feedbackHighpassCutoffHz.invmap(5.0), Scales::feedbackHighpassCutoffHz,
      "feedbackHighpassCutoffHz", Info::kCanAutomate);
    value[ID::feedbackLimiterRelease] = std::make_unique<LogValue>(
      Scales::feedbackLimiterRelease.invmap(0.2), Scales::feedbackLimiterRelease,
      "feedbackLimiterRelease", Info::kCanAutomate);
    value[ID::delayTimeInterpRate] = std::make_unique<LogValue>(
      Scales::delayTimeInterpRate.invmap(0.5), Scales::delayTimeInterpRate,
      "delayTimeInterpRate", Info::kCanAutomate);
    value[ID::delayTimeInterpLowpassSeconds] = std::make_unique<DecibelValue>(
      0.0, Scales::delayTimeInterpLowpassSeconds, "delayTimeInterpLowpassSeconds",
      Info::kCanAutomate);
    value[ID::stereoLean] = std::make_unique<LinearValue>(
      0.5, Scales::stereoLean, "stereoLean", Info::kCanAutomate);
    value[ID::stereoCross] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "stereoCross", Info::kCanAutomate);
    value[ID::feedbackToDelayTime] = std::make_unique<DecibelValue>(
      0.0, Scales::feedbackToDelayTime, "feedbackToDelayTime", Info::kCanAutomate);

    value[ID::dry] = std::make_unique<DecibelValue>(
      Scales::dry.invmapDB(0.0), Scales::dry, "dry", Info::kCanAutomate);
    value[ID::wet] = std::make_unique<DecibelValue>(
      Scales::wet.invmapDB(0.0), Scales::wet, "wet", Info::kCanAutomate);
    value[ID::channelType] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "channelType", Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "overSampling", Info::kCanAutomate);

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
