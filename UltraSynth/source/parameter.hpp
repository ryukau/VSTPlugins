// (c) 2021-2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr int octaveOffset = 8;
constexpr int semitoneOffset = 96;
constexpr int a4HzOffset = 100;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  rectificationMix,
  saturationMix,
  gainAttackSecond,
  gainDecaySecond,
  gainSustainAmplitude,
  gainReleaseSecond,

  osc1Octave,
  osc2Octave,
  osc1FineTuneCent,
  osc2FineTuneCent,
  osc1WaveShape,
  osc2WaveShape,
  osc1SawPulseMix,
  osc2SawPulseMix,
  phaseModFromLowpassToOsc1,
  pmPhase1ToPhase2,
  pmPhase2ToPhase1,
  pmOsc1ToPhase2,
  pmOsc2ToPhase1,
  oscMix,

  lowpassCutoffAttackSecond,
  lowpassCutoffDecaySecond,
  lowpassCutoffHz,
  lowpassQ,
  lowpassCutoffEnvelopeAmount,
  lowpassKeyFollow,

  lfoTempoSync,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoRetrigger,
  lfoWaveShape,
  lfoToPitch,
  lfoToOscMix,
  lfoToCutoff,
  lfoToPreSaturation,
  lfoToOsc1WaveShape,
  lfoToOsc2WaveShape,

  tuningSemitone,
  tuningCent,
  tuningET,
  tuningA4Hz,
  pitchBend,
  pitchBendRange,

  resetPhaseAtNoteOn,
  noteSlideTimeSecond,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> outputGain;
  static SomeDSP::DecibelScale<double> envelopeAttackSecond;
  static SomeDSP::DecibelScale<double> envelopeSecond;
  static SomeDSP::DecibelScale<double> gainSustainAmplitude;

  static SomeDSP::UIntScale<double> octave;
  static SomeDSP::LinearScale<double> fineTuneCent;
  static SomeDSP::LinearScale<double> waveShape;
  static SomeDSP::DecibelScale<double> phaseMod;

  static SomeDSP::DecibelScale<double> lowpassCutoffHz;
  static SomeDSP::DecibelScale<double> lowpassQ;
  static SomeDSP::DecibelScale<double> lowpassCutoffEnvelopeAmount;

  static SomeDSP::UIntScale<double> lfoTempoUpper;
  static SomeDSP::UIntScale<double> lfoTempoLower;
  static SomeDSP::DecibelScale<double> lfoRate;
  static SomeDSP::DecibelScale<double> lfoWaveShape;
  static SomeDSP::DecibelScale<double> lfoToPitch;
  static SomeDSP::DecibelScale<double> lfoToCutoff;

  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::UIntScale<double> equalTemperament;
  static SomeDSP::UIntScale<double> a4Hz;
  static SomeDSP::LinearScale<double> pitchBendRange;

  static SomeDSP::DecibelScale<double> noteSlideTimeSecond;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using NegativeDecibelValue = DoubleValue<SomeDSP::NegativeDecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::outputGain.invmap(1.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::rectificationMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "rectificationMix", Info::kCanAutomate);
    value[ID::saturationMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "saturationMix", Info::kCanAutomate);
    value[ID::gainAttackSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.002), Scales::envelopeSecond, "gainAttackSecond",
      Info::kCanAutomate);
    value[ID::gainDecaySecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.1), Scales::envelopeSecond, "gainDecaySecond",
      Info::kCanAutomate);
    value[ID::gainSustainAmplitude] = std::make_unique<DecibelValue>(
      Scales::gainSustainAmplitude.invmapDB(-6.0), Scales::gainSustainAmplitude,
      "gainSustainAmplitude", Info::kCanAutomate);
    value[ID::gainReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.01), Scales::envelopeSecond, "gainReleaseSecond",
      Info::kCanAutomate);

    value[ID::osc1Octave] = std::make_unique<UIntValue>(
      octaveOffset, Scales::octave, "osc1Octave", Info::kCanAutomate);
    value[ID::osc2Octave] = std::make_unique<UIntValue>(
      octaveOffset, Scales::octave, "osc2Octave", Info::kCanAutomate);
    value[ID::osc1FineTuneCent] = std::make_unique<LinearValue>(
      Scales::fineTuneCent.invmap(0.0), Scales::fineTuneCent, "osc1FineTuneCent",
      Info::kCanAutomate);
    value[ID::osc2FineTuneCent] = std::make_unique<LinearValue>(
      Scales::fineTuneCent.invmap(0.0), Scales::fineTuneCent, "osc2FineTuneCent",
      Info::kCanAutomate);
    value[ID::osc1WaveShape] = std::make_unique<LinearValue>(
      0.5, Scales::waveShape, "osc1WaveShape", Info::kCanAutomate);
    value[ID::osc2WaveShape] = std::make_unique<LinearValue>(
      0.5, Scales::waveShape, "osc2WaveShape", Info::kCanAutomate);
    value[ID::osc1SawPulseMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "osc1SawPulseMix", Info::kCanAutomate);
    value[ID::osc2SawPulseMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "osc2SawPulseMix", Info::kCanAutomate);
    value[ID::phaseModFromLowpassToOsc1] = std::make_unique<DecibelValue>(
      0.0, Scales::phaseMod, "phaseModFromLowpassToOsc1", Info::kCanAutomate);
    value[ID::pmPhase1ToPhase2] = std::make_unique<DecibelValue>(
      0.0, Scales::phaseMod, "pmPhase1ToPhase2", Info::kCanAutomate);
    value[ID::pmPhase2ToPhase1] = std::make_unique<DecibelValue>(
      0.0, Scales::phaseMod, "pmPhase2ToPhase1", Info::kCanAutomate);
    value[ID::pmOsc1ToPhase2] = std::make_unique<DecibelValue>(
      0.0, Scales::phaseMod, "pmOsc1ToPhase2", Info::kCanAutomate);
    value[ID::pmOsc2ToPhase1] = std::make_unique<DecibelValue>(
      0.0, Scales::phaseMod, "pmOsc2ToPhase1", Info::kCanAutomate);
    value[ID::oscMix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "oscMix", Info::kCanAutomate);

    value[ID::lowpassCutoffAttackSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeAttackSecond.invmap(0.0001), Scales::envelopeAttackSecond,
      "lowpassCutoffAttackSecond", Info::kCanAutomate);
    value[ID::lowpassCutoffDecaySecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.1), Scales::envelopeSecond,
      "lowpassCutoffDecaySecond", Info::kCanAutomate);
    value[ID::lowpassCutoffHz] = std::make_unique<DecibelValue>(
      1.0, Scales::lowpassCutoffHz, "lowpassCutoffHz", Info::kCanAutomate);
    value[ID::lowpassQ] = std::make_unique<DecibelValue>(
      Scales::lowpassQ.invmap(SomeDSP::halfSqrt2), Scales::lowpassQ, "lowpassQ",
      Info::kCanAutomate);
    value[ID::lowpassCutoffEnvelopeAmount] = std::make_unique<DecibelValue>(
      Scales::lowpassCutoffEnvelopeAmount.invmap(1.0),
      Scales::lowpassCutoffEnvelopeAmount, "lowpassCutoffEnvelopeAmount",
      Info::kCanAutomate);
    value[ID::lowpassKeyFollow] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(1.0), Scales::defaultScale, "lowpassKeyFollow",
      Info::kCanAutomate);

    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoTempoUpper] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoUpper, "lfoTempoUpper", Info::kCanAutomate);
    value[ID::lfoTempoLower] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoLower, "lfoTempoLower", Info::kCanAutomate);
    value[ID::lfoRate] = std::make_unique<DecibelValue>(
      Scales::lfoRate.invmap(1.0), Scales::lfoRate, "lfoRate", Info::kCanAutomate);
    value[ID::lfoRetrigger] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "lfoRetrigger", Info::kCanAutomate);
    value[ID::lfoWaveShape] = std::make_unique<DecibelValue>(
      Scales::lfoWaveShape.invmapDB(0.0), Scales::lfoWaveShape, "lfoWaveShape",
      Info::kCanAutomate);

    value[ID::lfoToPitch] = std::make_unique<DecibelValue>(
      0.0, Scales::lfoToPitch, "lfoToPitch", Info::kCanAutomate);
    value[ID::lfoToOscMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToOscMix", Info::kCanAutomate);
    value[ID::lfoToCutoff] = std::make_unique<DecibelValue>(
      0.0, Scales::lfoToCutoff, "lfoToCutoff", Info::kCanAutomate);
    value[ID::lfoToPreSaturation] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToPreSaturation", Info::kCanAutomate);
    value[ID::lfoToOsc1WaveShape] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToOsc1WaveShape", Info::kCanAutomate);
    value[ID::lfoToOsc2WaveShape] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToOsc2WaveShape", Info::kCanAutomate);

    value[ID::tuningSemitone] = std::make_unique<UIntValue>(
      semitoneOffset, Scales::semitone, "tuningSemitone", Info::kCanAutomate);
    value[ID::tuningCent] = std::make_unique<LinearValue>(
      Scales::cent.invmap(0.0), Scales::cent, "tuningCent", Info::kCanAutomate);
    value[ID::tuningET] = std::make_unique<UIntValue>(
      11, Scales::equalTemperament, "tuningET", Info::kCanAutomate);
    value[ID::tuningA4Hz] = std::make_unique<UIntValue>(
      440 - a4HzOffset, Scales::a4Hz, "tuningA4Hz", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::pitchBendRange.invmap(2.0), Scales::pitchBendRange, "pitchBendRange",
      Info::kCanAutomate);

    value[ID::resetPhaseAtNoteOn] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "resetPhaseAtNoteOn", Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.0), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);

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
