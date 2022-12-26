// (c) 2022 Takamitsu Endo
//
// This file is part of CombDistortion.
//
// CombDistortion is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CombDistortion is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CombDistortion.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

namespace Steinberg {
namespace Synth {

constexpr double maxDelayTime = 1.0;

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  mix,
  feedback,
  feedbackHighpassHz,
  feedbackLowpassHz,

  delayTimeSeconds,

  amMix,
  amClipGain,
  amClipGainNegative,

  fmMix,
  fmAmount,
  fmClip,

  parameterSmoothingSecond,
  oversampling,

  notePitchOrigin,
  notePitchToDelayTime,
  noteReleaseSeconds,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> outputGain;
  static SomeDSP::LinearScale<double> feedback;
  static SomeDSP::DecibelScale<double> highpassHz;
  static SomeDSP::DecibelScale<double> lowpassHz;
  static SomeDSP::DecibelScale<double> delayTimeSeconds;
  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
  static SomeDSP::UIntScale<double> oversampling;
  static SomeDSP::LinearScale<double> notePitchOrigin;
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
      Scales::outputGain.invmapDB(0.0), Scales::outputGain, "outputGain",
      Info::kCanAutomate);
    value[ID::mix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "mix", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.5, Scales::feedback, "feedback", Info::kCanAutomate);
    value[ID::feedbackHighpassHz] = std::make_unique<DecibelValue>(
      Scales::highpassHz.invmap(10.0), Scales::highpassHz, "feedbackHighpassHz",
      Info::kCanAutomate);
    value[ID::feedbackLowpassHz] = std::make_unique<DecibelValue>(
      1.0, Scales::lowpassHz, "feedbackLowpassHz", Info::kCanAutomate);

    value[ID::delayTimeSeconds] = std::make_unique<DecibelValue>(
      Scales::delayTimeSeconds.invmap(0.0002), Scales::delayTimeSeconds,
      "delayTimeSeconds", Info::kCanAutomate);

    value[ID::amMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "amMix", Info::kCanAutomate);
    value[ID::amClipGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "amClipGain", Info::kCanAutomate);
    value[ID::amClipGainNegative] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "amClipGainNegative", Info::kCanAutomate);

    value[ID::fmMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "fmMix", Info::kCanAutomate);
    value[ID::fmAmount] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "fmAmount", Info::kCanAutomate);
    value[ID::fmClip]
      = std::make_unique<DecibelValue>(1.0, Scales::gain, "fmClip", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.2), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);
    value[ID::oversampling] = std::make_unique<UIntValue>(
      2, Scales::oversampling, "oversampling", Info::kCanAutomate);

    value[ID::notePitchOrigin] = std::make_unique<LinearValue>(
      Scales::notePitchOrigin.invmap(60.0), Scales::notePitchOrigin, "notePitchOrigin",
      Info::kCanAutomate);
    value[ID::notePitchToDelayTime] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "notePitchToDelayTime", Info::kCanAutomate);
    value[ID::noteReleaseSeconds] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(4), Scales::parameterSmoothingSecond,
      "noteReleaseSeconds", Info::kCanAutomate);

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
