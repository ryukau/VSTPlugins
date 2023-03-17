// (c) 2023 Takamitsu Endo
//
// This file is part of RingModSpacer.
//
// RingModSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RingModSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RingModSpacer.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr double maxLimiterAttackSeconds = double(0.1);

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  sideMix,
  ringSubtractMix,

  inputGain,
  inputLimiterAttackSeconds,
  inputLimiterReleaseSeconds,

  sideGain,
  sideLimiterAttackSeconds,
  sideLimiterReleaseSeconds,

  parameterSmoothingSecond,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> limiterAttackSeconds;
  static SomeDSP::DecibelScale<double> limiterReleaseSeconds;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using NegativeDecibelValue = DoubleValue<SomeDSP::NegativeDecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::sideMix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "sideMix", Info::kCanAutomate);
    value[ID::ringSubtractMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "ringSubtractMix", Info::kCanAutomate);

    value[ID::inputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "inputGain", Info::kCanAutomate);
    value[ID::inputLimiterAttackSeconds] = std::make_unique<DecibelValue>(
      Scales::limiterAttackSeconds.invmap(64.0 / 48000.0), Scales::limiterAttackSeconds,
      "inputLimiterAttackSeconds", Info::kCanAutomate);
    value[ID::inputLimiterReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::limiterReleaseSeconds.invmap(0.001), Scales::limiterReleaseSeconds,
      "inputLimiterReleaseSeconds", Info::kCanAutomate);

    value[ID::sideGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "sideGain", Info::kCanAutomate);
    value[ID::sideLimiterAttackSeconds] = std::make_unique<DecibelValue>(
      Scales::limiterAttackSeconds.invmap(64.0 / 48000.0), Scales::limiterAttackSeconds,
      "sideLimiterAttackSeconds", Info::kCanAutomate);
    value[ID::sideLimiterReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::limiterReleaseSeconds.invmap(0.001), Scales::limiterReleaseSeconds,
      "sideLimiterReleaseSeconds", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.2), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);

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
