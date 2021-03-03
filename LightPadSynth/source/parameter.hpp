// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
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
  tableBufferSize,
  padSynthSeed,
  overtoneGainPower,
  overtoneWidthMultiply,
  overtonePitchMultiply,
  overtonePitchModulo,
  spectrumExpand,
  spectrumRotate,
  profileComb,
  profileShape,
  uniformPhaseProfile,

  gain,
  gainA,
  gainD,
  gainS,
  gainR,
  gainCurve,

  filterCutoff,
  filterResonance,
  filterA,
  filterD,
  filterS,
  filterR,
  filterAmount,
  filterKeyFollow,

  delayMix,
  delayDetuneSemi,
  delayDetuneMilli,
  delayFeedback,
  delayAttack,

  oscOctave,
  oscSemi,
  oscMilli,
  equalTemperament,
  pitchA4Hz,

  lfoWavetableType,
  lfoTempoNumerator,
  lfoTempoDenominator,
  lfoFrequencyMultiplier,
  lfoDelayAmount,
  lfoLowpass,

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
  smoothness,
  seed,

  pitchBend,

  refreshLFO,
  refreshTable,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , overtoneGain(-60.0, 0.0, true)
    , overtoneWidth(0.0, 100.0)
    , overtonePitch(0.0625, 16.0, 0.5, 1.0)
    , overtonePhase(0.0, SomeDSP::twopi)
    , tableBaseFrequency(0.1, 100.0, 0.5, 2.5)
    , tableBufferSize(11)
    , overtoneGainPower(0.2, 10.0, 0.5, 1.0)
    , overtoneWidthMultiply(0.05, 12.0, 0.5, 1.0)
    , overtonePitchMultiply(0.0001, 2.0, 0.75, 1.0)
    , overtonePitchModulo(0.0, 136.0)
    , spectrumExpand(0.03125, 32.0, 0.5, 1.0)
    , profileComb(255)
    , profileShape(0.0, 4.0, 0.5, 1.0)
    , seed(16777215)
    , gain(0.0, 2.0, 0.5, 0.5)
    , envelopeA(0.0001, 16.0, 0.5, 2.0)
    , envelopeD(0.0001, 16.0, 0.5, 4.0)
    , envelopeS(0.0001, 0.9995, 0.5, 0.3)
    , envelopeR(0.001, 16.0, 0.5, 2.0)
    , filterCutoff(0.0, 22000.0, 0.5, 400.0)
    , filterResonance(0.0, 0.995)
    , delayFeedback(-1.0, 1.0)
    , delayDetuneSemi(240)
    , oscOctave(16)
    , oscSemi(168)
    , oscMilli(2000)
    , equalTemperament(119)
    , pitchA4Hz(900)
    , lfoWavetable(-1.0, 1.0)
    , lfoWavetableType(2)
    , lfoTempoNumerator(255)
    , lfoTempoDenominator(255)
    , lfoFrequencyMultiplier(0.0, 16.0, 0.5, 1.0)
    , lfoDelayAmount(0.0, 1.0, 0.5, 0.2)
    , nUnison(15)
    , unisonDetune(0.0, 1.0, 0.5, 0.05)
    , unisonPanType(9)
    , nVoice(7)
    , smoothness(0.0, 0.5, 0.1, 0.04)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::DecibelScale<double> overtoneGain;
  SomeDSP::LinearScale<double> overtoneWidth;
  SomeDSP::LogScale<double> overtonePitch;
  SomeDSP::LinearScale<double> overtonePhase;

  SomeDSP::LogScale<double> tableBaseFrequency;
  SomeDSP::UIntScale<double> tableBufferSize;
  SomeDSP::LogScale<double> overtoneGainPower;
  SomeDSP::LogScale<double> overtoneWidthMultiply;
  SomeDSP::LogScale<double> overtonePitchMultiply;
  SomeDSP::LinearScale<double> overtonePitchModulo;
  SomeDSP::LogScale<double> spectrumExpand;
  SomeDSP::UIntScale<double> profileComb;
  SomeDSP::LogScale<double> profileShape;
  SomeDSP::UIntScale<double> seed;

  SomeDSP::LogScale<double> gain;
  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::LogScale<double> envelopeS;
  SomeDSP::LogScale<double> envelopeR;

  SomeDSP::LogScale<double> filterCutoff;
  SomeDSP::LinearScale<double> filterResonance;

  SomeDSP::UIntScale<double> delayDetuneSemi;
  SomeDSP::LinearScale<double> delayFeedback;

  SomeDSP::UIntScale<double> oscOctave;
  SomeDSP::UIntScale<double> oscSemi;
  SomeDSP::UIntScale<double> oscMilli;
  SomeDSP::UIntScale<double> equalTemperament;
  SomeDSP::UIntScale<double> pitchA4Hz;

  SomeDSP::LinearScale<double> lfoWavetable;
  SomeDSP::UIntScale<double> lfoWavetableType;
  SomeDSP::UIntScale<double> lfoTempoNumerator;
  SomeDSP::UIntScale<double> lfoTempoDenominator;
  SomeDSP::LogScale<double> lfoFrequencyMultiplier;
  SomeDSP::LogScale<double> lfoDelayAmount;

  SomeDSP::UIntScale<double> nUnison;
  SomeDSP::LogScale<double> unisonDetune;
  SomeDSP::UIntScale<double> unisonPanType;

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

    std::string gainLabel("gain");
    std::string widthLabel("width");
    std::string pitchLabel("pitch");
    std::string phaseLabel("phase");
    for (size_t idx = 0; idx < nOvertone; ++idx) {
      auto indexStr = std::to_string(idx + 1);
      value[ID::overtoneGain0 + idx] = std::make_unique<DecibelValue>(
        scale.overtoneGain.invmap(1.0 / (idx + 1)), scale.overtoneGain,
        (gainLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtoneWidth0 + idx] = std::make_unique<LinearValue>(
        0.5, scale.overtoneWidth, (widthLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtonePitch0 + idx] = std::make_unique<LogValue>(
        scale.overtonePitch.invmap(1.0), scale.overtonePitch,
        (pitchLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::overtonePhase0 + idx] = std::make_unique<LinearValue>(
        1.0, scale.overtonePhase, (phaseLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string lfoWavetableLabel("lfoWavetable");
    for (size_t idx = 0; idx < nLFOWavetable; ++idx) {
      auto indexStr = std::to_string(idx + 1);
      value[ID::lfoWavetable0 + idx] = std::make_unique<LinearValue>(
        scale.lfoWavetable.invmap(sin(SomeDSP::twopi * idx / double(nLFOWavetable))),
        scale.lfoWavetable, (lfoWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::tableBaseFrequency] = std::make_unique<LogValue>(
      scale.tableBaseFrequency.invmap(10.0), scale.tableBaseFrequency,
      "tableBaseFrequency", Info::kCanAutomate);
    value[ID::tableBufferSize] = std::make_unique<UIntValue>(
      8, scale.tableBufferSize, "tableBufferSize", Info::kCanAutomate);
    value[ID::padSynthSeed]
      = std::make_unique<UIntValue>(0, scale.seed, "padSynthSeed", Info::kCanAutomate);
    value[ID::overtoneGainPower] = std::make_unique<LogValue>(
      0.5, scale.overtoneGainPower, "overtoneGainPower", Info::kCanAutomate);
    value[ID::overtoneWidthMultiply] = std::make_unique<LogValue>(
      0.5, scale.overtoneWidthMultiply, "overtoneWidthMultiply", Info::kCanAutomate);
    value[ID::overtonePitchMultiply] = std::make_unique<LogValue>(
      scale.overtonePitchMultiply.invmap(1.0), scale.overtonePitchMultiply,
      "overtonePitchMultiply", Info::kCanAutomate);
    value[ID::overtonePitchModulo] = std::make_unique<LinearValue>(
      0.0, scale.overtonePitchModulo, "overtonePitchModulo", Info::kCanAutomate);
    value[ID::spectrumExpand] = std::make_unique<LogValue>(
      scale.spectrumExpand.invmap(1.0), scale.spectrumExpand, "spectrumExpand",
      Info::kCanAutomate);
    value[ID::spectrumRotate] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "spectrumRotate", Info::kCanAutomate);
    value[ID::profileComb] = std::make_unique<UIntValue>(
      0, scale.profileComb, "profileComb", Info::kCanAutomate);
    value[ID::profileShape] = std::make_unique<LogValue>(
      scale.profileShape.invmap(1.0), scale.profileShape, "profileShape",
      Info::kCanAutomate);
    value[ID::uniformPhaseProfile] = std::make_unique<UIntValue>(
      0, scale.boolScale, "uniformPhaseProfile", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::gainA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "gainA", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "gainD", Info::kCanAutomate);
    value[ID::gainS]
      = std::make_unique<LogValue>(0.5, scale.envelopeS, "gainS", Info::kCanAutomate);
    value[ID::gainR]
      = std::make_unique<LogValue>(0.0, scale.envelopeR, "gainR", Info::kCanAutomate);
    value[ID::gainCurve] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "gainCurve", Info::kCanAutomate);

    value[ID::filterCutoff] = std::make_unique<LogValue>(
      1.0, scale.filterCutoff, "filterCutoff", Info::kCanAutomate);
    value[ID::filterResonance] = std::make_unique<LinearValue>(
      0.0, scale.filterResonance, "filterResonance", Info::kCanAutomate);
    value[ID::filterA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "filterA", Info::kCanAutomate);
    value[ID::filterD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "filterD", Info::kCanAutomate);
    value[ID::filterS]
      = std::make_unique<LogValue>(0.5, scale.envelopeS, "filterS", Info::kCanAutomate);
    value[ID::filterR]
      = std::make_unique<LogValue>(1.0, scale.envelopeR, "filterR", Info::kCanAutomate);
    value[ID::filterAmount] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "filterAmount", Info::kCanAutomate);
    value[ID::filterKeyFollow] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "filterKeyFollow", Info::kCanAutomate);

    value[ID::delayMix] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "delayMix", Info::kCanAutomate);
    value[ID::delayDetuneSemi] = std::make_unique<UIntValue>(
      120, scale.delayDetuneSemi, "delayDetuneSemi", Info::kCanAutomate);
    value[ID::delayDetuneMilli] = std::make_unique<UIntValue>(
      1000, scale.oscMilli, "delayDetuneMilli", Info::kCanAutomate);
    value[ID::delayFeedback] = std::make_unique<LinearValue>(
      0.5, scale.delayFeedback, "delayFeedback", Info::kCanAutomate);
    value[ID::delayAttack] = std::make_unique<LogValue>(
      0.0, scale.envelopeA, "delayAttack", Info::kCanAutomate);

    value[ID::oscOctave]
      = std::make_unique<UIntValue>(12, scale.oscOctave, "oscOctave", Info::kCanAutomate);
    value[ID::oscSemi]
      = std::make_unique<UIntValue>(120, scale.oscSemi, "oscSemi", Info::kCanAutomate);
    value[ID::oscMilli]
      = std::make_unique<UIntValue>(1000, scale.oscMilli, "oscMilli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<UIntValue>(
      11, scale.equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<UIntValue>(
      340, scale.pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);

    value[ID::lfoWavetableType] = std::make_unique<UIntValue>(
      2, scale.lfoWavetableType, "lfoWavetableType", Info::kCanAutomate);
    value[ID::lfoTempoNumerator] = std::make_unique<UIntValue>(
      0, scale.lfoTempoNumerator, "lfoTempoNumerator", Info::kCanAutomate);
    value[ID::lfoTempoDenominator] = std::make_unique<UIntValue>(
      0, scale.lfoTempoDenominator, "lfoTempoDenominator", Info::kCanAutomate);
    value[ID::lfoFrequencyMultiplier] = std::make_unique<LogValue>(
      scale.lfoFrequencyMultiplier.invmap(1.0), scale.lfoFrequencyMultiplier,
      "lfoFrequencyMultiplier", Info::kCanAutomate);
    value[ID::lfoDelayAmount] = std::make_unique<LogValue>(
      0.5, scale.lfoDelayAmount, "lfoDelayAmount", Info::kCanAutomate);
    value[ID::lfoLowpass] = std::make_unique<LogValue>(
      1.0, scale.filterCutoff, "lfoLowpass", Info::kCanAutomate);

    value[ID::oscInitialPhase] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "oscInitialPhase", Info::kCanAutomate);
    value[ID::oscPhaseReset] = std::make_unique<UIntValue>(
      true, scale.boolScale, "oscPhaseReset", Info::kCanAutomate);
    value[ID::oscPhaseRandom] = std::make_unique<UIntValue>(
      true, scale.boolScale, "oscPhaseRandom", Info::kCanAutomate);

    value[ID::nUnison]
      = std::make_unique<UIntValue>(0, scale.nUnison, "nUnison", Info::kCanAutomate);
    value[ID::unisonDetune] = std::make_unique<LogValue>(
      0.2, scale.unisonDetune, "unisonDetune", Info::kCanAutomate);
    value[ID::unisonPan] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "unisonPan", Info::kCanAutomate);
    value[ID::unisonPhase] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "unisonPhase", Info::kCanAutomate);
    value[ID::unisonGainRandom] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "unisonGainRandom", Info::kCanAutomate);
    value[ID::unisonDetuneRandom] = std::make_unique<UIntValue>(
      1, scale.boolScale, "unisonDetuneRandom", Info::kCanAutomate);
    value[ID::unisonPanType] = std::make_unique<UIntValue>(
      0, scale.unisonPanType, "unisonPanType", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<UIntValue>(1, scale.nVoice, "nVoice", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);
    value[ID::seed]
      = std::make_unique<UIntValue>(0, scale.seed, "seed", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pitchBend", Info::kCanAutomate);

    value[ID::refreshLFO]
      = std::make_unique<UIntValue>(0, scale.boolScale, "refreshLFO", Info::kCanAutomate);
    value[ID::refreshTable] = std::make_unique<UIntValue>(
      0, scale.boolScale, "refreshTable", Info::kCanAutomate);

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
