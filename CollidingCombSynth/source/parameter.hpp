// (c) 2020 Takamitsu Endo
//
// This file is part of CollidingCombSynth.
//
// CollidingCombSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CollidingCombSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CollidingCombSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"

#ifdef TEST_DSP
#include "../../test/value.hpp"
#else
#include "../../common/value.hpp"
#endif

constexpr uint16_t nDelay = 24;
constexpr uint16_t nComb = 8;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  frequency0 = 1,

  combTime0 = 1 + nDelay,

  gain = 1 + nDelay + nComb,
  boost,

  exciterGain,
  exciterAttack,
  exciterDecay,
  exciterNoiseMix,
  exciterLowpassCutoff,

  lowpassCutoff,
  highpassCutoff,

  lowpassA,
  lowpassD,
  lowpassS,
  lowpassR,

  distance,
  propagation,
  connection,

  randomComb,
  randomFrequency,

  seedNoise,
  seedComb,
  seedString,
  seedUnison,

  retriggerNoise,
  retriggerComb,
  retriggerString,
  retriggerUnison,

  compressor,
  compressorTime,
  compressorThreshold,

  nVoice,
  nUnison,
  unisonDetune,
  unisonSpread,
  unisonGainRandom,
  unisonDetuneRandom,

  octave,
  semitone,
  milli,
  equalTemperament,
  pitchA4Hz,
  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , gain(0.0, 1.0, 0.5, 0.2)
    , boost(0.0, 8.0, 0.5, 1.0)
    , exciterGain(0.001, 2.0, 0.5, 0.5)
    , exciterAttack(0.001, 0.2, 0.5, 0.1)
    , exciterDecay(0.001, 0.5, 0.5, 0.1)
    , exciterLowpassCutoff(1.0, 20000.0, 0.5, 100.0)
    , combTime(0.0001, 0.002)
    , frequency(12.0, 1000.0, 0.5, 100.0)
    , lowpassCutoff(20.0, 20000.0, 0.5, 1000.0)
    , highpassCutoff(20.0, 20000.0, 0.5, 400.0)
    , envelopeA(0.0001, 16.0, 0.5, 2.0)
    , envelopeD(0.0001, 16.0, 0.5, 4.0)
    , envelopeS(0.0, 0.9995, 0.5, 0.3)
    , envelopeR(0.001, 16.0, 0.5, 2.0)
    , distance(0.001, 1.0, 0.5, 0.2)
    , propagation(0.001, 1.0, 0.5, 0.7)
    , seed(16777215)
    , randomFrequency(0.0, 1.0, 0.5, 0.05)
    , compressorTime(0.001, 8.0, 0.5, 1.0)
    , compressorThreshold(0.01, 2.0, 0.5, 0.5)
    , nVoice(15)
    , nUnison(7)
    , unisonDetune(0.0, 0.3, 0.2, 0.001)
    , octave(16)
    , semitone(168)
    , milli(2000)
    , equalTemperament(119)
    , pitchA4Hz(900)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> gain;
  SomeDSP::LogScale<double> boost;

  SomeDSP::LogScale<double> exciterGain;
  SomeDSP::LogScale<double> exciterAttack;
  SomeDSP::LogScale<double> exciterDecay;
  SomeDSP::LogScale<double> exciterLowpassCutoff;

  SomeDSP::LinearScale<double> combTime;
  SomeDSP::LogScale<double> frequency;
  SomeDSP::LogScale<double> lowpassCutoff;
  SomeDSP::LogScale<double> highpassCutoff;

  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::LogScale<double> envelopeS;
  SomeDSP::LogScale<double> envelopeR;

  SomeDSP::LogScale<double> distance;
  SomeDSP::LogScale<double> propagation;
  SomeDSP::UIntScale<double> seed;

  SomeDSP::LogScale<double> randomFrequency;

  SomeDSP::LogScale<double> compressorTime;
  SomeDSP::LogScale<double> compressorThreshold;

  SomeDSP::UIntScale<double> nVoice;
  SomeDSP::UIntScale<double> nUnison;
  SomeDSP::LogScale<double> unisonDetune;

  SomeDSP::UIntScale<double> octave;
  SomeDSP::UIntScale<double> semitone;
  SomeDSP::UIntScale<double> milli;
  SomeDSP::UIntScale<double> equalTemperament;
  SomeDSP::UIntScale<double> pitchA4Hz;
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
      false, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string frequencyLabel("frequency");
    for (size_t idx = 0; idx < nDelay; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::frequency0 + idx] = std::make_unique<LogValue>(
        scale.frequency.invmap(100.0 + 20.0 * idx), scale.frequency,
        (frequencyLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string combTimeLabel("combTime");
    for (size_t idx = 0; idx < nComb; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::combTime0 + idx] = std::make_unique<LinearValue>(
        0.5, scale.combTime, (combTimeLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::boost] = std::make_unique<LogValue>(
      scale.boost.invmap(1.0), scale.boost, "boost", Info::kCanAutomate);

    value[ID::exciterGain] = std::make_unique<LogValue>(
      0.5, scale.exciterGain, "exciterGain", Info::kCanAutomate);
    value[ID::exciterAttack] = std::make_unique<LogValue>(
      0.0, scale.exciterAttack, "exciterAttack", Info::kCanAutomate);
    value[ID::exciterDecay] = std::make_unique<LogValue>(
      0.0, scale.exciterDecay, "exciterDecay", Info::kCanAutomate);
    value[ID::exciterNoiseMix] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "exciterNoiseMix", Info::kCanAutomate);
    value[ID::exciterLowpassCutoff] = std::make_unique<LogValue>(
      0.5, scale.exciterLowpassCutoff, "exciterLowpassCutoff", Info::kCanAutomate);

    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      scale.lowpassCutoff.invmap(12000.0), scale.lowpassCutoff, "lowpassCutoff",
      Info::kCanAutomate);
    value[ID::highpassCutoff] = std::make_unique<LogValue>(
      0.5, scale.highpassCutoff, "highpassCutoff", Info::kCanAutomate);

    value[ID::lowpassA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "lowpassA", Info::kCanAutomate);
    value[ID::lowpassD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "lowpassD", Info::kCanAutomate);
    value[ID::lowpassS]
      = std::make_unique<LogValue>(0.5, scale.envelopeS, "lowpassS", Info::kCanAutomate);
    value[ID::lowpassR]
      = std::make_unique<LogValue>(0.5, scale.envelopeR, "lowpassR", Info::kCanAutomate);

    value[ID::distance]
      = std::make_unique<LogValue>(0.5, scale.distance, "distance", Info::kCanAutomate);
    value[ID::propagation] = std::make_unique<LogValue>(
      scale.propagation.invmap(0.9), scale.propagation, "propagation",
      Info::kCanAutomate);
    value[ID::connection]
      = std::make_unique<UIntValue>(0, scale.boolScale, "connection", Info::kCanAutomate);

    value[ID::randomComb] = std::make_unique<LinearValue>(
      0.01, scale.defaultScale, "randomComb", Info::kCanAutomate);
    value[ID::randomFrequency] = std::make_unique<LogValue>(
      0.0, scale.randomFrequency, "randomFrequency", Info::kCanAutomate);

    value[ID::seedNoise] = std::make_unique<UIntValue>(
      11467559, scale.seed, "seedNoise", Info::kCanAutomate);
    value[ID::seedComb]
      = std::make_unique<UIntValue>(14700349, scale.seed, "seedComb", Info::kCanAutomate);
    value[ID::seedString] = std::make_unique<UIntValue>(
      3937273, scale.seed, "seedString", Info::kCanAutomate);
    value[ID::seedUnison] = std::make_unique<UIntValue>(
      2060287, scale.seed, "seedUnison", Info::kCanAutomate);

    value[ID::retriggerNoise] = std::make_unique<UIntValue>(
      false, scale.boolScale, "retriggerNoise", Info::kCanAutomate);
    value[ID::retriggerComb] = std::make_unique<UIntValue>(
      false, scale.boolScale, "retriggerComb", Info::kCanAutomate);
    value[ID::retriggerString] = std::make_unique<UIntValue>(
      true, scale.boolScale, "retriggerString", Info::kCanAutomate);
    value[ID::retriggerUnison] = std::make_unique<UIntValue>(
      false, scale.boolScale, "retriggerUnison", Info::kCanAutomate);

    value[ID::compressor]
      = std::make_unique<UIntValue>(1, scale.boolScale, "compressor", Info::kCanAutomate);
    value[ID::compressorTime] = std::make_unique<LogValue>(
      scale.compressorTime.invmap(0.1), scale.compressorTime, "compressorTime",
      Info::kCanAutomate);
    value[ID::compressorThreshold] = std::make_unique<LogValue>(
      scale.compressorThreshold.invmap(0.5), scale.compressorThreshold,
      "compressorThreshold", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<UIntValue>(7, scale.nVoice, "nVoice", Info::kCanAutomate);
    value[ID::nUnison]
      = std::make_unique<UIntValue>(1, scale.nUnison, "nUnison", Info::kCanAutomate);
    value[ID::unisonDetune] = std::make_unique<LogValue>(
      0.25, scale.unisonDetune, "unisonDetune", Info::kCanAutomate);
    value[ID::unisonSpread] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "unisonSpread", Info::kCanAutomate);
    value[ID::unisonGainRandom] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "unisonGainRandom", Info::kCanAutomate);
    value[ID::unisonDetuneRandom] = std::make_unique<UIntValue>(
      1, scale.boolScale, "unisonDetuneRandom", Info::kCanAutomate);

    value[ID::octave]
      = std::make_unique<UIntValue>(12, scale.octave, "octave", Info::kCanAutomate);
    value[ID::semitone]
      = std::make_unique<UIntValue>(120, scale.semitone, "semitone", Info::kCanAutomate);
    value[ID::milli]
      = std::make_unique<UIntValue>(1000, scale.milli, "milli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<UIntValue>(
      11, scale.equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<UIntValue>(
      340, scale.pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pitchBend", Info::kCanAutomate);

#ifndef TEST_DSP
    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
#endif
  }

#ifdef TEST_DSP
  // Not used in DSP test.
  double getDefaultNormalized(int32_t tag) { return 0.0; }

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

  double getDefaultNormalized(int32_t tag)
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
#endif
};

} // namespace Synth
} // namespace Steinberg
