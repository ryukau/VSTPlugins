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

    using ValueBool = IntValue;
    using ValueLinear = FloatValue<SomeDSP::LinearScale<double>>;
    using ValueLog = FloatValue<SomeDSP::LogScale<double>>;
    using ValueSPoly = FloatValue<SomeDSP::SPolyScale<double>>;

    value[ParameterID::bypass] = std::make_unique<ValueBool>(
      0.0, 1, "Bypass", Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
      ParameterID::bypass);
    value[ParameterID::time] = std::make_unique<ValueLog>(
      0.5, Scales::time, "Time", Vst::ParameterInfo::kCanAutomate, ParameterID::time);
    value[ParameterID::feedback] = std::make_unique<ValueLinear>(
      0.625, Scales::defaultScale, "Feedback", Vst::ParameterInfo::kCanAutomate,
      ParameterID::feedback);
    value[ParameterID::offset] = std::make_unique<ValueSPoly>(
      0.5, Scales::offset, "Stereo", Vst::ParameterInfo::kCanAutomate,
      ParameterID::offset);
    value[ParameterID::wetMix] = std::make_unique<ValueLinear>(
      0.75, Scales::defaultScale, "WetMix", Vst::ParameterInfo::kCanAutomate,
      ParameterID::wetMix);
    value[ParameterID::dryMix] = std::make_unique<ValueLinear>(
      1.0, Scales::defaultScale, "DryMix", Vst::ParameterInfo::kCanAutomate,
      ParameterID::dryMix);
    value[ParameterID::tempoSync] = std::make_unique<ValueBool>(
      0.0, 1, "TempoSync", Vst::ParameterInfo::kCanAutomate, ParameterID::tempoSync);
    value[ParameterID::negativeFeedback] = std::make_unique<ValueBool>(
      0.0, 1, "NegativeFeedback", Vst::ParameterInfo::kCanAutomate,
      ParameterID::negativeFeedback);
    value[ParameterID::lfoTimeAmount] = std::make_unique<ValueLog>(
      0.0, Scales::lfoTimeAmount, "LFO to Time", Vst::ParameterInfo::kCanAutomate,
      ParameterID::lfoTimeAmount);
    value[ParameterID::lfoFrequency] = std::make_unique<ValueLog>(
      0.5, Scales::lfoFrequency, "LFO Frequency", Vst::ParameterInfo::kCanAutomate,
      ParameterID::lfoFrequency);
    value[ParameterID::lfoShape] = std::make_unique<ValueLog>(
      0.5, Scales::lfoShape, "LFO Shape", Vst::ParameterInfo::kCanAutomate,
      ParameterID::lfoShape);
    value[ParameterID::lfoInitialPhase] = std::make_unique<ValueLinear>(
      0.0, Scales::lfoInitialPhase, "LFO Initial Phase", Vst::ParameterInfo::kCanAutomate,
      ParameterID::lfoInitialPhase);
    value[ParameterID::lfoHold] = std::make_unique<ValueBool>(
      0.0, 1, "LFO Phase Hold", Vst::ParameterInfo::kCanAutomate, ParameterID::lfoHold);
    value[ParameterID::smoothness] = std::make_unique<ValueLog>(
      0.3, Scales::smoothness, "Smoothness", Vst::ParameterInfo::kCanAutomate,
      ParameterID::smoothness);
    value[ParameterID::inSpread] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Input Stereo Spread", Vst::ParameterInfo::kCanAutomate,
      ParameterID::inSpread);
    value[ParameterID::inPan] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, "Input Pan", Vst::ParameterInfo::kCanAutomate,
      ParameterID::inPan);
    value[ParameterID::outSpread] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Output Stereo Spread", Vst::ParameterInfo::kCanAutomate,
      ParameterID::outSpread);
    value[ParameterID::outPan] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, "Output Pan", Vst::ParameterInfo::kCanAutomate,
      ParameterID::outPan);
    value[ParameterID::toneCutoff] = std::make_unique<ValueLog>(
      1.0, Scales::toneCutoff, "Allpass Cutoff", Vst::ParameterInfo::kCanAutomate,
      ParameterID::toneCutoff);
    value[ParameterID::dckill] = std::make_unique<ValueLog>(
      0.0, Scales::dckill, "DC Kill", Vst::ParameterInfo::kCanAutomate,
      ParameterID::dckill);
    value[ParameterID::lfoToneAmount] = std::make_unique<ValueLog>(
      0.0, Scales::lfoToneAmount, "LFO to Allpass", Vst::ParameterInfo::kCanAutomate,
      ParameterID::lfoToneAmount);
    value[ParameterID::toneQ] = std::make_unique<ValueLog>(
      0.9, Scales::toneQ, "Allpass Q", Vst::ParameterInfo::kCanAutomate,
      ParameterID::toneQ);
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
