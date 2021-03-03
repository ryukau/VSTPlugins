// (c) 2019-2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  osc1Semi,
  osc1Cent,
  osc1PitchDrift,
  osc1Slope,
  osc1PulseWidth,
  osc1Feedback,

  osc2Semi,
  osc2Cent,
  osc2Overtone,
  osc2Slope,
  osc2PulseWidth,
  pmOsc2ToOsc1,

  gain,
  gainA,
  gainD,
  gainS,
  gainR,
  gainCurve,
  gainEnvRetrigger,

  filterOrder,
  filterCutoff,
  filterFeedback,
  filterSaturation,
  filterEnvToCutoff,
  filterKeyToCutoff,
  oscMixToFilterCutoff,
  filterA,
  filterD,
  filterS,
  filterR,
  filterCurve,
  filterEnvToOctave,
  filterEnvRetrigger,

  modEnv1Attack,
  modEnv1Curve,
  modEnv1ToPhaseMod,
  modEnv1Retrigger,

  modEnv2Attack,
  modEnv2Curve,
  modEnv2ToFeedback,
  modEnv2ToLFOFrequency,
  modEnv2ToOsc2Slope,
  modEnv2ToShifter1,
  modEnv2Retrigger,

  shifter1Semi,
  shifter1Cent,
  shifter1Gain,
  shifter2Semi,
  shifter2Cent,
  shifter2Gain,

  lfoType,
  lfoTempoSync,
  lfoFrequency,
  lfoShape,
  lfoToPitch,
  lfoToSlope,
  lfoToPulseWidth,
  lfoToCutoff,

  oscMix,
  octave,

  pitchSlideType,
  pitchSlide,
  pitchSlideOffset,

  smoothness,

  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , semi(-24.0, 24.0)
    , cent(-100.0, 100.0)
    , overtone(1.0, 32.0)
    , pitchDrift(0.0, 3.0)
    , oscSlope(1.0, 32.0, 0.5, 8.0)
    , oscFeedback(0.0, 1.0, 0.5, 0.1)
    , phaseModulation(0.0, 1.0, 0.5, 0.2)
    , envelopeA(0.0, 16.0, 0.5, 1.0)
    , envelopeD(0.01, 16.0, 0.5, 2.0)
    , envelopeS(0.0, 1.0, 0.5, 0.3)
    , envelopeR(0.0012, 16.0, 0.5, 2.0)
    , filterCutoff(20.0, 20000.0, 0.5, 1000.0)
    , filterResonance(0.0, 1.0, 0.5, 0.3)
    , filterSaturation(0.01, 8.0, 0.3, 0.5)
    , filterOrder(7)
    , filterEnvToCutoff(-1.0, 1.0, 0.3)
    , filterKeyToCutoff(0.0, 2.0)
    , oscMixToFilterCutoff(0.0, 0.5)
    , filterEnvToOctave(-9.0, 9.0)
    , modEnvelopeA(0.0, 4.0, 0.5, 0.5)
    , modEnvelopeCurve(1.0, 96.0, 0.5, 8.0)
    , modEnvToShifter(0.0, 4000.0, 0.5, 500.0)
    , shifterSemi(0.0, 24.0)
    , shifterCent(0.0, 100.0)
    , lfoType(3)
    , lfoFrequency(0.1, 20.0, 0.5, 4.0)
    , lfoSlope(0.0, 32.0, 0.5, 8.0)
    , octave(-4.0, 4.0)
    , pitchSlideType(2)
    , pitchSlide(0.001, 0.5, 0.5, 0.15)
    , pitchSlideOffset(0.0, 2.0)
    , smoothness(0.001, 0.5, 0.5, 0.2)
    , gain(0.0, 4.0, 0.5, 0.75)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LinearScale<double> semi;
  SomeDSP::LinearScale<double> cent;
  SomeDSP::LinearScale<double> overtone;
  SomeDSP::LinearScale<double> pitchDrift;
  SomeDSP::LogScale<double> oscSlope;
  SomeDSP::LogScale<double> oscFeedback;
  SomeDSP::LogScale<double> phaseModulation;

  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::LogScale<double> envelopeS;
  SomeDSP::LogScale<double> envelopeR;

  SomeDSP::LogScale<double> filterCutoff;
  SomeDSP::LogScale<double> filterResonance;
  SomeDSP::LogScale<double> filterSaturation;
  SomeDSP::UIntScale<double> filterOrder;
  SomeDSP::SPolyScale<double> filterEnvToCutoff;
  SomeDSP::LinearScale<double> filterKeyToCutoff;
  SomeDSP::LinearScale<double> oscMixToFilterCutoff;
  SomeDSP::LinearScale<double> filterEnvToOctave;

  SomeDSP::LogScale<double> modEnvelopeA;
  SomeDSP::LogScale<double> modEnvelopeCurve;
  SomeDSP::LogScale<double> modEnvToShifter;

  SomeDSP::LinearScale<double> shifterSemi;
  SomeDSP::LinearScale<double> shifterCent;

  SomeDSP::UIntScale<double> lfoType;
  SomeDSP::LogScale<double> lfoFrequency;
  SomeDSP::LogScale<double> lfoSlope;

  SomeDSP::LinearScale<double> octave;

  SomeDSP::UIntScale<double> pitchSlideType;
  SomeDSP::LogScale<double> pitchSlide;
  SomeDSP::LinearScale<double> pitchSlideOffset;

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
    using SPolyValue = FloatValue<SomeDSP::SPolyScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::osc1Semi]
      = std::make_unique<LinearValue>(0.5, scale.semi, "osc1Semi", Info::kCanAutomate);
    value[ID::osc1Cent]
      = std::make_unique<LinearValue>(0.5, scale.cent, "osc1Cent", Info::kCanAutomate);
    value[ID::osc1PitchDrift] = std::make_unique<LinearValue>(
      0.0, scale.pitchDrift, "osc1PitchDrift", Info::kCanAutomate);
    value[ID::osc1Slope]
      = std::make_unique<LogValue>(0.5, scale.oscSlope, "osc1Slope", Info::kCanAutomate);
    value[ID::osc1PulseWidth] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "osc1PulseWidth", Info::kCanAutomate);
    value[ID::osc1Feedback] = std::make_unique<LogValue>(
      0.0, scale.oscFeedback, "osc1Feedback", Info::kCanAutomate);

    value[ID::osc2Semi]
      = std::make_unique<LinearValue>(0.5, scale.semi, "osc2Semi", Info::kCanAutomate);
    value[ID::osc2Cent]
      = std::make_unique<LinearValue>(0.5, scale.cent, "osc2Cent", Info::kCanAutomate);
    value[ID::osc2Overtone] = std::make_unique<LinearValue>(
      0, scale.overtone, "osc2Overtone", Info::kCanAutomate);
    value[ID::osc2Slope]
      = std::make_unique<LogValue>(0.5, scale.oscSlope, "osc2Slope", Info::kCanAutomate);
    value[ID::osc2PulseWidth] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "osc2PulseWidth", Info::kCanAutomate);
    value[ID::pmOsc2ToOsc1] = std::make_unique<LogValue>(
      0.0, scale.phaseModulation, "pmOsc2ToOsc1", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::gainA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "gainA", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "gainD", Info::kCanAutomate);
    value[ID::gainS]
      = std::make_unique<LogValue>(0.8, scale.envelopeS, "gainS", Info::kCanAutomate);
    value[ID::gainR]
      = std::make_unique<LogValue>(0.1, scale.envelopeR, "gainR", Info::kCanAutomate);
    value[ID::gainCurve] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "gainCurve", Info::kCanAutomate);
    value[ID::gainEnvRetrigger] = std::make_unique<UIntValue>(
      true, scale.boolScale, "gainEnvRetrigger", Info::kCanAutomate);

    value[ID::filterOrder] = std::make_unique<UIntValue>(
      7, scale.filterOrder, "filterOrder", Info::kCanAutomate);
    value[ID::filterCutoff] = std::make_unique<LogValue>(
      1.0, scale.filterCutoff, "filterCutoff", Info::kCanAutomate);
    value[ID::filterFeedback] = std::make_unique<LogValue>(
      0.0, scale.filterResonance, "filterFeedback", Info::kCanAutomate);
    value[ID::filterSaturation] = std::make_unique<LogValue>(
      0.3, scale.filterSaturation, "filterSaturation", Info::kCanAutomate);
    value[ID::filterEnvToCutoff] = std::make_unique<SPolyValue>(
      0.5, scale.filterEnvToCutoff, "filterEnvToCutoff", Info::kCanAutomate);
    value[ID::filterKeyToCutoff] = std::make_unique<LinearValue>(
      0.5, scale.filterKeyToCutoff, "filterKeyToCutoff", Info::kCanAutomate);
    value[ID::oscMixToFilterCutoff] = std::make_unique<LinearValue>(
      0.0, scale.oscMixToFilterCutoff, "oscMixToFilterCutoff", Info::kCanAutomate);

    value[ID::filterA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "filterA", Info::kCanAutomate);
    value[ID::filterD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "filterD", Info::kCanAutomate);
    value[ID::filterS]
      = std::make_unique<LogValue>(0.0, scale.envelopeS, "filterS", Info::kCanAutomate);
    value[ID::filterR]
      = std::make_unique<LogValue>(0.5, scale.envelopeR, "filterR", Info::kCanAutomate);
    value[ID::filterCurve] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "filterCurve", Info::kCanAutomate);
    value[ID::filterEnvToOctave] = std::make_unique<LinearValue>(
      0.5, scale.filterEnvToOctave, "filterEnvToOctave", Info::kCanAutomate);
    value[ID::filterEnvRetrigger] = std::make_unique<UIntValue>(
      true, scale.boolScale, "filterEnvRetrigger", Info::kCanAutomate);

    value[ID::modEnv1Attack] = std::make_unique<LogValue>(
      0.0, scale.modEnvelopeA, "modEnv1Attack", Info::kCanAutomate);
    value[ID::modEnv1Curve] = std::make_unique<LogValue>(
      0.5, scale.modEnvelopeCurve, "modEnv1Curve", Info::kCanAutomate);
    value[ID::modEnv1ToPhaseMod] = std::make_unique<LogValue>(
      0.0, scale.phaseModulation, "modEnv1ToPhaseMod", Info::kCanAutomate);
    value[ID::modEnv1Retrigger] = std::make_unique<UIntValue>(
      true, scale.boolScale, "modEnv1Retrigger", Info::kCanAutomate);

    value[ID::modEnv2Attack] = std::make_unique<LogValue>(
      0.0, scale.modEnvelopeA, "modEnv2Attack", Info::kCanAutomate);
    value[ID::modEnv2Curve] = std::make_unique<LogValue>(
      0.5, scale.modEnvelopeCurve, "modEnv2Curve", Info::kCanAutomate);
    value[ID::modEnv2ToFeedback] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "modEnv2ToFeedback", Info::kCanAutomate);
    value[ID::modEnv2ToLFOFrequency] = std::make_unique<LogValue>(
      0.0, scale.lfoFrequency, "modEnv2ToLFOFrequency", Info::kCanAutomate);
    value[ID::modEnv2ToOsc2Slope] = std::make_unique<LogValue>(
      0.0, scale.oscSlope, "modEnv2ToOsc2Slope", Info::kCanAutomate);
    value[ID::modEnv2ToShifter1] = std::make_unique<LogValue>(
      0.0, scale.modEnvToShifter, "modEnv2ToShifter1", Info::kCanAutomate);
    value[ID::modEnv2Retrigger] = std::make_unique<UIntValue>(
      true, scale.boolScale, "modEnv2Retrigger", Info::kCanAutomate);

    value[ID::shifter1Semi] = std::make_unique<LinearValue>(
      7.001 / scale.shifterSemi.getMax(), scale.shifterSemi, "shifter1Semi",
      Info::kCanAutomate);
    value[ID::shifter1Cent] = std::make_unique<LinearValue>(
      0.0, scale.shifterCent, "shifter1Cent", Info::kCanAutomate);
    value[ID::shifter1Gain] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "shifter1Gain", Info::kCanAutomate);
    value[ID::shifter2Semi] = std::make_unique<LinearValue>(
      14.001 / scale.shifterSemi.getMax(), scale.shifterSemi, "shifter2Semi",
      Info::kCanAutomate);
    value[ID::shifter2Cent] = std::make_unique<LinearValue>(
      0.0, scale.shifterCent, "shifter2Cent", Info::kCanAutomate);
    value[ID::shifter2Gain] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "shifter2Gain", Info::kCanAutomate);

    value[ID::lfoType]
      = std::make_unique<UIntValue>(0, scale.lfoType, "lfoType", Info::kCanAutomate);
    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      0, scale.boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoFrequency] = std::make_unique<LogValue>(
      0.5, scale.lfoFrequency, "lfoFrequency", Info::kCanAutomate);
    value[ID::lfoShape] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "lfoShape", Info::kCanAutomate);
    value[ID::lfoToPitch] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "lfoToPitch", Info::kCanAutomate);
    value[ID::lfoToSlope]
      = std::make_unique<LogValue>(0.0, scale.lfoSlope, "lfoToSlope", Info::kCanAutomate);
    value[ID::lfoToPulseWidth] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "lfoToPulseWidth", Info::kCanAutomate);
    value[ID::lfoToCutoff] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "lfoToCutoff", Info::kCanAutomate);

    value[ID::oscMix] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "oscMix", Info::kCanAutomate);
    value[ID::octave]
      = std::make_unique<LinearValue>(0.5, scale.octave, "octave", Info::kCanAutomate);

    value[ID::pitchSlideType] = std::make_unique<UIntValue>(
      1, scale.pitchSlideType, "pitchSlideType", Info::kCanAutomate);
    value[ID::pitchSlide] = std::make_unique<LogValue>(
      0.1, scale.pitchSlide, "pitchSlide", Info::kCanAutomate);
    value[ID::pitchSlideOffset] = std::make_unique<LinearValue>(
      0.5, scale.pitchSlideOffset, "pitchSlideOffset", Info::kCanAutomate);

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
