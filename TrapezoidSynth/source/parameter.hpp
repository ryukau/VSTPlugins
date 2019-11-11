// (c) 2019 Takamitsu Endo
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

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "value.hpp"

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
  static SomeDSP::IntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LinearScale<double> semi;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::LinearScale<double> overtone;
  static SomeDSP::LinearScale<double> pitchDrift;
  static SomeDSP::LogScale<double> oscSlope;
  static SomeDSP::LogScale<double> oscFeedback;
  static SomeDSP::LogScale<double> phaseModulation;

  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::LogScale<double> envelopeS;
  static SomeDSP::LogScale<double> envelopeR;

  static SomeDSP::LogScale<double> filterCutoff;
  static SomeDSP::LogScale<double> filterResonance;
  static SomeDSP::LogScale<double> filterSaturation;
  static SomeDSP::IntScale<double> filterOrder;
  static SomeDSP::SPolyScale<double> filterEnvToCutoff;
  static SomeDSP::LinearScale<double> filterKeyToCutoff;
  static SomeDSP::LinearScale<double> oscMixToFilterCutoff;
  static SomeDSP::LinearScale<double> filterEnvToOctave;

  static SomeDSP::LogScale<double> modEnvelopeA;
  static SomeDSP::LogScale<double> modEnvelopeCurve;
  static SomeDSP::LogScale<double> modEnvToShifter;

  static SomeDSP::LinearScale<double> shifterSemi;
  static SomeDSP::LinearScale<double> shifterCent;

  static SomeDSP::IntScale<double> lfoType;
  static SomeDSP::LogScale<double> lfoFrequency;
  static SomeDSP::LogScale<double> lfoSlope;

  static SomeDSP::LinearScale<double> octave;

  static SomeDSP::IntScale<double> pitchSlideType;
  static SomeDSP::LogScale<double> pitchSlide;
  static SomeDSP::LinearScale<double> pitchSlideOffset;

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
    using SPolyValue = FloatValue<SomeDSP::SPolyScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass,
      ID::bypass);

    value[ID::osc1Semi] = std::make_unique<LinearValue>(
      0.5, Scales::semi, "osc1Semi", Info::kCanAutomate, ID::osc1Semi);
    value[ID::osc1Cent] = std::make_unique<LinearValue>(
      0.5, Scales::cent, "osc1Cent", Info::kCanAutomate, ID::osc1Cent);
    value[ID::osc1PitchDrift] = std::make_unique<LinearValue>(
      0.0, Scales::pitchDrift, "osc1PitchDrift", Info::kCanAutomate, ID::osc1PitchDrift);
    value[ID::osc1Slope] = std::make_unique<LogValue>(
      0.5, Scales::oscSlope, "osc1Slope", Info::kCanAutomate, ID::osc1Slope);
    value[ID::osc1PulseWidth] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "osc1PulseWidth", Info::kCanAutomate,
      ID::osc1PulseWidth);
    value[ID::osc1Feedback] = std::make_unique<LogValue>(
      0.0, Scales::oscFeedback, "osc1Feedback", Info::kCanAutomate, ID::osc1Feedback);

    value[ID::osc2Semi] = std::make_unique<LinearValue>(
      0.5, Scales::semi, "osc2Semi", Info::kCanAutomate, ID::osc2Semi);
    value[ID::osc2Cent] = std::make_unique<LinearValue>(
      0.5, Scales::cent, "osc2Cent", Info::kCanAutomate, ID::osc2Cent);
    value[ID::osc2Overtone] = std::make_unique<LinearValue>(
      0, Scales::overtone, "osc2Overtone", Info::kCanAutomate, ID::osc2Overtone);
    value[ID::osc2Slope] = std::make_unique<LogValue>(
      0.5, Scales::oscSlope, "osc2Slope", Info::kCanAutomate, ID::osc2Slope);
    value[ID::osc2PulseWidth] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "osc2PulseWidth", Info::kCanAutomate,
      ID::osc2PulseWidth);
    value[ID::pmOsc2ToOsc1] = std::make_unique<LogValue>(
      0.0, Scales::phaseModulation, "pmOsc2ToOsc1", Info::kCanAutomate, ID::pmOsc2ToOsc1);

    value[ID::gain] = std::make_unique<LogValue>(
      0.5, Scales::gain, "gain", Info::kCanAutomate, ID::gain);
    value[ID::gainA] = std::make_unique<LogValue>(
      0.0, Scales::envelopeA, "gainA", Info::kCanAutomate, ID::gainA);
    value[ID::gainD] = std::make_unique<LogValue>(
      0.5, Scales::envelopeD, "gainD", Info::kCanAutomate, ID::gainD);
    value[ID::gainS] = std::make_unique<LogValue>(
      0.8, Scales::envelopeS, "gainS", Info::kCanAutomate, ID::gainS);
    value[ID::gainR] = std::make_unique<LogValue>(
      0.1, Scales::envelopeR, "gainR", Info::kCanAutomate, ID::gainR);
    value[ID::gainCurve] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "gainCurve", Info::kCanAutomate, ID::gainCurve);
    value[ID::gainEnvRetrigger] = std::make_unique<IntValue>(
      true, Scales::boolScale, "gainEnvRetrigger", Info::kCanAutomate,
      ID::gainEnvRetrigger);

    value[ID::filterOrder] = std::make_unique<IntValue>(
      7, Scales::filterOrder, "filterOrder", Info::kCanAutomate, ID::filterOrder);
    value[ID::filterCutoff] = std::make_unique<LogValue>(
      1.0, Scales::filterCutoff, "filterCutoff", Info::kCanAutomate, ID::filterCutoff);
    value[ID::filterFeedback] = std::make_unique<LogValue>(
      0.0, Scales::filterResonance, "filterFeedback", Info::kCanAutomate,
      ID::filterFeedback);
    value[ID::filterSaturation] = std::make_unique<LogValue>(
      0.3, Scales::filterSaturation, "filterSaturation", Info::kCanAutomate,
      ID::filterSaturation);
    value[ID::filterEnvToCutoff] = std::make_unique<SPolyValue>(
      0.5, Scales::filterEnvToCutoff, "filterEnvToCutoff", Info::kCanAutomate,
      ID::filterEnvToCutoff);
    value[ID::filterKeyToCutoff] = std::make_unique<LinearValue>(
      0.5, Scales::filterKeyToCutoff, "filterKeyToCutoff", Info::kCanAutomate,
      ID::filterKeyToCutoff);
    value[ID::oscMixToFilterCutoff] = std::make_unique<LinearValue>(
      0.0, Scales::oscMixToFilterCutoff, "oscMixToFilterCutoff", Info::kCanAutomate,
      ID::oscMixToFilterCutoff);

    value[ID::filterA] = std::make_unique<LogValue>(
      0.0, Scales::envelopeA, "filterA", Info::kCanAutomate, ID::filterA);
    value[ID::filterD] = std::make_unique<LogValue>(
      0.5, Scales::envelopeD, "filterD", Info::kCanAutomate, ID::filterD);
    value[ID::filterS] = std::make_unique<LogValue>(
      0.0, Scales::envelopeS, "filterS", Info::kCanAutomate, ID::filterS);
    value[ID::filterR] = std::make_unique<LogValue>(
      0.5, Scales::envelopeR, "filterR", Info::kCanAutomate, ID::filterR);
    value[ID::filterCurve] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "filterCurve", Info::kCanAutomate, ID::filterCurve);
    value[ID::filterEnvToOctave] = std::make_unique<LinearValue>(
      0.5, Scales::filterEnvToOctave, "filterEnvToOctave", Info::kCanAutomate,
      ID::filterEnvToOctave);
    value[ID::filterEnvRetrigger] = std::make_unique<IntValue>(
      true, Scales::boolScale, "filterEnvRetrigger", Info::kCanAutomate,
      ID::filterEnvRetrigger);

    value[ID::modEnv1Attack] = std::make_unique<LogValue>(
      0.0, Scales::modEnvelopeA, "modEnv1Attack", Info::kCanAutomate, ID::modEnv1Attack);
    value[ID::modEnv1Curve] = std::make_unique<LogValue>(
      0.5, Scales::modEnvelopeCurve, "modEnv1Curve", Info::kCanAutomate,
      ID::modEnv1Curve);
    value[ID::modEnv1ToPhaseMod] = std::make_unique<LogValue>(
      0.0, Scales::phaseModulation, "modEnv1ToPhaseMod", Info::kCanAutomate,
      ID::modEnv1ToPhaseMod);
    value[ID::modEnv1Retrigger] = std::make_unique<IntValue>(
      true, Scales::boolScale, "modEnv1Retrigger", Info::kCanAutomate,
      ID::modEnv1Retrigger);

    value[ID::modEnv2Attack] = std::make_unique<LogValue>(
      0.0, Scales::modEnvelopeA, "modEnv2Attack", Info::kCanAutomate, ID::modEnv2Attack);
    value[ID::modEnv2Curve] = std::make_unique<LogValue>(
      0.5, Scales::modEnvelopeCurve, "modEnv2Curve", Info::kCanAutomate,
      ID::modEnv2Curve);
    value[ID::modEnv2ToFeedback] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "modEnv2ToFeedback", Info::kCanAutomate,
      ID::modEnv2ToFeedback);
    value[ID::modEnv2ToLFOFrequency] = std::make_unique<LogValue>(
      0.0, Scales::lfoFrequency, "modEnv2ToLFOFrequency", Info::kCanAutomate,
      ID::modEnv2ToLFOFrequency);
    value[ID::modEnv2ToOsc2Slope] = std::make_unique<LogValue>(
      0.0, Scales::oscSlope, "modEnv2ToOsc2Slope", Info::kCanAutomate,
      ID::modEnv2ToOsc2Slope);
    value[ID::modEnv2ToShifter1] = std::make_unique<LogValue>(
      0.0, Scales::modEnvToShifter, "modEnv2ToShifter1", Info::kCanAutomate,
      ID::modEnv2ToShifter1);
    value[ID::modEnv2Retrigger] = std::make_unique<IntValue>(
      true, Scales::boolScale, "modEnv2Retrigger", Info::kCanAutomate,
      ID::modEnv2Retrigger);

    value[ID::shifter1Semi] = std::make_unique<LinearValue>(
      7.001 / Scales::shifterSemi.getMax(), Scales::shifterSemi, "shifter1Semi",
      Info::kCanAutomate, ID::shifter1Semi);
    value[ID::shifter1Cent] = std::make_unique<LinearValue>(
      0.0, Scales::shifterCent, "shifter1Cent", Info::kCanAutomate, ID::shifter1Cent);
    value[ID::shifter1Gain] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "shifter1Gain", Info::kCanAutomate, ID::shifter1Gain);
    value[ID::shifter2Semi] = std::make_unique<LinearValue>(
      14.001 / Scales::shifterSemi.getMax(), Scales::shifterSemi, "shifter2Semi",
      Info::kCanAutomate, ID::shifter2Semi);
    value[ID::shifter2Cent] = std::make_unique<LinearValue>(
      0.0, Scales::shifterCent, "shifter2Cent", Info::kCanAutomate, ID::shifter2Cent);
    value[ID::shifter2Gain] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "shifter2Gain", Info::kCanAutomate, ID::shifter2Gain);

    value[ID::lfoType] = std::make_unique<IntValue>(
      0, Scales::lfoType, "lfoType", Info::kCanAutomate, ID::lfoType);
    value[ID::lfoTempoSync] = std::make_unique<IntValue>(
      0, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate, ID::lfoTempoSync);
    value[ID::lfoFrequency] = std::make_unique<LogValue>(
      0.5, Scales::lfoFrequency, "lfoFrequency", Info::kCanAutomate, ID::lfoFrequency);
    value[ID::lfoShape] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "lfoShape", Info::kCanAutomate, ID::lfoShape);
    value[ID::lfoToPitch] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToPitch", Info::kCanAutomate, ID::lfoToPitch);
    value[ID::lfoToSlope] = std::make_unique<LogValue>(
      0.0, Scales::lfoSlope, "lfoToSlope", Info::kCanAutomate, ID::lfoToSlope);
    value[ID::lfoToPulseWidth] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToPulseWidth", Info::kCanAutomate,
      ID::lfoToPulseWidth);
    value[ID::lfoToCutoff] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToCutoff", Info::kCanAutomate, ID::lfoToCutoff);

    value[ID::oscMix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "oscMix", Info::kCanAutomate, ID::oscMix);
    value[ID::octave] = std::make_unique<LinearValue>(
      0.5, Scales::octave, "octave", Info::kCanAutomate, ID::octave);

    value[ID::pitchSlideType] = std::make_unique<IntValue>(
      1, Scales::pitchSlideType, "pitchSlideType", Info::kCanAutomate,
      ID::pitchSlideType);
    value[ID::pitchSlide] = std::make_unique<LogValue>(
      0.1, Scales::pitchSlide, "pitchSlide", Info::kCanAutomate, ID::pitchSlide);
    value[ID::pitchSlideOffset] = std::make_unique<LinearValue>(
      0.5, Scales::pitchSlideOffset, "pitchSlideOffset", Info::kCanAutomate,
      ID::pitchSlideOffset);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate, ID::smoothness);

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

  void validate()
  {
    for (size_t i = 0; i < value.size(); ++i) {
      if (value[i] == nullptr) {
        std::cout << "PluginError: GlobalParameter::value[" << std::to_string(i)
                  << "] is nullptr. Forgetting initialization?\n";
        std::exit(EXIT_FAILURE);
      }
    }
  }
};

} // namespace Synth
} // namespace Steinberg
