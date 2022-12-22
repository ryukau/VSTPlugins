// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t nLfoWavetable = 64;
constexpr double maxDelayTime = 4.0;

namespace ParameterID {
enum ID {
  bypass,

  dryGain,
  wetGain,

  feedback,
  delayTimeSeconds,
  shiftPitch,
  shiftHz,
  highpassHz,
  lowpassHz,

  lfoToPrimaryDelayTime,
  lfoToPrimaryShiftPitch,
  lfoToPrimaryShiftHz,

  lfoTempoSync,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoPhaseOffset,
  lfoPhaseConstant,
  lfoShapeClip,
  lfoShapeSkew,

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
  static SomeDSP::DecibelScale<double> delayTimeSeconds;
  static SomeDSP::LinearScale<double> shiftPitch;
  static SomeDSP::DecibelScale<double> shiftHz;
  static SomeDSP::DecibelScale<double> cutoffHz;
  static SomeDSP::LinearScale<double> lfoToPrimary;

  static SomeDSP::UIntScale<double> lfoTempoUpper;
  static SomeDSP::UIntScale<double> lfoTempoLower;
  static SomeDSP::DecibelScale<double> lfoRate;
  static SomeDSP::DecibelScale<double> lfoShapeClip;
  static SomeDSP::DecibelScale<double> lfoShapeSkew;

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

    value[ID::dryGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "dryGain", Info::kCanAutomate);
    value[ID::wetGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "wetGain", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.75, Scales::defaultScale, "feedback", Info::kCanAutomate);
    value[ID::delayTimeSeconds] = std::make_unique<DecibelValue>(
      Scales::delayTimeSeconds.invmap(0.1), Scales::delayTimeSeconds, "delayTimeSeconds",
      Info::kCanAutomate);
    value[ID::shiftPitch] = std::make_unique<LinearValue>(
      Scales::shiftPitch.invmap(0.0), Scales::shiftPitch, "shiftPitch",
      Info::kCanAutomate);
    value[ID::shiftHz] = std::make_unique<DecibelValue>(
      Scales::shiftHz.invmap(0.0), Scales::shiftHz, "shiftHz", Info::kCanAutomate);
    value[ID::highpassHz] = std::make_unique<DecibelValue>(
      0.0, Scales::cutoffHz, "highpassHz", Info::kCanAutomate);
    value[ID::lowpassHz] = std::make_unique<DecibelValue>(
      1.0, Scales::cutoffHz, "lowpassHz", Info::kCanAutomate);

    value[ID::lfoToPrimaryDelayTime] = std::make_unique<LinearValue>(
      0.5, Scales::lfoToPrimary, "lfoToPrimaryDelayTime", Info::kCanAutomate);
    value[ID::lfoToPrimaryShiftPitch] = std::make_unique<LinearValue>(
      0.5, Scales::lfoToPrimary, "lfoToPrimaryShiftPitch", Info::kCanAutomate);
    value[ID::lfoToPrimaryShiftHz] = std::make_unique<LinearValue>(
      0.5, Scales::lfoToPrimary, "lfoToPrimaryShiftHz", Info::kCanAutomate);

    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoTempoUpper] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoUpper, "lfoTempoUpper", Info::kCanAutomate);
    value[ID::lfoTempoLower] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoLower, "lfoTempoLower", Info::kCanAutomate);
    value[ID::lfoRate] = std::make_unique<DecibelValue>(
      Scales::lfoRate.invmap(1.0), Scales::lfoRate, "lfoRate", Info::kCanAutomate);
    value[ID::lfoPhaseOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseOffset", Info::kCanAutomate);
    value[ID::lfoPhaseConstant] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseConstant", Info::kCanAutomate);
    value[ID::lfoShapeClip] = std::make_unique<DecibelValue>(
      Scales::lfoShapeClip.invmap(1.0), Scales::lfoShapeClip, "lfoShapeClip",
      Info::kCanAutomate);
    value[ID::lfoShapeSkew] = std::make_unique<DecibelValue>(
      Scales::lfoShapeSkew.invmap(1.0), Scales::lfoShapeSkew, "lfoShapeSkew",
      Info::kCanAutomate);

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
