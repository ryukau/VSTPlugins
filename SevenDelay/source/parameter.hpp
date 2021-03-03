// (c) 2019-2020 Takamitsu Endo
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

#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

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
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , time(0.0001, maxDelayTime, 0.5, 1.0)
    , offset(-1.0, 1.0, 0.8)
    , lfoTimeAmount(0, 1.0, 0.5, 0.07)
    , lfoToneAmount(0, 0.5, 0.5, 0.1)
    , lfoFrequency(0.01, 100.0, 0.5, 1.0)
    , lfoShape(0.01, 10.0, 0.5, 1.0)
    , lfoInitialPhase(0.0, 2.0 * pi)
    , smoothness(0.0, 1.0, 0.3, 0.04)
    , toneCutoff(90.0, maxToneFrequency, 0.5, 1000.0)
    , toneQ(1e-5, 1.0, 0.5, 0.1)
    , toneMix(0.0, 1.0, 0.9, 0.05)
    , dckill(minDCKillFrequency, 120.0, 0.5, 20.0)
    , dckillMix(0.0, 1.0, 0.9, 0.05)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> time;
  SomeDSP::SPolyScale<double> offset;
  SomeDSP::LogScale<double> lfoTimeAmount;
  SomeDSP::LogScale<double> lfoToneAmount;
  SomeDSP::LogScale<double> lfoFrequency;
  SomeDSP::LogScale<double> lfoShape;
  SomeDSP::LinearScale<double> lfoInitialPhase;
  SomeDSP::LogScale<double> smoothness;
  SomeDSP::LogScale<double> toneCutoff;
  SomeDSP::LogScale<double> toneQ;
  SomeDSP::LogScale<double> toneMix; // internal
  SomeDSP::LogScale<double> dckill;
  SomeDSP::LogScale<double> dckillMix; // internal
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
      0.0, scale.boolScale, "Bypass", Info::kCanAutomate | Info::kIsBypass);
    value[ID::time]
      = std::make_unique<LogValue>(0.5, scale.time, "Time", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.625, scale.defaultScale, "Feedback", Info::kCanAutomate);
    value[ID::offset]
      = std::make_unique<SPolyValue>(0.5, scale.offset, "Stereo", Info::kCanAutomate);
    value[ID::wetMix] = std::make_unique<LinearValue>(
      0.75, scale.defaultScale, "WetMix", Info::kCanAutomate);
    value[ID::dryMix] = std::make_unique<LinearValue>(
      1.0, scale.defaultScale, "DryMix", Info::kCanAutomate);
    value[ID::tempoSync] = std::make_unique<UIntValue>(
      0.0, scale.boolScale, "TempoSync", Info::kCanAutomate);
    value[ID::negativeFeedback] = std::make_unique<UIntValue>(
      0.0, scale.boolScale, "NegativeFeedback", Info::kCanAutomate);
    value[ID::lfoTimeAmount] = std::make_unique<LogValue>(
      0.0, scale.lfoTimeAmount, "LFO to Time", Info::kCanAutomate);
    value[ID::lfoFrequency] = std::make_unique<LogValue>(
      0.5, scale.lfoFrequency, "LFO Frequency", Info::kCanAutomate);
    value[ID::lfoShape]
      = std::make_unique<LogValue>(0.5, scale.lfoShape, "LFO Shape", Info::kCanAutomate);
    value[ID::lfoInitialPhase] = std::make_unique<LinearValue>(
      0.0, scale.lfoInitialPhase, "LFO Initial Phase", Info::kCanAutomate);
    value[ID::lfoHold] = std::make_unique<UIntValue>(
      0.0, scale.boolScale, "LFO Phase Hold", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.3, scale.smoothness, "Smoothness", Info::kCanAutomate);
    value[ID::inSpread] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "Input Stereo Spread", Info::kCanAutomate);
    value[ID::inPan] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "Input Pan", Info::kCanAutomate);
    value[ID::outSpread] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "Output Stereo Spread", Info::kCanAutomate);
    value[ID::outPan] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "Output Pan", Info::kCanAutomate);
    value[ID::toneCutoff] = std::make_unique<LogValue>(
      1.0, scale.toneCutoff, "Allpass Cutoff", Info::kCanAutomate);
    value[ID::dckill]
      = std::make_unique<LogValue>(0.0, scale.dckill, "DC Kill", Info::kCanAutomate);
    value[ID::lfoToneAmount] = std::make_unique<LogValue>(
      0.0, scale.lfoToneAmount, "LFO to Allpass", Info::kCanAutomate);
    value[ID::toneQ]
      = std::make_unique<LogValue>(0.9, scale.toneQ, "Allpass Q", Info::kCanAutomate);

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
