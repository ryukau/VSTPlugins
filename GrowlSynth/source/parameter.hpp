// (c) 2021-2023 Takamitsu Endo
//
// This file is part of GrowlSynth.
//
// GrowlSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrowlSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrowlSynth.  If not, see <https://www.gnu.org/licenses/>.

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
constexpr size_t nModDelay = 8;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  safetyHighpassEnable,
  safetyHighpassHz,
  overSampling,

  envelopeAttackSeconds,
  envelopeDecaySeconds,
  envelopeReleaseSeconds,
  envelopeTransitionSeconds,
  envelopeAM,

  breathGain,
  breathDecaySeconds,
  breathAMLowpassCutoffHz,
  breathFormantOctave,

  pulseGain,
  pulsePitchOctave,
  pulseBendOctave,
  frequencyModulationMix,
  pulseFormantOctave,

  formantSeed,

  combFollowNote,
  combLowpassHz,
  combHighpassHz,
  combAllpassCut,
  combAllpassMod,
  combAllpassQ,
  combEnergyLossThreshold,
  combDelayFrequencyHz,
  combDelayFrequencyRandom,
  combFeedbackGain,
  combFeedbackFollowEnvelope,
  combDelayTimeMod,
  combDelayTimeSlewRate,
  maxTimeSpreadSamples,

  tuningSemitone,
  tuningCent,
  tuningET,
  tuningA4Hz,
  pitchBend,
  pitchBendRange,
  noteSlideTimeSecond,
  slideAtNoteOn,
  slideAtNoteOff,

  ID_ENUM_LENGTH,
  // ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> safetyHighpassHz;
  static SomeDSP::DecibelScale<double> cutoffHz;
  static SomeDSP::LinearScale<double> pulseBendOctave;
  static SomeDSP::LinearScale<double> formantOctave;

  static SomeDSP::DecibelScale<double> envelopeSeconds;
  static SomeDSP::DecibelScale<double> envelopeAM;

  static SomeDSP::DecibelScale<double> frequencyRatio;
  static SomeDSP::LinearScale<double> cutoffMod;
  static SomeDSP::DecibelScale<double> filterQ;
  static SomeDSP::DecibelScale<double> energyLossThreshold;
  static SomeDSP::DecibelScale<double> frequencyHz;
  static SomeDSP::LinearScale<double> randomOctave;
  static SomeDSP::NegativeDecibelScale<double> feedbackGain;
  static SomeDSP::DecibelScale<double> slewRate;
  static SomeDSP::DecibelScale<double> maxTimeSpreadSamples;

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
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::safetyHighpassEnable] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "safetyHighpassEnable", Info::kCanAutomate);
    value[ID::safetyHighpassHz] = std::make_unique<DecibelValue>(
      Scales::safetyHighpassHz.invmap(4.0), Scales::safetyHighpassHz, "safetyHighpassHz",
      Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);

    value[ID::envelopeAttackSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(1.0), Scales::envelopeSeconds,
      "envelopeAttackSeconds", Info::kCanAutomate);
    value[ID::envelopeDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(2.0), Scales::envelopeSeconds,
      "envelopeDecaySeconds", Info::kCanAutomate);
    value[ID::envelopeReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.1), Scales::envelopeSeconds,
      "envelopeReleaseSeconds", Info::kCanAutomate);
    value[ID::envelopeTransitionSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(10.0), Scales::envelopeSeconds,
      "envelopeTransitionSeconds", Info::kCanAutomate);
    value[ID::envelopeAM] = std::make_unique<DecibelValue>(
      Scales::envelopeAM.invmap(0.01), Scales::envelopeAM, "envelopeAM",
      Info::kCanAutomate);

    value[ID::breathGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(0.1), Scales::gain, "breathGain", Info::kCanAutomate);
    value[ID::breathDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.03), Scales::envelopeSeconds, "breathDecaySeconds",
      Info::kCanAutomate);
    value[ID::breathAMLowpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(4000.0), Scales::cutoffHz, "breathAMLowpassCutoffHz",
      Info::kCanAutomate);
    value[ID::breathFormantOctave] = std::make_unique<LinearValue>(
      Scales::formantOctave.invmap(-2), Scales::formantOctave, "breathFormantOctave",
      Info::kCanAutomate);

    value[ID::pulseGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "pulseGain", Info::kCanAutomate);
    value[ID::pulsePitchOctave] = std::make_unique<LinearValue>(
      Scales::formantOctave.invmap(0), Scales::formantOctave, "pulsePitchOctave",
      Info::kCanAutomate);
    value[ID::pulseBendOctave] = std::make_unique<LinearValue>(
      Scales::pulseBendOctave.invmap(3.2), Scales::pulseBendOctave, "pulseBendOctave",
      Info::kCanAutomate);
    value[ID::frequencyModulationMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "frequencyModulationMix", Info::kCanAutomate);
    value[ID::pulseFormantOctave] = std::make_unique<LinearValue>(
      Scales::formantOctave.invmap(0), Scales::formantOctave, "pulseFormantOctave",
      Info::kCanAutomate);

    value[ID::formantSeed] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "formantSeed", Info::kCanAutomate);

    value[ID::combFollowNote] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.0), Scales::defaultScale, "combFollowNote",
      Info::kCanAutomate);
    value[ID::combLowpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(22000), Scales::cutoffHz, "combLowpassHz",
      Info::kCanAutomate);
    value[ID::combHighpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(20), Scales::cutoffHz, "combHighpassHz",
      Info::kCanAutomate);
    value[ID::combAllpassCut] = std::make_unique<DecibelValue>(
      Scales::frequencyRatio.invmap(0.2), Scales::frequencyRatio, "combAllpassCut",
      Info::kCanAutomate);
    value[ID::combAllpassMod] = std::make_unique<LinearValue>(
      Scales::cutoffMod.invmap(1.0), Scales::cutoffMod, "combAllpassMod",
      Info::kCanAutomate);
    value[ID::combAllpassQ] = std::make_unique<DecibelValue>(
      Scales::filterQ.invmap(0.2), Scales::filterQ, "combAllpassQ", Info::kCanAutomate);
    value[ID::combEnergyLossThreshold] = std::make_unique<DecibelValue>(
      Scales::energyLossThreshold.invmap(4.0), Scales::energyLossThreshold,
      "combEnergyLossThreshold", Info::kCanAutomate);
    value[ID::combDelayFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::frequencyHz.invmap(815.0), Scales::frequencyHz, "combDelayFrequencyHz",
      Info::kCanAutomate);
    value[ID::combDelayFrequencyRandom] = std::make_unique<LinearValue>(
      Scales::randomOctave.invmap(1.0), Scales::randomOctave, "combDelayFrequencyRandom",
      Info::kCanAutomate);
    value[ID::combFeedbackGain] = std::make_unique<NegativeDecibelValue>(
      Scales::feedbackGain.invmap(0.94), Scales::feedbackGain, "combFeedbackGain",
      Info::kCanAutomate);
    value[ID::combFeedbackFollowEnvelope] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "combFeedbackFollowEnvelope", Info::kCanAutomate);
    value[ID::combDelayTimeMod] = std::make_unique<LinearValue>(
      Scales::cutoffMod.invmap(1.0), Scales::cutoffMod, "combDelayTimeMod",
      Info::kCanAutomate);
    value[ID::combDelayTimeSlewRate] = std::make_unique<DecibelValue>(
      Scales::slewRate.invmap(0.25), Scales::slewRate, "combDelayTimeSlewRate",
      Info::kCanAutomate);
    value[ID::maxTimeSpreadSamples] = std::make_unique<DecibelValue>(
      Scales::maxTimeSpreadSamples.invmap(0.001), Scales::maxTimeSpreadSamples,
      "maxTimeSpreadSamples", Info::kCanAutomate);

    value[ID::tuningSemitone] = std::make_unique<UIntValue>(
      semitoneOffset - 24, Scales::semitone, "tuningSemitone", Info::kCanAutomate);
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
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(2.0), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);
    value[ID::slideAtNoteOn] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "slideAtNoteOn", Info::kCanAutomate);
    value[ID::slideAtNoteOff] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "slideAtNoteOff", Info::kCanAutomate);

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
