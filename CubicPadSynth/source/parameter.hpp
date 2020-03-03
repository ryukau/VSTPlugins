// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

constexpr int32_t nOvertone = 360;
constexpr int32_t nLFOWavetable = 64;
constexpr int32_t tableSize = 262144;
constexpr int32_t lfoTableSize = 1023;
constexpr int32_t spectrumSize = tableSize / 2 + 1;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  overtoneGain0,
  overtoneWidth0 = 361,
  overtonePitch0 = 721,
  overtonePhase0 = 1081,

  lfoWavetable0 = 1441,

  // Do not add parameter here.

  tableBaseFrequency = 1505,
  padSynthSeed,
  overtoneGainPower,
  overtoneWidthMultiply,
  overtonePitchRandom,
  overtonePitchMultiply,
  overtonePitchModulo,
  spectrumInvert,
  spectrumExpand,
  spectrumShift,
  profileComb,
  profileShape,
  uniformPhaseProfile,

  gain,
  gainA,
  gainD,
  gainS,
  gainR,

  oscOctave,
  oscSemi,
  oscMilli,
  equalTemperament,
  pitchA4Hz,

  pitchEnvelopeAmount,
  pitchEnvelopeAmountNegative,
  pitchA,
  pitchD,
  pitchS,
  pitchR,

  lfoWavetableType,
  lfoTempoNumerator,
  lfoTempoDenominator,
  lfoFrequencyMultiplier,
  lfoPitchAmount,
  lfoPhaseReset,
  lfoLowpass,

  tableLowpass,
  tableLowpassKeyFollow,
  tableLowpassEnvelopeAmount,
  tableLowpassA,
  tableLowpassD,
  tableLowpassS,
  tableLowpassR,

  oscInitialPhase,
  oscPhaseReset,
  oscPhaseRandom,

  nUnison,
  unisonDetune,
  unisonPan,
  unisonPhase,
  unisonGainRandom,
  unisonDetuneRandom,
  unisonPanType,

  nVoice,
  voicePool,
  smoothness,

  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::DecibelScale<double> overtoneGain;
  static SomeDSP::LinearScale<double> overtoneWidth;
  static SomeDSP::LogScale<double> overtonePitch;
  static SomeDSP::LinearScale<double> overtonePhase;

  static SomeDSP::LogScale<double> tableBaseFrequency;
  static SomeDSP::LogScale<double> overtoneGainPower;
  static SomeDSP::LogScale<double> overtoneWidthMultiply;
  static SomeDSP::LinearScale<double> overtonePitchMultiply;
  static SomeDSP::LinearScale<double> overtonePitchModulo;
  static SomeDSP::LogScale<double> spectrumExpand;
  static SomeDSP::IntScale<double> spectrumShift;
  static SomeDSP::IntScale<double> profileComb;
  static SomeDSP::LogScale<double> profileShape;
  static SomeDSP::IntScale<double> seed;

  static SomeDSP::LogScale<double> gain;
  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::LogScale<double> envelopeS;
  static SomeDSP::LogScale<double> envelopeR;

  static SomeDSP::IntScale<double> oscOctave;
  static SomeDSP::IntScale<double> oscSemi;
  static SomeDSP::IntScale<double> oscMilli;
  static SomeDSP::IntScale<double> equalTemperament;
  static SomeDSP::IntScale<double> pitchA4Hz;

  static SomeDSP::LogScale<double> pitchAmount;

  static SomeDSP::LinearScale<double> lfoWavetable;
  static SomeDSP::IntScale<double> lfoWavetableType;
  static SomeDSP::IntScale<double> lfoTempoNumerator;
  static SomeDSP::IntScale<double> lfoTempoDenominator;
  static SomeDSP::LogScale<double> lfoFrequencyMultiplier;
  static SomeDSP::LogScale<double> lfoLowpass;

  static SomeDSP::LinearScale<double> tableLowpass;
  static SomeDSP::LinearScale<double> tableLowpassAmount;

  static SomeDSP::IntScale<double> nUnison;
  static SomeDSP::LogScale<double> unisonDetune;
  static SomeDSP::IntScale<double> unisonPanType;

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

    std::string gainLabel("gain");
    std::string widthLabel("width");
    std::string pitchLabel("pitch");
    std::string phaseLabel("phase");
    for (size_t idx = 0; idx < nOvertone; ++idx) {
      auto indexStr = std::to_string(idx + 1);
      value[ID::overtoneGain0 + idx] = std::make_unique<DecibelValue>(
        Scales::overtoneGain.invmap(1.0 / (idx + 1)), Scales::overtoneGain,
        (gainLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtoneWidth0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::overtoneWidth, (widthLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtonePitch0 + idx] = std::make_unique<LogValue>(
        Scales::overtonePitch.invmap(1.0), Scales::overtonePitch,
        (pitchLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtonePhase0 + idx] = std::make_unique<LinearValue>(
        1.0, Scales::overtonePhase, (phaseLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string lfoWavetableLabel("lfoWavetable");
    for (size_t idx = 0; idx < nLFOWavetable; ++idx) {
      auto indexStr = std::to_string(idx + 1);
      value[ID::lfoWavetable0 + idx] = std::make_unique<LinearValue>(
        Scales::lfoWavetable.invmap(sin(SomeDSP::twopi * idx / double(nLFOWavetable))),
        Scales::lfoWavetable, (lfoWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::tableBaseFrequency] = std::make_unique<LogValue>(
      Scales::tableBaseFrequency.invmap(10.0), Scales::tableBaseFrequency,
      "tableBaseFrequency", Info::kCanAutomate);
    value[ID::padSynthSeed]
      = std::make_unique<IntValue>(0, Scales::seed, "padSynthSeed", Info::kCanAutomate);
    value[ID::overtoneGainPower] = std::make_unique<LogValue>(
      0.5, Scales::overtoneGainPower, "overtoneGainPower", Info::kCanAutomate);
    value[ID::overtoneWidthMultiply] = std::make_unique<LogValue>(
      0.5, Scales::overtoneWidthMultiply, "overtoneWidthMultiply", Info::kCanAutomate);
    value[ID::overtonePitchRandom] = std::make_unique<IntValue>(
      0, Scales::boolScale, "overtonePitchRandom", Info::kCanAutomate);
    value[ID::overtonePitchMultiply] = std::make_unique<LinearValue>(
      Scales::overtonePitchMultiply.invmap(1.0), Scales::overtonePitchMultiply,
      "overtonePitchMultiply", Info::kCanAutomate);
    value[ID::overtonePitchModulo] = std::make_unique<LinearValue>(
      0.0, Scales::overtonePitchModulo, "overtonePitchModulo", Info::kCanAutomate);
    value[ID::spectrumInvert] = std::make_unique<IntValue>(
      0, Scales::boolScale, "spectrumInvert", Info::kCanAutomate);
    value[ID::spectrumExpand] = std::make_unique<LogValue>(
      Scales::spectrumExpand.invmap(1.0), Scales::spectrumExpand, "spectrumExpand",
      Info::kCanAutomate);
    value[ID::spectrumShift] = std::make_unique<IntValue>(
      spectrumSize, Scales::spectrumShift, "spectrumShift", Info::kCanAutomate);
    value[ID::profileComb] = std::make_unique<IntValue>(
      0, Scales::profileComb, "profileComb", Info::kCanAutomate);
    value[ID::profileShape] = std::make_unique<LogValue>(
      Scales::profileShape.invmap(1.0), Scales::profileShape, "profileShape",
      Info::kCanAutomate);
    value[ID::uniformPhaseProfile] = std::make_unique<IntValue>(
      0, Scales::boolScale, "uniformPhaseProfile", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, Scales::gain, "gain", Info::kCanAutomate);
    value[ID::gainA]
      = std::make_unique<LogValue>(0.0, Scales::envelopeA, "gainA", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<LogValue>(0.5, Scales::envelopeD, "gainD", Info::kCanAutomate);
    value[ID::gainS]
      = std::make_unique<LogValue>(0.5, Scales::envelopeS, "gainS", Info::kCanAutomate);
    value[ID::gainR]
      = std::make_unique<LogValue>(0.0, Scales::envelopeR, "gainR", Info::kCanAutomate);

    value[ID::oscOctave] = std::make_unique<IntValue>(
      12, Scales::oscOctave, "oscOctave", Info::kCanAutomate);
    value[ID::oscSemi]
      = std::make_unique<IntValue>(120, Scales::oscSemi, "oscSemi", Info::kCanAutomate);
    value[ID::oscMilli] = std::make_unique<IntValue>(
      1000, Scales::oscMilli, "oscMilli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<IntValue>(
      11, Scales::equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<IntValue>(
      340, Scales::pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);

    value[ID::pitchEnvelopeAmount] = std::make_unique<LogValue>(
      0.0, Scales::pitchAmount, "pitchEnvelopeAmount", Info::kCanAutomate);
    value[ID::pitchEnvelopeAmountNegative] = std::make_unique<IntValue>(
      false, Scales::boolScale, "pitchEnvelopeAmountNegative", Info::kCanAutomate);
    value[ID::pitchA]
      = std::make_unique<LogValue>(0.0, Scales::envelopeA, "pitchA", Info::kCanAutomate);
    value[ID::pitchD]
      = std::make_unique<LogValue>(0.5, Scales::envelopeD, "pitchD", Info::kCanAutomate);
    value[ID::pitchS]
      = std::make_unique<LogValue>(0.0, Scales::envelopeS, "pitchS", Info::kCanAutomate);
    value[ID::pitchR]
      = std::make_unique<LogValue>(0.35, Scales::envelopeR, "pitchR", Info::kCanAutomate);

    value[ID::lfoWavetableType] = std::make_unique<IntValue>(
      0, Scales::lfoWavetableType, "lfoWavetableType", Info::kCanAutomate);
    value[ID::lfoTempoNumerator] = std::make_unique<IntValue>(
      0, Scales::lfoTempoNumerator, "lfoTempoNumerator", Info::kCanAutomate);
    value[ID::lfoTempoDenominator] = std::make_unique<IntValue>(
      0, Scales::lfoTempoDenominator, "lfoTempoDenominator", Info::kCanAutomate);
    value[ID::lfoFrequencyMultiplier] = std::make_unique<LogValue>(
      Scales::lfoFrequencyMultiplier.invmap(1.0), Scales::lfoFrequencyMultiplier,
      "lfoFrequencyMultiplier", Info::kCanAutomate);
    value[ID::lfoPitchAmount] = std::make_unique<LogValue>(
      0.0, Scales::pitchAmount, "lfoPitchAmount", Info::kCanAutomate);
    value[ID::lfoPhaseReset] = std::make_unique<IntValue>(
      1, Scales::boolScale, "lfoPhaseReset", Info::kCanAutomate);
    value[ID::lfoLowpass] = std::make_unique<LogValue>(
      1.0, Scales::lfoLowpass, "lfoLowpass", Info::kCanAutomate);

    value[ID::tableLowpass] = std::make_unique<LinearValue>(
      1.0, Scales::tableLowpass, "tableLowpass", Info::kCanAutomate);
    value[ID::tableLowpassKeyFollow] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "tableLowpassKeyFollow", Info::kCanAutomate);
    value[ID::tableLowpassEnvelopeAmount] = std::make_unique<LinearValue>(
      0.0, Scales::tableLowpassAmount, "tableLowpassEnvelopeAmount", Info::kCanAutomate);
    value[ID::tableLowpassA] = std::make_unique<LogValue>(
      0.0, Scales::envelopeA, "tableLowpassA", Info::kCanAutomate);
    value[ID::tableLowpassD] = std::make_unique<LogValue>(
      0.5, Scales::envelopeD, "tableLowpassD", Info::kCanAutomate);
    value[ID::tableLowpassS] = std::make_unique<LogValue>(
      0.5, Scales::envelopeS, "tableLowpassS", Info::kCanAutomate);
    value[ID::tableLowpassR] = std::make_unique<LogValue>(
      0.5, Scales::envelopeR, "tableLowpassR", Info::kCanAutomate);

    value[ID::oscInitialPhase] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "oscInitialPhase", Info::kCanAutomate);
    value[ID::oscPhaseReset] = std::make_unique<IntValue>(
      true, Scales::boolScale, "oscPhaseReset", Info::kCanAutomate);
    value[ID::oscPhaseRandom] = std::make_unique<IntValue>(
      true, Scales::boolScale, "oscPhaseRandom", Info::kCanAutomate);

    value[ID::nUnison]
      = std::make_unique<IntValue>(0, Scales::nUnison, "nUnison", Info::kCanAutomate);
    value[ID::unisonDetune] = std::make_unique<LogValue>(
      0.2, Scales::unisonDetune, "unisonDetune", Info::kCanAutomate);
    value[ID::unisonPan] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "unisonPan", Info::kCanAutomate);
    value[ID::unisonPhase] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "unisonPhase", Info::kCanAutomate);
    value[ID::unisonGainRandom] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "unisonGainRandom", Info::kCanAutomate);
    value[ID::unisonDetuneRandom] = std::make_unique<IntValue>(
      1, Scales::boolScale, "unisonDetuneRandom", Info::kCanAutomate);
    value[ID::unisonPanType] = std::make_unique<IntValue>(
      0, Scales::unisonPanType, "unisonPanType", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<IntValue>(1, Scales::nVoice, "nVoice", Info::kCanAutomate);
    value[ID::voicePool]
      = std::make_unique<IntValue>(0, Scales::boolScale, "voicePool", Info::kCanAutomate);
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
