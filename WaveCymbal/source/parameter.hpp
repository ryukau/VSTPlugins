// (c) 2019-2020 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

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
  randomAmount,
  nCymbal,
  stack,
  decay,
  distance,
  minFrequency,
  maxFrequency,
  bandpassQ,
  damping,
  pulsePosition,
  pulseWidth,
  pickCombFeedback,
  pickCombTime,
  retrigger,
  cutoffMap,
  excitation,
  collision,
  oscType,
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
    , randomAmount(0.0, 1.0, 0.5, 0.1)
    , nCymbal(0.0, 3.0)
    , decay(0.0, 16.0, 0.5, 4.0)
    , damping(0.0, 0.999, 0.5, 0.9)
    , minFrequency(0.0, 1000.0, 0.5, 100.0)
    , maxFrequency(10.0, 4000.0, 0.5, 400.0)
    , bandpassQ(0.0001, 0.9999)
    , distance(0.0, 8.0, 0.5, 0.1)
    , stack(63)
    , pickCombFeedback(0.0, 0.9999, 0.5, 0.7)
    , pickCombTime(0.005, 0.4, 0.5, 0.1)
    , oscType(4)
    , smoothness(0.0, 0.1, 0.2, 0.02)
    , gain(0.0, 4.0, 0.75, 1.0)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::UIntScale<double> seed;
  SomeDSP::LogScale<double> randomAmount;
  SomeDSP::LinearScale<double> nCymbal;
  SomeDSP::LogScale<double> decay;
  SomeDSP::LogScale<double> damping;
  SomeDSP::LogScale<double> minFrequency;
  SomeDSP::LogScale<double> maxFrequency;
  SomeDSP::LinearScale<double> bandpassQ;
  SomeDSP::LogScale<double> distance;
  SomeDSP::UIntScale<double> stack;
  SomeDSP::LogScale<double> pickCombFeedback;
  SomeDSP::LogScale<double> pickCombTime;
  SomeDSP::UIntScale<double> oscType;
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
      0, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::seed]
      = std::make_unique<UIntValue>(6583421, scale.seed, "seed", Info::kCanAutomate);
    value[ID::randomAmount] = std::make_unique<LogValue>(
      1.0, scale.randomAmount, "randomAmount", Info::kCanAutomate);
    value[ID::nCymbal]
      = std::make_unique<LinearValue>(1.0, scale.nCymbal, "nCymbal", Info::kCanAutomate);
    value[ID::stack]
      = std::make_unique<UIntValue>(24, scale.stack, "stack", Info::kCanAutomate);
    value[ID::decay]
      = std::make_unique<LogValue>(0.5, scale.decay, "decay", Info::kCanAutomate);
    value[ID::distance]
      = std::make_unique<LogValue>(0.5, scale.distance, "distance", Info::kCanAutomate);
    value[ID::minFrequency] = std::make_unique<LogValue>(
      0.5, scale.minFrequency, "minFrequency", Info::kCanAutomate);
    value[ID::maxFrequency] = std::make_unique<LogValue>(
      0.5, scale.maxFrequency, "maxFrequency", Info::kCanAutomate);
    value[ID::bandpassQ] = std::make_unique<LinearValue>(
      0.5, scale.bandpassQ, "bandpassQ", Info::kCanAutomate);
    value[ID::damping]
      = std::make_unique<LogValue>(0.5, scale.damping, "damping", Info::kCanAutomate);
    value[ID::pulsePosition] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pulsePosition", Info::kCanAutomate);
    value[ID::pulseWidth] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pulseWidth", Info::kCanAutomate);
    value[ID::pickCombFeedback] = std::make_unique<LogValue>(
      0.5, scale.pickCombFeedback, "pickCombFeedback", Info::kCanAutomate);
    value[ID::pickCombTime] = std::make_unique<LogValue>(
      0.25, scale.pickCombTime, "pickCombTime", Info::kCanAutomate);
    value[ID::retrigger]
      = std::make_unique<UIntValue>(0, scale.boolScale, "retrigger", Info::kCanAutomate);
    value[ID::cutoffMap]
      = std::make_unique<UIntValue>(0, scale.boolScale, "cutoffMap", Info::kCanAutomate);
    value[ID::excitation]
      = std::make_unique<UIntValue>(1, scale.boolScale, "excitation", Info::kCanAutomate);
    value[ID::collision]
      = std::make_unique<UIntValue>(1, scale.boolScale, "collision", Info::kCanAutomate);
    value[ID::oscType]
      = std::make_unique<UIntValue>(2, scale.oscType, "oscType", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.7, scale.smoothness, "smoothness", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.4, scale.gain, "gain", Info::kCanAutomate);
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
