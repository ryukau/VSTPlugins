// (c) 2021-2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr float maxDelayTime = 1.0f;
constexpr size_t nLfoWavetable = 64;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  lfoWavetable0,
  lfoInterpolation = lfoWavetable0 + nLfoWavetable,
  lfoTempoSync,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoStereoOffset,
  lfoUnisonOffset,

  lfoToPitch,
  lfoToUnison,

  pitch,
  unisonPitchOffset,
  pitchCross,
  shifterMainReverse,
  shifterUnisonReverse,
  mirrorUnisonPitch,

  delayTime,
  stereoLean,
  stereoCross,
  feedback,
  highpassCutoffHz,

  dry,
  wet,
  unisonMix,
  channelType,
  smoothingTime,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LinearScale<double> lfoWavetable;
  static SomeDSP::UIntScale<double> lfoInterpolation;
  static SomeDSP::UIntScale<double> lfoTempoUpper;
  static SomeDSP::UIntScale<double> lfoTempoLower;
  static SomeDSP::DecibelScale<double> lfoRate;

  static SomeDSP::DecibelScale<double> pitch;
  static SomeDSP::LogScale<double> delayTime;
  static SomeDSP::LinearScale<double> stereoLean;
  static SomeDSP::LogScale<double> feedback;
  static SomeDSP::SemitoneScale<double> highpassCutoffHz;

  static SomeDSP::DecibelScale<double> dry;
  static SomeDSP::DecibelScale<double> wet;
  static SomeDSP::DecibelScale<double> smoothingTime;
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

    std::string lfoWavetableLabel("lfoWavetable");
    for (size_t idx = 0; idx < nLfoWavetable; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::lfoWavetable0 + idx] = std::make_unique<LinearValue>(
        Scales::lfoWavetable.invmap(sin(SomeDSP::twopi * idx / double(nLfoWavetable))),
        Scales::lfoWavetable, (lfoWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
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
      Scales::lfoRate.invmap(1.0), Scales::lfoRate, "lfoRate", Info::kCanAutomate);
    value[ID::lfoStereoOffset] = std::make_unique<LinearValue>(
      0.5, Scales::stereoLean, "lfoStereoOffset", Info::kCanAutomate);
    value[ID::lfoUnisonOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoUnisonOffset",
      Info::kCanAutomate | Info::kIsWrapAround);

    value[ID::lfoToPitch] = std::make_unique<DecibelValue>(
      0.0, Scales::pitch, "lfoToPitch", Info::kCanAutomate);
    value[ID::lfoToUnison] = std::make_unique<DecibelValue>(
      0.0, Scales::pitch, "lfoToUnison", Info::kCanAutomate);

    value[ID::pitch] = std::make_unique<DecibelValue>(
      Scales::pitch.invmapDB(0), Scales::pitch, "pitch", Info::kCanAutomate);
    value[ID::unisonPitchOffset] = std::make_unique<DecibelValue>(
      0.0, Scales::pitch, "unisonPitchOffset", Info::kCanAutomate);
    value[ID::pitchCross] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "pitchCross", Info::kCanAutomate);
    value[ID::shifterMainReverse] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "shifterMainReverse", Info::kCanAutomate);
    value[ID::shifterUnisonReverse] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "shifterUnisonReverse", Info::kCanAutomate);
    value[ID::mirrorUnisonPitch] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "mirrorUnisonPitch", Info::kCanAutomate);

    value[ID::delayTime] = std::make_unique<LogValue>(
      Scales::delayTime.invmap(0.1), Scales::delayTime, "delayTime", Info::kCanAutomate);
    value[ID::stereoLean] = std::make_unique<LinearValue>(
      0.5, Scales::stereoLean, "stereoLean", Info::kCanAutomate);
    value[ID::stereoCross] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "stereoCross", Info::kCanAutomate);
    value[ID::feedback]
      = std::make_unique<LogValue>(0.0, Scales::feedback, "feedback", Info::kCanAutomate);
    value[ID::highpassCutoffHz] = std::make_unique<SemitoneValue>(
      Scales::highpassCutoffHz.invmap(5.0), Scales::highpassCutoffHz, "highpassCutoffHz",
      Info::kCanAutomate);

    value[ID::dry]
      = std::make_unique<DecibelValue>(0.0, Scales::dry, "dry", Info::kCanAutomate);
    value[ID::wet] = std::make_unique<DecibelValue>(
      Scales::wet.invmapDB(0.0), Scales::wet, "wet", Info::kCanAutomate);
    value[ID::unisonMix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "unisonMix", Info::kCanAutomate);
    value[ID::channelType] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "channelType", Info::kCanAutomate);
    value[ID::smoothingTime] = std::make_unique<DecibelValue>(
      Scales::smoothingTime.invmap(0.2), Scales::smoothingTime, "smoothingTime",
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
