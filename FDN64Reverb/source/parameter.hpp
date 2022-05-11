// (c) 2021-2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"
#include "dsp/matrixtype.hpp"

#ifdef TEST_DSP
#include "../../test/value.hpp"
#else
#include "../../common/value.hpp"
#endif

constexpr size_t nDelay = 64;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  delayTime0,
  timeLfoAmount0 = delayTime0 + nDelay,
  lowpassCutoffHz0 = timeLfoAmount0 + nDelay,
  highpassCutoffHz0 = lowpassCutoffHz0 + nDelay,

  timeMultiplier = highpassCutoffHz0 + nDelay,
  feedback,
  delayTimeInterpRate,
  seed,
  matrixType,
  gateThreshold,

  dry,
  wet,
  stereoCross,

  splitRotationHz,
  splitPhaseOffset,
  splitSkew,

  refreshMatrix,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LogScale<double> delayTime;
  static SomeDSP::SemitoneScale<double> lowpassCutoffHz;
  static SomeDSP::SemitoneScale<double> highpassCutoffHz;
  static SomeDSP::UIntScale<double> matrixType;
  static SomeDSP::DecibelScale<double> gateThreshold;
  static SomeDSP::DecibelScale<double> dry;
  static SomeDSP::DecibelScale<double> wet;
  static SomeDSP::LogScale<double> feedback;
  static SomeDSP::LogScale<double> delayTimeInterpRate;
  static SomeDSP::UIntScale<double> seed;
  static SomeDSP::LogScale<double> splitRotationHz;
  static SomeDSP::LinearScale<double> splitSkew;
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
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using SemitoneValue = DoubleValue<SomeDSP::SemitoneScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string delayTimeLabel("delayTime");
    std::string timeLfoAmountLabel("timeLfoAmount");
    std::string lowpassCutoffHzLabel("lowpassCutoffHz");
    std::string highpassCutoffHzLabel("highpassCutoffHz");

    for (size_t idx = 0; idx < nDelay; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::delayTime0 + idx] = std::make_unique<LogValue>(
        Scales::delayTime.invmap(0.1), Scales::delayTime,
        (delayTimeLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::timeLfoAmount0 + idx] = std::make_unique<LogValue>(
        0.0, Scales::delayTime, (timeLfoAmountLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::lowpassCutoffHz0 + idx] = std::make_unique<SemitoneValue>(
        Scales::highpassCutoffHz.invmap(22000.0), Scales::lowpassCutoffHz,
        (lowpassCutoffHzLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::highpassCutoffHz0 + idx] = std::make_unique<SemitoneValue>(
        Scales::highpassCutoffHz.invmap(5.0), Scales::highpassCutoffHz,
        (highpassCutoffHzLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::timeMultiplier] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "timeMultiplier", Info::kCanAutomate);
    value[ID::feedback]
      = std::make_unique<LogValue>(0.5, Scales::feedback, "feedback", Info::kCanAutomate);
    value[ID::delayTimeInterpRate] = std::make_unique<LogValue>(
      Scales::delayTimeInterpRate.invmap(0.5), Scales::delayTimeInterpRate,
      "delayTimeInterpRate", Info::kCanAutomate);
    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::matrixType] = std::make_unique<UIntValue>(
      SomeDSP::FeedbackMatrixType::specialOrthogonal, Scales::matrixType, "matrixType",
      Info::kCanAutomate);
    value[ID::gateThreshold] = std::make_unique<DecibelValue>(
      0.0, Scales::gateThreshold, "gateThreshold", Info::kCanAutomate);

    value[ID::dry] = std::make_unique<DecibelValue>(
      Scales::dry.invmapDB(0.0), Scales::dry, "dry", Info::kCanAutomate);
    value[ID::wet] = std::make_unique<DecibelValue>(
      Scales::wet.invmapDB(0.0), Scales::wet, "wet", Info::kCanAutomate);
    value[ID::stereoCross] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "stereoCross", Info::kCanAutomate);

    value[ID::splitRotationHz] = std::make_unique<LogValue>(
      0.0, Scales::splitRotationHz, "splitRotationHz", Info::kCanAutomate);
    value[ID::splitPhaseOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "splitPhaseOffset",
      Info::kCanAutomate | Info::kIsWrapAround);
    value[ID::splitSkew] = std::make_unique<LinearValue>(
      0.0, Scales::splitSkew, "splitSkew", Info::kCanAutomate);

    value[ID::refreshMatrix] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "refreshMatrix", Info::kCanAutomate);

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
