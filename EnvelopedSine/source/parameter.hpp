// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <iostream>

#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

constexpr size_t nOvertone = 64;

namespace ParameterID {
enum ID {
  bypass,

  // 64 for each.
  attack0,
  decay0 = 65,
  overtone0 = 129,
  saturation0 = 193,

  // Do not add parameter here.

  gain = 257,
  gainBoost,

  aliasing,
  masterOctave,
  pitchMultiply,
  pitchModulo,

  seed,
  randomRetrigger,
  randomGain,
  randomFrequency,
  randomAttack,
  randomDecay,
  randomSaturation,
  randomPhase,

  overtoneExpand,
  overtoneShift,

  attackMultiplier,
  decayMultiplier,
  declick,
  gainPower,
  saturationMix,

  phaserMix,
  phaserFrequency,
  phaserFeedback,
  phaserRange,
  phaserMin,
  phaserPhase,
  phaserOffset,
  phaserStage,

  unison,
  nVoice,
  smoothness,

  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::DecibelScale<double> gainDecibel;
  static SomeDSP::LogScale<double> saturation;

  static SomeDSP::LogScale<double> gain;
  static SomeDSP::LinearScale<double> gainBoost;

  static SomeDSP::LinearScale<double> masterOctave;
  static SomeDSP::LinearScale<double> equalTemperament;
  static SomeDSP::LinearScale<double> pitchMultiply;
  static SomeDSP::LinearScale<double> pitchModulo;

  static SomeDSP::IntScale<double> seed;
  static SomeDSP::LinearScale<double> randomGain;
  static SomeDSP::LogScale<double> randomFrequency;
  static SomeDSP::LinearScale<double> randomAttack;
  static SomeDSP::LinearScale<double> randomDecay;
  static SomeDSP::LinearScale<double> randomSaturation;
  static SomeDSP::LinearScale<double> randomPhase;

  static SomeDSP::LogScale<double> overtoneExpand;
  static SomeDSP::LinearScale<double> overtoneShift;

  static SomeDSP::LogScale<double> envelopeMultiplier;
  static SomeDSP::LinearScale<double> gainPower;

  static SomeDSP::LogScale<double> phaserFrequency;
  static SomeDSP::LinearScale<double> phaserFeedback;
  static SomeDSP::LogScale<double> phaserRange;
  static SomeDSP::LinearScale<double> phaserPhase;
  static SomeDSP::IntScale<double> phaserStage;

  static SomeDSP::IntScale<double> nVoice;
  static SomeDSP::LogScale<double> smoothness;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;
    using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string attackLabel("attack");
    std::string decayLabel("decay");
    std::string overtoneLabel("overtone");
    std::string saturationLabel("saturation");
    for (size_t i = 0; i < nOvertone; ++i) {
      auto indexStr = std::to_string(i);
      value[ID::attack0 + i] = std::make_unique<LogValue>(
        0.0, Scales::envelopeA, attackLabel + indexStr, Info::kCanAutomate);
      value[ID::decay0 + i] = std::make_unique<LogValue>(
        0.5, Scales::envelopeD, decayLabel + indexStr, Info::kCanAutomate);
      value[ID::overtone0 + i] = std::make_unique<DecibelValue>(
        Scales::gainDecibel.invmap(1.0 / (i + 1)), Scales::gainDecibel,
        overtoneLabel + indexStr, Info::kCanAutomate);
      value[ID::saturation0 + i] = std::make_unique<LogValue>(
        0.0, Scales::saturation, saturationLabel + indexStr, Info::kCanAutomate);
    }

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, Scales::gain, "gain", Info::kCanAutomate);
    value[ID::gainBoost] = std::make_unique<LinearValue>(
      0.0, Scales::gainBoost, "gainBoost", Info::kCanAutomate);

