// (c) 2022 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t maxAllpass = 64;

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  mix,
  feedback,
  feedbackClip,
  feedbackHighpassHz,
  outputHighpassHz,

  stage,
  allpassSpread,
  allpassCenterHz,

  modSideChain,
  modType,
  modAmount,
  modAsymmetry,
  modLowpassHz,

  notePitchCenter,
  notePitchToAllpassCutoff,
  notePitchSlideSecond,
  notePitchReleaseSecond,

  parameterSmoothingSecond,
  oversampling,

  tooMuchFeedback,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = tooMuchFeedback,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> outputGain;
  static SomeDSP::LinearScale<double> feedback;
  static SomeDSP::DecibelScale<double> feedbackClip;

  static SomeDSP::UIntScale<double> stage;
  static SomeDSP::LinearScale<double> allpassSpread;
  static SomeDSP::DecibelScale<double> cutoffHz;

  static SomeDSP::UIntScale<double> modType;
  static SomeDSP::DecibelScale<double> modulation;
  static SomeDSP::LinearScale<double> modAsymmetry;

  static SomeDSP::LinearScale<double> notePitchCenter;
  static SomeDSP::LinearScale<double> notePitchToAllpassCutoff;
  static SomeDSP::DecibelScale<double> notePitchEnvelopeSecond;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
  static SomeDSP::UIntScale<double> oversampling;
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
      1.0, Scales::bipolarScale, "mix", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<LinearValue>(
      0.5, Scales::feedback, "feedback", Info::kCanAutomate);
    value[ID::feedbackClip] = std::make_unique<DecibelValue>(
      Scales::feedbackClip.invmap(1.0), Scales::feedbackClip, "feedback",
      Info::kCanAutomate);
    value[ID::feedbackHighpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(5.0), Scales::cutoffHz, "feedbackHighpassHz",
      Info::kCanAutomate);
    value[ID::outputHighpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(5.0), Scales::cutoffHz, "outputHighpassHz",
      Info::kCanAutomate);

    value[ID::stage]
      = std::make_unique<UIntValue>(15, Scales::stage, "stage", Info::kCanAutomate);
    value[ID::allpassSpread] = std::make_unique<LinearValue>(
      Scales::allpassSpread.invmap(1.0), Scales::allpassSpread, "allpassSpread",
      Info::kCanAutomate);
    value[ID::allpassCenterHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(100.0), Scales::cutoffHz, "allpassCenterHz",
      Info::kCanAutomate);

    value[ID::modSideChain] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "modSideChain", Info::kCanAutomate);
    value[ID::modType]
      = std::make_unique<UIntValue>(0, Scales::modType, "modType", Info::kCanAutomate);
    value[ID::modAmount] = std::make_unique<DecibelValue>(
      Scales::modulation.invmap(1.0), Scales::modulation, "modAmount",
      Info::kCanAutomate);
    value[ID::modAsymmetry] = std::make_unique<LinearValue>(
      Scales::modAsymmetry.invmap(0.0), Scales::modAsymmetry, "modAsymmetry",
      Info::kCanAutomate);
    value[ID::modLowpassHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(2000.0), Scales::cutoffHz, "modLowpassHz",
      Info::kCanAutomate);

    value[ID::notePitchCenter] = std::make_unique<LinearValue>(
      Scales::notePitchCenter.invmap(60.0), Scales::notePitchCenter, "notePitchCenter",
      Info::kCanAutomate);
    value[ID::notePitchToAllpassCutoff] = std::make_unique<LinearValue>(
      Scales::notePitchToAllpassCutoff.invmap(1.0), Scales::notePitchToAllpassCutoff,
      "notePitchToAllpassCutoff", Info::kCanAutomate);
    value[ID::notePitchSlideSecond] = std::make_unique<DecibelValue>(
      Scales::notePitchEnvelopeSecond.invmap(0.001), Scales::notePitchEnvelopeSecond,
      "notePitchSlideSecond", Info::kCanAutomate);
    value[ID::notePitchReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::notePitchEnvelopeSecond.invmap(1.0), Scales::notePitchEnvelopeSecond,
      "notePitchReleaseSecond", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.2), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);
    value[ID::oversampling] = std::make_unique<UIntValue>(
      1, Scales::oversampling, "oversampling", Info::kCanAutomate);

    value[ID::tooMuchFeedback] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "tooMuchFeedback", Info::kIsReadOnly);

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
