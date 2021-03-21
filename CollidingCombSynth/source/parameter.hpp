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
#include "../../common/parameterInterface.hpp"

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
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> gain;
  static SomeDSP::LogScale<double> boost;

  static SomeDSP::LogScale<double> exciterGain;
  static SomeDSP::LogScale<double> exciterAttack;
  static SomeDSP::LogScale<double> exciterDecay;
  static SomeDSP::LogScale<double> exciterLowpassCutoff;

  static SomeDSP::LinearScale<double> combTime;
  static SomeDSP::LogScale<double> frequency;
  static SomeDSP::LogScale<double> lowpassCutoff;
  static SomeDSP::LogScale<double> highpassCutoff;

  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::LogScale<double> envelopeS;
  static SomeDSP::LogScale<double> envelopeR;

  static SomeDSP::LogScale<double> distance;
  static SomeDSP::LogScale<double> propagation;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::LogScale<double> randomFrequency;

  static SomeDSP::LogScale<double> compressorTime;
  static SomeDSP::LogScale<double> compressorThreshold;

  static SomeDSP::UIntScale<double> nVoice;
  static SomeDSP::UIntScale<double> nUnison;
  static SomeDSP::LogScale<double> unisonDetune;

  static SomeDSP::UIntScale<double> octave;
  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::UIntScale<double> milli;
  static SomeDSP::UIntScale<double> equalTemperament;
  static SomeDSP::UIntScale<double> pitchA4Hz;
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

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string frequencyLabel("frequency");
    for (size_t idx = 0; idx < nDelay; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::frequency0 + idx] = std::make_unique<LogValue>(
        Scales::frequency.invmap(100.0 + 20.0 * idx), Scales::frequency,
        (frequencyLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string combTimeLabel("combTime");
    for (size_t idx = 0; idx < nComb; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::combTime0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::combTime, (combTimeLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, Scales::gain, "gain", Info::kCanAutomate);
    value[ID::boost] = std::make_unique<LogValue>(
      Scales::boost.invmap(1.0), Scales::boost, "boost", Info::kCanAutomate);

    value[ID::exciterGain] = std::make_unique<LogValue>(
      0.5, Scales::exciterGain, "exciterGain", Info::kCanAutomate);
    value[ID::exciterAttack] = std::make_unique<LogValue>(
      0.0, Scales::exciterAttack, "exciterAttack", Info::kCanAutomate);
    value[ID::exciterDecay] = std::make_unique<LogValue>(
      0.0, Scales::exciterDecay, "exciterDecay", Info::kCanAutomate);
    value[ID::exciterNoiseMix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "exciterNoiseMix", Info::kCanAutomate);
    value[ID::exciterLowpassCutoff] = std::make_unique<LogValue>(
      0.5, Scales::exciterLowpassCutoff, "exciterLowpassCutoff", Info::kCanAutomate);

    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      Scales::lowpassCutoff.invmap(12000.0), Scales::lowpassCutoff, "lowpassCutoff",
      Info::kCanAutomate);
    value[ID::highpassCutoff] = std::make_unique<LogValue>(
      0.5, Scales::highpassCutoff, "highpassCutoff", Info::kCanAutomate);

    value[ID::lowpassA] = std::make_unique<LogValue>(
      0.0, Scales::envelopeA, "lowpassA", Info::kCanAutomate);
    value[ID::lowpassD] = std::make_unique<LogValue>(
      0.5, Scales::envelopeD, "lowpassD", Info::kCanAutomate);
    value[ID::lowpassS] = std::make_unique<LogValue>(
      0.5, Scales::envelopeS, "lowpassS", Info::kCanAutomate);
    value[ID::lowpassR] = std::make_unique<LogValue>(
      0.5, Scales::envelopeR, "lowpassR", Info::kCanAutomate);

    value[ID::distance]
      = std::make_unique<LogValue>(0.5, Scales::distance, "distance", Info::kCanAutomate);
    value[ID::propagation] = std::make_unique<LogValue>(
      Scales::propagation.invmap(0.9), Scales::propagation, "propagation",
      Info::kCanAutomate);
    value[ID::connection] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "connection", Info::kCanAutomate);

    value[ID::randomComb] = std::make_unique<LinearValue>(
      0.01, Scales::defaultScale, "randomComb", Info::kCanAutomate);
    value[ID::randomFrequency] = std::make_unique<LogValue>(
      0.0, Scales::randomFrequency, "randomFrequency", Info::kCanAutomate);

    value[ID::seedNoise] = std::make_unique<UIntValue>(
      11467559, Scales::seed, "seedNoise", Info::kCanAutomate);
    value[ID::seedComb] = std::make_unique<UIntValue>(
      14700349, Scales::seed, "seedComb", Info::kCanAutomate);
    value[ID::seedString] = std::make_unique<UIntValue>(
      3937273, Scales::seed, "seedString", Info::kCanAutomate);
    value[ID::seedUnison] = std::make_unique<UIntValue>(
      2060287, Scales::seed, "seedUnison", Info::kCanAutomate);

    value[ID::retriggerNoise] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "retriggerNoise", Info::kCanAutomate);
    value[ID::retriggerComb] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "retriggerComb", Info::kCanAutomate);
    value[ID::retriggerString] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "retriggerString", Info::kCanAutomate);
    value[ID::retriggerUnison] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "retriggerUnison", Info::kCanAutomate);

    value[ID::compressor] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "compressor", Info::kCanAutomate);
    value[ID::compressorTime] = std::make_unique<LogValue>(
      Scales::compressorTime.invmap(0.1), Scales::compressorTime, "compressorTime",
      Info::kCanAutomate);
    value[ID::compressorThreshold] = std::make_unique<LogValue>(
      Scales::compressorThreshold.invmap(0.5), Scales::compressorThreshold,
      "compressorThreshold", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<UIntValue>(7, Scales::nVoice, "nVoice", Info::kCanAutomate);
    value[ID::nUnison]
      = std::make_unique<UIntValue>(1, Scales::nUnison, "nUnison", Info::kCanAutomate);
    value[ID::unisonDetune] = std::make_unique<LogValue>(
      0.25, Scales::unisonDetune, "unisonDetune", Info::kCanAutomate);
    value[ID::unisonSpread] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "unisonSpread", Info::kCanAutomate);
    value[ID::unisonGainRandom] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "unisonGainRandom", Info::kCanAutomate);
    value[ID::unisonDetuneRandom] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "unisonDetuneRandom", Info::kCanAutomate);

    value[ID::octave]
      = std::make_unique<UIntValue>(12, Scales::octave, "octave", Info::kCanAutomate);
    value[ID::semitone] = std::make_unique<UIntValue>(
      120, Scales::semitone, "semitone", Info::kCanAutomate);
    value[ID::milli]
      = std::make_unique<UIntValue>(1000, Scales::milli, "milli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<UIntValue>(
      11, Scales::equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<UIntValue>(
      340, Scales::pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pitchBend", Info::kCanAutomate);

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
