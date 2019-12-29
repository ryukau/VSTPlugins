// (c) 2019 Takamitsu Endo
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

#include "value.hpp"

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
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::IntScale<double> seed;
  static SomeDSP::LogScale<double> randomAmount;
  static SomeDSP::LinearScale<double> nCymbal;
  static SomeDSP::LogScale<double> decay;
  static SomeDSP::LogScale<double> damping;
  static SomeDSP::LogScale<double> minFrequency;
  static SomeDSP::LogScale<double> maxFrequency;
  static SomeDSP::LinearScale<double> bandpassQ;
  static SomeDSP::LogScale<double> distance;
  static SomeDSP::IntScale<double> stack;
  static SomeDSP::LogScale<double> pickCombFeedback;
  static SomeDSP::LogScale<double> pickCombTime;
  static SomeDSP::IntScale<double> oscType;
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
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass, ID::bypass);

    value[ID::seed] = std::make_unique<IntValue>(
      6583421, Scales::seed, "seed", Info::kCanAutomate, ID::seed);
    value[ID::randomAmount] = std::make_unique<LogValue>(
      1.0, Scales::randomAmount, "randomAmount", Info::kCanAutomate, ID::randomAmount);
    value[ID::nCymbal] = std::make_unique<LinearValue>(
      1.0, Scales::nCymbal, "nCymbal", Info::kCanAutomate, ID::nCymbal);
    value[ID::stack] = std::make_unique<IntValue>(
      24, Scales::stack, "stack", Info::kCanAutomate, ID::stack);
    value[ID::decay] = std::make_unique<LogValue>(
      0.5, Scales::decay, "decay", Info::kCanAutomate, ID::decay);
    value[ID::distance] = std::make_unique<LogValue>(
      0.5, Scales::distance, "distance", Info::kCanAutomate, ID::distance);
    value[ID::minFrequency] = std::make_unique<LogValue>(
      0.5, Scales::minFrequency, "minFrequency", Info::kCanAutomate, ID::minFrequency);
    value[ID::maxFrequency] = std::make_unique<LogValue>(
      0.5, Scales::maxFrequency, "maxFrequency", Info::kCanAutomate, ID::maxFrequency);
    value[ID::bandpassQ] = std::make_unique<LinearValue>(
      0.5, Scales::bandpassQ, "bandpassQ", Info::kCanAutomate, ID::bandpassQ);
    value[ID::damping] = std::make_unique<LogValue>(
      0.5, Scales::damping, "damping", Info::kCanAutomate, ID::damping);
    value[ID::pulsePosition] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pulsePosition", Info::kCanAutomate, ID::pulsePosition);
    value[ID::pulseWidth] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pulseWidth", Info::kCanAutomate, ID::pulseWidth);
    value[ID::pickCombFeedback] = std::make_unique<LogValue>(
      0.5, Scales::pickCombFeedback, "pickCombFeedback", Info::kCanAutomate,
      ID::pickCombFeedback);
    value[ID::pickCombTime] = std::make_unique<LogValue>(
      0.25, Scales::pickCombTime, "pickCombTime", Info::kCanAutomate, ID::pickCombTime);
    value[ID::retrigger] = std::make_unique<IntValue>(
      0, Scales::boolScale, "retrigger", Info::kCanAutomate, ID::retrigger);
    value[ID::cutoffMap] = std::make_unique<IntValue>(
      0, Scales::boolScale, "cutoffMap", Info::kCanAutomate, ID::cutoffMap);
    value[ID::excitation] = std::make_unique<IntValue>(
      1, Scales::boolScale, "excitation", Info::kCanAutomate, ID::excitation);
    value[ID::collision] = std::make_unique<IntValue>(
      1, Scales::boolScale, "collision", Info::kCanAutomate, ID::collision);
    value[ID::oscType] = std::make_unique<IntValue>(
      2, Scales::oscType, "oscType", Info::kCanAutomate, ID::oscType);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.7, Scales::smoothness, "smoothness", Info::kCanAutomate, ID::smoothness);

    value[ID::gain] = std::make_unique<LogValue>(
      0.4, Scales::gain, "gain", Info::kCanAutomate, ID::gain);
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
