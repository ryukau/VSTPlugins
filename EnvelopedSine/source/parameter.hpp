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
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , envelopeA(0.0, 16.0, 0.5, 0.5)
    , envelopeD(0.0, 4.0, 0.5, 0.5)
    , gainDecibel(-40.0, 0.0, true)
    , saturation(0.5, 5.0, 0.5, 2.0)
    , gain(0.0, 32.0, 0.5, 8.0)
    , gainBoost(1.0, 8.0)
    , masterOctave(-4.0, 4.0)
    , equalTemperament(1.0, 60.0)
    , pitchMultiply(0.001, 4.0)
    , pitchModulo(0.0, 60.0)
    , seed(16777215)
    , randomGain(0.0, 4.0)
    , randomFrequency(0.0, 1.0, 0.5, 0.1)
    , randomAttack(0.0, 1.0)
    , randomDecay(0.0, 1.0)
    , randomSaturation(0.0, 1.0)
    , randomPhase(0.0, 1.0)
    , overtoneExpand(0.01, 4.0, 0.4, 1.0)
    , overtoneShift(0.0, 64.0)
    , envelopeMultiplier(0.001, 4.0, 0.4, 1.0)
    , gainPower(0.001, 16.0)
    , phaserFrequency(0.000, 16.0, 0.5, 2.0)
    , phaserFeedback(-1.0, 1.0)
    , phaserRange(0.0, 128.0, 0.5, 32.0)
    , phaserPhase(0.0, SomeDSP::twopi)
    , phaserStage(15)
    , nVoice(5)
    , smoothness(0.0, 0.5, 0.1, 0.04)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::DecibelScale<double> gainDecibel;
  SomeDSP::LogScale<double> saturation;

  SomeDSP::LogScale<double> gain;
  SomeDSP::LinearScale<double> gainBoost;

  SomeDSP::LinearScale<double> masterOctave;
  SomeDSP::LinearScale<double> equalTemperament;
  SomeDSP::LinearScale<double> pitchMultiply;
  SomeDSP::LinearScale<double> pitchModulo;

  SomeDSP::UIntScale<double> seed;
  SomeDSP::LinearScale<double> randomGain;
  SomeDSP::LogScale<double> randomFrequency;
  SomeDSP::LinearScale<double> randomAttack;
  SomeDSP::LinearScale<double> randomDecay;
  SomeDSP::LinearScale<double> randomSaturation;
  SomeDSP::LinearScale<double> randomPhase;

  SomeDSP::LogScale<double> overtoneExpand;
  SomeDSP::LinearScale<double> overtoneShift;

  SomeDSP::LogScale<double> envelopeMultiplier;
  SomeDSP::LinearScale<double> gainPower;

  SomeDSP::LogScale<double> phaserFrequency;
  SomeDSP::LinearScale<double> phaserFeedback;
  SomeDSP::LogScale<double> phaserRange;
  SomeDSP::LinearScale<double> phaserPhase;
  SomeDSP::UIntScale<double> phaserStage;

  SomeDSP::UIntScale<double> nVoice;
  SomeDSP::LogScale<double> smoothness;
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
    using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string attackLabel("attack");
    std::string decayLabel("decay");
    std::string overtoneLabel("overtone");
    std::string saturationLabel("saturation");
    for (size_t i = 0; i < nOvertone; ++i) {
      auto indexStr = std::to_string(i);
      value[ID::attack0 + i] = std::make_unique<LogValue>(
        0.0, scale.envelopeA, attackLabel + indexStr, Info::kCanAutomate);
      value[ID::decay0 + i] = std::make_unique<LogValue>(
        0.5, scale.envelopeD, decayLabel + indexStr, Info::kCanAutomate);
      value[ID::overtone0 + i] = std::make_unique<DecibelValue>(
        scale.gainDecibel.invmap(1.0 / (i + 1)), scale.gainDecibel,
        overtoneLabel + indexStr, Info::kCanAutomate);
      value[ID::saturation0 + i] = std::make_unique<LogValue>(
        0.0, scale.saturation, saturationLabel + indexStr, Info::kCanAutomate);
    }

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::gainBoost] = std::make_unique<LinearValue>(
      0.0, scale.gainBoost, "gainBoost", Info::kCanAutomate);

    value[ID::aliasing]
      = std::make_unique<UIntValue>(0, scale.boolScale, "aliasing", Info::kCanAutomate);
    value[ID::masterOctave] = std::make_unique<LinearValue>(
      0.5, scale.masterOctave, "masterOctave", Info::kCanAutomate);
    value[ID::pitchMultiply] = std::make_unique<LinearValue>(
      scale.pitchMultiply.invmap(1.0), scale.pitchMultiply, "pitchMultiply",
      Info::kCanAutomate);
    value[ID::pitchModulo] = std::make_unique<LinearValue>(
      0.0, scale.pitchModulo, "pitchModulo", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, scale.seed, "seed", Info::kCanAutomate);
    value[ID::randomRetrigger] = std::make_unique<UIntValue>(
      0, scale.boolScale, "randomRetrigger", Info::kCanAutomate);
    value[ID::randomGain] = std::make_unique<LinearValue>(
      0.0, scale.randomGain, "randomGain", Info::kCanAutomate);
    value[ID::randomFrequency] = std::make_unique<LogValue>(
      0.0, scale.randomFrequency, "randomFrequency", Info::kCanAutomate);
    value[ID::randomAttack] = std::make_unique<LinearValue>(
      0.0, scale.randomAttack, "randomAttack", Info::kCanAutomate);
    value[ID::randomDecay] = std::make_unique<LinearValue>(
      0.0, scale.randomDecay, "randomDecay", Info::kCanAutomate);
    value[ID::randomSaturation] = std::make_unique<LinearValue>(
      0.0, scale.randomSaturation, "randomSaturation", Info::kCanAutomate);
    value[ID::randomPhase] = std::make_unique<LinearValue>(
      0.0, scale.randomPhase, "randomPhase", Info::kCanAutomate);

    value[ID::overtoneExpand] = std::make_unique<LogValue>(
      scale.overtoneExpand.invmap(1.0), scale.overtoneExpand, "overtoneExpand",
      Info::kCanAutomate);
    value[ID::overtoneShift] = std::make_unique<LinearValue>(
      0, scale.overtoneShift, "overtoneShift", Info::kCanAutomate);

    value[ID::attackMultiplier] = std::make_unique<LogValue>(
      scale.envelopeMultiplier.invmap(1.0), scale.envelopeMultiplier, "attackMultiplier",
      Info::kCanAutomate);
    value[ID::decayMultiplier] = std::make_unique<LogValue>(
      scale.envelopeMultiplier.invmap(1.0), scale.envelopeMultiplier, "decayMultiplier",
      Info::kCanAutomate);
    value[ID::declick]
      = std::make_unique<UIntValue>(1, scale.boolScale, "declick", Info::kCanAutomate);
    value[ID::gainPower] = std::make_unique<LinearValue>(
      scale.gainPower.invmap(1.0), scale.gainPower, "gainPower", Info::kCanAutomate);
    value[ID::saturationMix] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "saturationMix", Info::kCanAutomate);

    value[ID::phaserMix] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "phaserMix", Info::kCanAutomate);
    value[ID::phaserFrequency] = std::make_unique<LogValue>(
      0.5, scale.phaserFrequency, "phaserFrequency", Info::kCanAutomate);
    value[ID::phaserFeedback] = std::make_unique<LinearValue>(
      0.5, scale.phaserFeedback, "phaserFeedback", Info::kCanAutomate);
    value[ID::phaserRange] = std::make_unique<LogValue>(
      1.0, scale.phaserRange, "phaserRange", Info::kCanAutomate);
    value[ID::phaserMin] = std::make_unique<LogValue>(
      0.0, scale.phaserRange, "phaserMin", Info::kCanAutomate);
    value[ID::phaserPhase] = std::make_unique<LinearValue>(
      0.0, scale.phaserPhase, "phaserPhase", Info::kCanAutomate);
    value[ID::phaserOffset] = std::make_unique<LinearValue>(
      0.5, scale.phaserPhase, "phaserOffset", Info::kCanAutomate);
    value[ID::phaserStage] = std::make_unique<UIntValue>(
      15, scale.phaserStage, "phaserStage", Info::kCanAutomate);

    value[ID::unison]
      = std::make_unique<UIntValue>(0, scale.boolScale, "unison", Info::kCanAutomate);
    value[ID::nVoice]
      = std::make_unique<UIntValue>(5, scale.nVoice, "nVoice", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);

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