    value[ID::aliasing]
      = std::make_unique<IntValue>(0, Scales::boolScale, "aliasing", Info::kCanAutomate);
    value[ID::masterOctave] = std::make_unique<LinearValue>(
      0.5, Scales::masterOctave, "masterOctave", Info::kCanAutomate);
    value[ID::pitchMultiply] = std::make_unique<LinearValue>(
      Scales::pitchMultiply.invmap(1.0), Scales::pitchMultiply, "pitchMultiply",
      Info::kCanAutomate);
    value[ID::pitchModulo] = std::make_unique<LinearValue>(
      0.0, Scales::pitchModulo, "pitchModulo", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<IntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::randomRetrigger] = std::make_unique<IntValue>(
      0, Scales::boolScale, "randomRetrigger", Info::kCanAutomate);
    value[ID::randomGain] = std::make_unique<LinearValue>(
      0.0, Scales::randomGain, "randomGain", Info::kCanAutomate);
    value[ID::randomFrequency] = std::make_unique<LogValue>(
      0.0, Scales::randomFrequency, "randomFrequency", Info::kCanAutomate);
    value[ID::randomAttack] = std::make_unique<LinearValue>(
      0.0, Scales::randomAttack, "randomAttack", Info::kCanAutomate);
    value[ID::randomDecay] = std::make_unique<LinearValue>(
      0.0, Scales::randomDecay, "randomDecay", Info::kCanAutomate);
    value[ID::randomSaturation] = std::make_unique<LinearValue>(
      0.0, Scales::randomSaturation, "randomSaturation", Info::kCanAutomate);
    value[ID::randomPhase] = std::make_unique<LinearValue>(
      0.0, Scales::randomPhase, "randomPhase", Info::kCanAutomate);

    value[ID::overtoneExpand] = std::make_unique<LogValue>(
      Scales::overtoneExpand.invmap(1.0), Scales::overtoneExpand, "overtoneExpand",
      Info::kCanAutomate);
    value[ID::overtoneShift] = std::make_unique<LinearValue>(
      0, Scales::overtoneShift, "overtoneShift", Info::kCanAutomate);

    value[ID::attackMultiplier] = std::make_unique<LogValue>(
      Scales::envelopeMultiplier.invmap(1.0), Scales::envelopeMultiplier,
      "attackMultiplier", Info::kCanAutomate);
    value[ID::decayMultiplier] = std::make_unique<LogValue>(
      Scales::envelopeMultiplier.invmap(1.0), Scales::envelopeMultiplier,
      "decayMultiplier", Info::kCanAutomate);
    value[ID::declick]
      = std::make_unique<IntValue>(1, Scales::boolScale, "declick", Info::kCanAutomate);
    value[ID::gainPower] = std::make_unique<LinearValue>(
      Scales::gainPower.invmap(1.0), Scales::gainPower, "gainPower", Info::kCanAutomate);
    value[ID::saturationMix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "saturationMix", Info::kCanAutomate);

    value[ID::phaserMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "phaserMix", Info::kCanAutomate);
    value[ID::phaserFrequency] = std::make_unique<LogValue>(
      0.5, Scales::phaserFrequency, "phaserFrequency", Info::kCanAutomate);
    value[ID::phaserFeedback] = std::make_unique<LinearValue>(
      0.5, Scales::phaserFeedback, "phaserFeedback", Info::kCanAutomate);
    value[ID::phaserRange] = std::make_unique<LogValue>(
      1.0, Scales::phaserRange, "phaserRange", Info::kCanAutomate);
    value[ID::phaserMin] = std::make_unique<LogValue>(
      0.0, Scales::phaserRange, "phaserMin", Info::kCanAutomate);
    value[ID::phaserPhase] = std::make_unique<LinearValue>(
      0.0, Scales::phaserPhase, "phaserPhase", Info::kCanAutomate);
    value[ID::phaserOffset] = std::make_unique<LinearValue>(
      0.5, Scales::phaserPhase, "phaserOffset", Info::kCanAutomate);
    value[ID::phaserStage] = std::make_unique<IntValue>(
      15, Scales::phaserStage, "phaserStage", Info::kCanAutomate);

    value[ID::unison]
      = std::make_unique<IntValue>(0, Scales::boolScale, "unison", Info::kCanAutomate);
    value[ID::nVoice]
      = std::make_unique<IntValue>(5, Scales::nVoice, "nVoice", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pitchBend", Info::kCanAutomate);

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
