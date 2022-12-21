// (c) 2022 Takamitsu Endo
//
// This file is part of LongPhaser.
//
// LongPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LongPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LongPhaser.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t maxAllpass = 64;
constexpr size_t nLfoWavetable = 64;
constexpr double maxDelayTime = 1.0;

namespace ParameterID {
enum ID {
  bypass,

  stage,
  outputGain,
  mix,
  outerFeed,
  delayTimeSpread,
  delayTimeCenterSeconds,
  delayTimeRateLimit,
  delayTimeModOctave,
  delayTimeModType,
  innerFeed,
  lfoToInnerFeed,

  lfoWavetable0,
  lfoInterpolation = lfoWavetable0 + nLfoWavetable,
  lfoTempoSync,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoPhaseOffset,
  lfoPhaseConstant,

  parameterSmoothingSecond,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::UIntScale<double> stage;

  static SomeDSP::DecibelScale<double> outputGain;
  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> delayTimeSeconds;
  static SomeDSP::DecibelScale<double> delayTimeRateLimit;
  static SomeDSP::LinearScale<double> delayTimeModOctave;
  static SomeDSP::UIntScale<double> delayTimeModType;
  static SomeDSP::LinearScale<double> lfoToInnerFeed;

  static SomeDSP::UIntScale<double> lfoInterpolation;
  static SomeDSP::UIntScale<double> lfoTempoUpper;
  static SomeDSP::UIntScale<double> lfoTempoLower;
  static SomeDSP::DecibelScale<double> lfoRate;

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

    value[ID::stage]
      = std::make_unique<UIntValue>(15, Scales::stage, "stage", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::outputGain.invmapDB(0.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::mix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "mix", Info::kCanAutomate);
    value[ID::outerFeed] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "outerFeed", Info::kCanAutomate);
    value[ID::delayTimeSpread] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "delayTimeSpread", Info::kCanAutomate);
    value[ID::delayTimeCenterSeconds] = std::make_unique<DecibelValue>(
      Scales::delayTimeSeconds.invmap(0.0015), Scales::delayTimeSeconds,
      "delayTimeCenterSeconds", Info::kCanAutomate);
    value[ID::delayTimeRateLimit] = std::make_unique<DecibelValue>(
      Scales::delayTimeRateLimit.invmap(0.5), Scales::delayTimeRateLimit,
      "delayTimeRateLimit", Info::kCanAutomate);
    value[ID::delayTimeModOctave] = std::make_unique<LinearValue>(
      0.0, Scales::delayTimeModOctave, "delayTimeModOctave", Info::kCanAutomate);
    value[ID::delayTimeModType] = std::make_unique<UIntValue>(
      0, Scales::delayTimeModType, "delayTimeModType", Info::kCanAutomate);
    value[ID::innerFeed] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "innerFeed", Info::kCanAutomate);
    value[ID::lfoToInnerFeed] = std::make_unique<LinearValue>(
      0.0, Scales::lfoToInnerFeed, "lfoToInnerFeed", Info::kCanAutomate);

    std::string lfoWavetableLabel("lfoWavetable");
    for (size_t idx = 0; idx < nLfoWavetable; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::lfoWavetable0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(sin(SomeDSP::twopi * idx / double(nLfoWavetable))),
        Scales::bipolarScale, (lfoWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
    }
    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoInterpolation] = std::make_unique<UIntValue>(
      2, Scales::lfoInterpolation, "lfoInterpolation", Info::kCanAutomate);
    value[ID::lfoTempoUpper] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoUpper, "lfoTempoUpper", Info::kCanAutomate);
    value[ID::lfoTempoLower] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoLower, "lfoTempoLower", Info::kCanAutomate);
    value[ID::lfoRate] = std::make_unique<DecibelValue>(
      Scales::lfoRate.invmap(1.0), Scales::gain, "lfoRate", Info::kCanAutomate);
    value[ID::lfoPhaseOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseOffset", Info::kCanAutomate);
    value[ID::lfoPhaseConstant] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseConstant", Info::kCanAutomate);

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
