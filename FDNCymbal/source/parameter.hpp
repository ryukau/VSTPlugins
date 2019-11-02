// (c) 2019 Takamitsu Endo
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

#include "value.hpp"

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

  smoothness,

  gain,
  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::IntScale<double> seed;
  static SomeDSP::LogScale<double> fdnTime;
  static SomeDSP::LogScale<double> fdnFeedback;
  static SomeDSP::LogScale<double> fdnCascadeMix;
  static SomeDSP::LogScale<double> allpassTime;
  static SomeDSP::LogScale<double> allpassFeedback;
  static SomeDSP::LogScale<double> allpassHighpassCutoff;
  static SomeDSP::LogScale<double> tremoloFrequency;
  static SomeDSP::LogScale<double> tremoloDelayTime;
  static SomeDSP::LogScale<double> stickDecay;
  static SomeDSP::LogScale<double> stickToneMix;

  static SomeDSP::LogScale<double> smoothness;

  static SomeDSP::LogScale<double> gain;
};

struct GlobalParameter {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass,
      ID::bypass);

    value[ID::seed] = std::make_unique<IntValue>(
      6583421, Scales::seed, "seed", Info::kCanAutomate, ID::seed);
    value[ID::retriggerTime] = std::make_unique<IntValue>(
      true, Scales::boolScale, "retriggerTime", Info::kCanAutomate, ID::retriggerTime);
    value[ID::retriggerStick] = std::make_unique<IntValue>(
      false, Scales::boolScale, "retriggerStick", Info::kCanAutomate, ID::retriggerStick);
    value[ID::retriggerTremolo] = std::make_unique<IntValue>(
      false, Scales::boolScale, "retriggerTremolo", Info::kCanAutomate,
      ID::retriggerTremolo);

    value[ID::fdn] = std::make_unique<IntValue>(
      true, Scales::boolScale, "fdn", Info::kCanAutomate, ID::fdn);
    value[ID::fdnTime] = std::make_unique<LogValue>(
      0.2, Scales::fdnTime, "fdnTime", Info::kCanAutomate, ID::fdnTime);
    value[ID::fdnFeedback] = std::make_unique<LogValue>(
      0.5, Scales::fdnFeedback, "fdnFeedback", Info::kCanAutomate, ID::fdnFeedback);
    value[ID::fdnCascadeMix] = std::make_unique<LogValue>(
      0.5, Scales::fdnCascadeMix, "fdnCascadeMix", Info::kCanAutomate, ID::fdnCascadeMix);

    value[ID::allpassMix] = std::make_unique<LinearValue>(
      0.75, Scales::defaultScale, "allpassMix", Info::kCanAutomate, ID::allpassMix);
    value[ID::allpass1Saturation] = std::make_unique<IntValue>(
      true, Scales::boolScale, "allpass1Saturation", Info::kCanAutomate,
      ID::allpass1Saturation);
    value[ID::allpass1Time] = std::make_unique<LogValue>(
      0.5, Scales::allpassTime, "allpass1Time", Info::kCanAutomate, ID::allpass1Time);
    value[ID::allpass1Feedback] = std::make_unique<LogValue>(
      0.75, Scales::allpassFeedback, "allpass1Feedback", Info::kCanAutomate,
      ID::allpass1Feedback);
    value[ID::allpass1HighpassCutoff] = std::make_unique<LogValue>(
      0.5, Scales::allpassHighpassCutoff, "allpass1HighpassCutoff", Info::kCanAutomate,
      ID::allpass1HighpassCutoff);
    value[ID::allpass2Time] = std::make_unique<LogValue>(
      0.5, Scales::allpassTime, "allpass2Time", Info::kCanAutomate, ID::allpass2Time);
    value[ID::allpass2Feedback] = std::make_unique<LogValue>(
      0.5, Scales::allpassFeedback, "allpass2Feedback", Info::kCanAutomate,
      ID::allpass2Feedback);
    value[ID::allpass2HighpassCutoff] = std::make_unique<LogValue>(
      0.5, Scales::allpassHighpassCutoff, "allpass2HighpassCutoff", Info::kCanAutomate,
      ID::allpass2HighpassCutoff);

    value[ID::tremoloMix] = std::make_unique<LinearValue>(
      0.2, Scales::defaultScale, "tremoloMix", Info::kCanAutomate, ID::tremoloMix);
    value[ID::tremoloDepth] = std::make_unique<LinearValue>(
      0.8, Scales::defaultScale, "tremoloDepth", Info::kCanAutomate, ID::tremoloDepth);
    value[ID::tremoloFrequency] = std::make_unique<LogValue>(
      0.5, Scales::tremoloFrequency, "tremoloFrequency", Info::kCanAutomate,
      ID::tremoloFrequency);
    value[ID::tremoloDelayTime] = std::make_unique<LogValue>(
      0.25, Scales::tremoloDelayTime, "tremoloDelayTime", Info::kCanAutomate,
      ID::tremoloDelayTime);
    value[ID::randomTremoloDepth] = std::make_unique<LinearValue>(
      0.35, Scales::defaultScale, "randomTremoloDepth", Info::kCanAutomate,
      ID::randomTremoloDepth);
    value[ID::randomTremoloFrequency] = std::make_unique<LinearValue>(
      0.35, Scales::defaultScale, "randomTremoloFrequency", Info::kCanAutomate,
      ID::randomTremoloFrequency);
    value[ID::randomTremoloDelayTime] = std::make_unique<LinearValue>(
      0.35, Scales::defaultScale, "randomTremoloDelayTime", Info::kCanAutomate,
      ID::randomTremoloDelayTime);

    value[ID::stick] = std::make_unique<IntValue>(
      true, Scales::boolScale, "stick", Info::kCanAutomate, ID::stick);
    value[ID::stickDecay] = std::make_unique<LogValue>(
      0.5, Scales::stickDecay, "stickDecay", Info::kCanAutomate, ID::stickDecay);
    value[ID::stickToneMix] = std::make_unique<LogValue>(
      0.5, Scales::stickToneMix, "stickToneMix", Info::kCanAutomate, ID::stickToneMix);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate, ID::smoothness);

    value[ID::gain] = std::make_unique<LogValue>(
      0.5, Scales::gain, "gain", Info::kCanAutomate, ID::gain);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pitchBend", Info::kCanAutomate, ID::pitchBend);
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
};

} // namespace Synth
} // namespace Steinberg
