// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <vector>

#include "dsp/scale.hpp"
#include "value.hpp"

namespace Steinberg {
namespace SevenDelay {

constexpr Vst::ParamValue maxDelayTime = 8.0;
constexpr Vst::ParamValue maxToneFrequency = 20000.0;
constexpr Vst::ParamValue minDCKillFrequency = 5.0;

namespace ParameterID {
enum ID : Vst::ParamID {
  bypass,
  time,
  feedback,
  offset,
  wetMix,
  dryMix,
  tempoSync,
  negativeFeedback,
  lfoTimeAmount,
  lfoFrequency,
  lfoShape,
  lfoInitialPhase,
  lfoHold,
  smoothness,
  inSpread,
  inPan,
  outSpread,
  outPan,
  toneCutoff,
  dckill,
  lfoToneAmount,
  toneQ,
  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::BoolScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LogScale<double> time;
  static SomeDSP::SPolyScale<double> offset;
  static SomeDSP::LogScale<double> lfoTimeAmount;
  static SomeDSP::LogScale<double> lfoToneAmount;
  static SomeDSP::LogScale<double> lfoFrequency;
  static SomeDSP::LogScale<double> lfoShape;
  static SomeDSP::LinearScale<double> lfoInitialPhase;
  static SomeDSP::LogScale<double> smoothness;
  static SomeDSP::LogScale<double> toneCutoff;
  static SomeDSP::LogScale<double> toneQ;
  static SomeDSP::LogScale<double> toneMix; // internal
  static SomeDSP::LogScale<double> dckill;
  static SomeDSP::LogScale<double> dckillMix; // internal
};

struct GlobalParameter {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using ID = ParameterID::ID;
    using ValueBool = InternalValue<SomeDSP::BoolScale<double>, bool>;
    using ValueLinear = InternalValue<SomeDSP::LinearScale<double>, double>;
    using ValueLog = InternalValue<SomeDSP::LogScale<double>, double>;
    using ValueSPoly = InternalValue<SomeDSP::SPolyScale<double>, double>;

    value[ParameterID::bypass] = std::make_unique<ValueBool>(
      0.0, Scales::boolScale, ParameterID::bypass, "Bypass",
      Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass);
    value[ParameterID::time] = std::make_unique<ValueLog>(
      0.5, Scales::time, ParameterID::time, "Time", Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::feedback] = std::make_unique<ValueLinear>(
      0.625, Scales::defaultScale, ParameterID::feedback, "Feedback",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::offset] = std::make_unique<ValueSPoly>(
      0.5, Scales::offset, ParameterID::offset, "Stereo",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::wetMix] = std::make_unique<ValueLinear>(
      0.75, Scales::defaultScale, ParameterID::wetMix, "WetMix",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::dryMix] = std::make_unique<ValueLinear>(
      1.0, Scales::defaultScale, ParameterID::dryMix, "DryMix",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::tempoSync] = std::make_unique<ValueBool>(
      0.0, Scales::boolScale, ParameterID::tempoSync, "TempoSync",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::negativeFeedback] = std::make_unique<ValueBool>(
      0.0, Scales::boolScale, ParameterID::negativeFeedback, "NegativeFeedback",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoTimeAmount] = std::make_unique<ValueLog>(
      0.0, Scales::lfoTimeAmount, ParameterID::lfoTimeAmount, "LFO to Time",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoFrequency] = std::make_unique<ValueLog>(
      0.5, Scales::lfoFrequency, ParameterID::lfoFrequency, "LFO Frequency",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoShape] = std::make_unique<ValueLog>(
      0.5, Scales::lfoShape, ParameterID::lfoShape, "LFO Shape",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoInitialPhase] = std::make_unique<ValueLinear>(
      0.0, Scales::lfoInitialPhase, ParameterID::lfoInitialPhase, "LFO Initial Phase",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoHold] = std::make_unique<ValueBool>(
      0.0, Scales::boolScale, ParameterID::lfoHold, "LFO Phase Hold",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::smoothness] = std::make_unique<ValueLog>(
      0.3, Scales::smoothness, ParameterID::smoothness, "Smoothness",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::inSpread] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, ParameterID::inSpread, "Input Stereo Spread",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::inPan] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, ParameterID::inPan, "Input Pan",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::outSpread] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, ParameterID::outSpread, "Output Stereo Spread",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::outPan] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, ParameterID::outPan, "Output Pan",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::toneCutoff] = std::make_unique<ValueLog>(
      1.0, Scales::toneCutoff, ParameterID::toneCutoff, "Allpass Cutoff",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::dckill] = std::make_unique<ValueLog>(
      0.0, Scales::dckill, ParameterID::dckill, "DC Kill",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::lfoToneAmount] = std::make_unique<ValueLog>(
      0.0, Scales::lfoToneAmount, ParameterID::lfoToneAmount, "LFO to Allpass",
      Vst::ParameterInfo::kCanAutomate);
    value[ParameterID::toneQ] = std::make_unique<ValueLog>(
      0.9, Scales::toneQ, ParameterID::toneQ, "Allpass Q",
      Vst::ParameterInfo::kCanAutomate);
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
};

} // namespace SevenDelay
} // namespace Steinberg
