// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <memory>
#include <numbers>
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
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> frequency;
  static SomeDSP::LinearScale<double> feedback;
  static SomeDSP::LogScale<double> range;
  static SomeDSP::LinearScale<double> phase;
  static SomeDSP::LinearScale<double> cascadeOffset;
  static SomeDSP::UIntScale<double> stage;

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
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
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
      Scales::cascadeOffset.invmap(double(2) * std::numbers::pi_v<double> / 16.0),
      Scales::cascadeOffset, "cascadeOffset", Info::kCanAutomate);
    value[ID::stage]
      = std::make_unique<UIntValue>(15, Scales::stage, "stage", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      Scales::smoothness.invmap(0.35), Scales::smoothness, "smoothness",
      Info::kCanAutomate);

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
