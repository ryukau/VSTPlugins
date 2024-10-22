// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

  inputGain,
  outputGain,
  clip,
  ratio,
  slope,
  orderInteger,
  orderFraction,

  oversample,

  smoothness,

  guiInputGain,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = guiInputGain,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> inputGain;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::LogScale<double> clip;
  static SomeDSP::UIntScale<double> orderInteger;

  static SomeDSP::LogScale<double> smoothness;

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

    value[ID::orderInteger] = std::make_unique<UIntValue>(
      2, Scales::orderInteger, "orderInteger", Info::kCanAutomate);
    value[ID::orderFraction] = std::make_unique<LinearValue>(
      0, Scales::defaultScale, "orderFraction", Info::kCanAutomate);

    value[ID::oversample] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "oversample", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

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
