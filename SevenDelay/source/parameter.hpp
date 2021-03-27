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

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
#include "../../test/value.hpp"
#else
#include "../../common/value.hpp"
#endif

namespace Steinberg {
namespace Synth {

constexpr double maxDelayTime = 8.0;
constexpr double maxToneFrequency = 20000.0;
constexpr double minDCKillFrequency = 5.0;

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
  static SomeDSP::UIntScale<double> boolScale;
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

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using LogValue = DoubleValue<SomeDSP::LogScale<double>>;
    using SPolyValue = DoubleValue<SomeDSP::SPolyScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "Bypass", Info::kCanAutomate | Info::kIsBypass);
    value[ID::time]
      = std::make_unique<LogValue>(0.5, Scales::time, "Time", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.625, Scales::defaultScale, "Feedback", Info::kCanAutomate);
    value[ID::offset]
      = std::make_unique<SPolyValue>(0.5, Scales::offset, "Stereo", Info::kCanAutomate);
    value[ID::wetMix] = std::make_unique<LinearValue>(
      0.75, Scales::defaultScale, "WetMix", Info::kCanAutomate);
    value[ID::dryMix] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "DryMix", Info::kCanAutomate);
    value[ID::tempoSync] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "TempoSync", Info::kCanAutomate);
    value[ID::negativeFeedback] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "NegativeFeedback", Info::kCanAutomate);
    value[ID::lfoTimeAmount] = std::make_unique<LogValue>(
      0.0, Scales::lfoTimeAmount, "LFO to Time", Info::kCanAutomate);
    value[ID::lfoFrequency] = std::make_unique<LogValue>(
      0.5, Scales::lfoFrequency, "LFO Frequency", Info::kCanAutomate);
    value[ID::lfoShape] = std::make_unique<LogValue>(
      0.5, Scales::lfoShape, "LFO Shape", Info::kCanAutomate);
    value[ID::lfoInitialPhase] = std::make_unique<LinearValue>(
      0.0, Scales::lfoInitialPhase, "LFO Initial Phase", Info::kCanAutomate);
    value[ID::lfoHold] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "LFO Phase Hold", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.3, Scales::smoothness, "Smoothness", Info::kCanAutomate);
    value[ID::inSpread] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "Input Stereo Spread", Info::kCanAutomate);
    value[ID::inPan] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "Input Pan", Info::kCanAutomate);
    value[ID::outSpread] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "Output Stereo Spread", Info::kCanAutomate);
    value[ID::outPan] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "Output Pan", Info::kCanAutomate);
    value[ID::toneCutoff] = std::make_unique<LogValue>(
      1.0, Scales::toneCutoff, "Allpass Cutoff", Info::kCanAutomate);
    value[ID::dckill]
      = std::make_unique<LogValue>(0.0, Scales::dckill, "DC Kill", Info::kCanAutomate);
    value[ID::lfoToneAmount] = std::make_unique<LogValue>(
      0.0, Scales::lfoToneAmount, "LFO to Allpass", Info::kCanAutomate);
    value[ID::toneQ]
      = std::make_unique<LogValue>(0.9, Scales::toneQ, "Allpass Q", Info::kCanAutomate);

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
