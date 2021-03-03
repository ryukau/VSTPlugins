// (c) 2019-2020 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <vector>

#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  seed,
  retriggerTime,
  retriggerStick,
  retriggerTremolo,
  fdn,
  fdnTime,
  fdnFeedback,
  fdnCascadeMix,
  allpassMix,
  allpass1Saturation,
  allpass1Time,
  allpass1Feedback,
  allpass1HighpassCutoff,
  allpass2Time,
  allpass2Feedback,
  allpass2HighpassCutoff,
  tremoloMix,
  tremoloDepth,
  tremoloFrequency,
  tremoloDelayTime,
  randomTremoloDepth,
  randomTremoloFrequency,
  randomTremoloDelayTime,
  stick,
  stickDecay,
  stickToneMix,
  stickPulseMix,
  stickVelvetMix,

  smoothness,

  gain,
  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , seed(16777215)
    , fdnTime(0.0001, 0.5, 0.5, 0.1)
    , fdnFeedback(0.0, 4.0, 0.75, 1.0)
    , fdnCascadeMix(0.0, 1.0, 0.5, 0.2)
    , allpassTime(0.0, 0.005, 0.5, 0.001)
    , allpassFeedback(0.0, 0.9999, 0.5, 0.9)
    , allpassHighpassCutoff(1.0, 40.0, 0.5, 10.0)
    , tremoloFrequency(0.1, 20, 0.5, 4.0)
    , tremoloDelayTime(0.00003, 0.001, 0.5, 0.0001)
    , stickDecay(0.01, 4.0, 0.5, 0.1)
    , stickToneMix(0.0, 0.02, 0.5, 0.001)
    , smoothness(0.0, 0.5, 0.2, 0.02)
    , gain(0.0, 4.0, 0.75, 0.5)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::UIntScale<double> seed;
  SomeDSP::LogScale<double> fdnTime;
  SomeDSP::LogScale<double> fdnFeedback;
  SomeDSP::LogScale<double> fdnCascadeMix;
  SomeDSP::LogScale<double> allpassTime;
  SomeDSP::LogScale<double> allpassFeedback;
  SomeDSP::LogScale<double> allpassHighpassCutoff;
  SomeDSP::LogScale<double> tremoloFrequency;
  SomeDSP::LogScale<double> tremoloDelayTime;
  SomeDSP::LogScale<double> stickDecay;
  SomeDSP::LogScale<double> stickToneMix;

  SomeDSP::LogScale<double> smoothness;

  SomeDSP::LogScale<double> gain;
};

struct PlugParameter {
  Scales scale;
  std::vector<std::unique_ptr<ValueInterface>> value;

  PlugParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::seed]
      = std::make_unique<UIntValue>(6583421, scale.seed, "seed", Info::kCanAutomate);
    value[ID::retriggerTime] = std::make_unique<UIntValue>(
      true, scale.boolScale, "retriggerTime", Info::kCanAutomate);
    value[ID::retriggerStick] = std::make_unique<UIntValue>(
      false, scale.boolScale, "retriggerStick", Info::kCanAutomate);
    value[ID::retriggerTremolo] = std::make_unique<UIntValue>(
      false, scale.boolScale, "retriggerTremolo", Info::kCanAutomate);

    value[ID::fdn]
      = std::make_unique<UIntValue>(true, scale.boolScale, "fdn", Info::kCanAutomate);
    value[ID::fdnTime]
      = std::make_unique<LogValue>(0.2, scale.fdnTime, "fdnTime", Info::kCanAutomate);
    value[ID::fdnFeedback] = std::make_unique<LogValue>(
      0.5, scale.fdnFeedback, "fdnFeedback", Info::kCanAutomate);
    value[ID::fdnCascadeMix] = std::make_unique<LogValue>(
      0.5, scale.fdnCascadeMix, "fdnCascadeMix", Info::kCanAutomate);

    value[ID::allpassMix] = std::make_unique<LinearValue>(
      0.75, scale.defaultScale, "allpassMix", Info::kCanAutomate);
    value[ID::allpass1Saturation] = std::make_unique<UIntValue>(
      true, scale.boolScale, "allpass1Saturation", Info::kCanAutomate);
    value[ID::allpass1Time] = std::make_unique<LogValue>(
      0.5, scale.allpassTime, "allpass1Time", Info::kCanAutomate);
    value[ID::allpass1Feedback] = std::make_unique<LogValue>(
      0.75, scale.allpassFeedback, "allpass1Feedback", Info::kCanAutomate);
    value[ID::allpass1HighpassCutoff] = std::make_unique<LogValue>(
      0.5, scale.allpassHighpassCutoff, "allpass1HighpassCutoff", Info::kCanAutomate);
    value[ID::allpass2Time] = std::make_unique<LogValue>(
      0.5, scale.allpassTime, "allpass2Time", Info::kCanAutomate);
    value[ID::allpass2Feedback] = std::make_unique<LogValue>(
      0.5, scale.allpassFeedback, "allpass2Feedback", Info::kCanAutomate);
    value[ID::allpass2HighpassCutoff] = std::make_unique<LogValue>(
      0.5, scale.allpassHighpassCutoff, "allpass2HighpassCutoff", Info::kCanAutomate);

    value[ID::tremoloMix] = std::make_unique<LinearValue>(
      0.2, scale.defaultScale, "tremoloMix", Info::kCanAutomate);
    value[ID::tremoloDepth] = std::make_unique<LinearValue>(
      0.8, scale.defaultScale, "tremoloDepth", Info::kCanAutomate);
    value[ID::tremoloFrequency] = std::make_unique<LogValue>(
      0.5, scale.tremoloFrequency, "tremoloFrequency", Info::kCanAutomate);
    value[ID::tremoloDelayTime] = std::make_unique<LogValue>(
      0.25, scale.tremoloDelayTime, "tremoloDelayTime", Info::kCanAutomate);
    value[ID::randomTremoloDepth] = std::make_unique<LinearValue>(
      0.35, scale.defaultScale, "randomTremoloDepth", Info::kCanAutomate);
    value[ID::randomTremoloFrequency] = std::make_unique<LinearValue>(
      0.35, scale.defaultScale, "randomTremoloFrequency", Info::kCanAutomate);
    value[ID::randomTremoloDelayTime] = std::make_unique<LinearValue>(
      0.35, scale.defaultScale, "randomTremoloDelayTime", Info::kCanAutomate);

    value[ID::stick]
      = std::make_unique<UIntValue>(true, scale.boolScale, "stick", Info::kCanAutomate);
    value[ID::stickDecay] = std::make_unique<LogValue>(
      0.5, scale.stickDecay, "stickDecay", Info::kCanAutomate);
    value[ID::stickToneMix] = std::make_unique<LogValue>(
      0.5, scale.stickToneMix, "stickToneMix", Info::kCanAutomate);
    value[ID::stickPulseMix] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "stickPulseMix", Info::kCanAutomate);
    value[ID::stickVelvetMix] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "stickVelvetMix", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pitchBend", Info::kCanAutomate);

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
