// (c) 2021 Takamitsu Endo
//
// This file is part of MatrixShifter.
//
// MatrixShifter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixShifter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixShifter.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr float maxShiftDelaySeconds = 0.03f;

constexpr float minFeedbackCutoffNote = 3.4868205763524287f; // 10 Hz in midi note.
constexpr float maxFeedbackCutoffNote = 62.36950772365466f;  // 300 Hz in midi note.

constexpr size_t nParallel = 4;
constexpr size_t nSerial = 4;
constexpr size_t nShifter = nParallel * nSerial;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  shiftSemi0,
  shiftDelay0 = 1 + nShifter,
  shiftGain0 = 1 + nShifter + nSerial,

  lfoHz = 1 + nShifter + 2 * nSerial,
  lfoAmount,
  lfoSkew,
  lfoShiftOffset,
  shiftMix,
  shiftPhase,
  shiftFeedbackGain,
  shiftSemiMultiplier,
  gain,
  smoothness,

  invertEachSection,
  shiftFeedbackCutoff,
  lfoToFeedbackCutoff,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LinearScale<double> shiftSemi;
  static SomeDSP::LogScale<double> shiftDelay;
  static SomeDSP::DecibelScale<double> shiftGain;
  static SomeDSP::LinearScale<double> shiftPhase;
  static SomeDSP::DecibelScale<double> shiftFeedbackGain;
  static SomeDSP::SemitoneScale<double> shiftFeedbackCutoff;
  static SomeDSP::LogScale<double> shiftSemiMultiplier;

  static SomeDSP::SemitoneScale<double> lfoHz;
  static SomeDSP::LinearScale<double> lfoShiftOffset;
  static SomeDSP::LinearScale<double> lfoToFeedbackCutoff;
  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::LogScale<double> smoothness;
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

    std::string shiftHzLabel("shiftHz");
    for (size_t idx = 0; idx < nShifter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::shiftSemi0 + idx] = std::make_unique<LinearValue>(
        Scales::shiftSemi.invmap(0.0), Scales::shiftSemi,
        (shiftHzLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    std::string delayLabel("delay");
    std::string gainLabel("gain");
    for (size_t idx = 0; idx < nSerial; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::shiftDelay0 + idx] = std::make_unique<LogValue>(
        Scales::shiftDelay.invmap(0.0), Scales::shiftDelay,
        (shiftHzLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::shiftGain0 + idx] = std::make_unique<DecibelValue>(
        Scales::shiftGain.invmap(1.0), Scales::shiftGain,
        (shiftHzLabel + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::lfoHz]
      = std::make_unique<SemitoneValue>(0.5, Scales::lfoHz, "lfoHz", Info::kCanAutomate);
    value[ID::lfoAmount] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoAmount", Info::kCanAutomate);
    value[ID::lfoSkew] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoSkew", Info::kCanAutomate);
    value[ID::lfoShiftOffset] = std::make_unique<LinearValue>(
      0.5, Scales::lfoShiftOffset, "lfoShiftOffset", Info::kCanAutomate);

    value[ID::shiftMix] = std::make_unique<LinearValue>(
      0.75, Scales::defaultScale, "shiftMix", Info::kCanAutomate);
    value[ID::shiftPhase] = std::make_unique<LinearValue>(
      0.5, Scales::shiftPhase, "shiftPhase", Info::kCanAutomate);
    value[ID::shiftFeedbackGain] = std::make_unique<DecibelValue>(
      Scales::shiftFeedbackGain.invmap(0.0), Scales::shiftFeedbackGain,
      "shiftFeedbackGain", Info::kCanAutomate);
    value[ID::shiftSemiMultiplier] = std::make_unique<LogValue>(
      Scales::shiftSemiMultiplier.invmap(0.02), Scales::shiftSemiMultiplier,
      "shiftSemiMultiplier", Info::kCanAutomate);

    value[ID::gain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "gain", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      Scales::smoothness.invmap(0.35), Scales::smoothness, "smoothness",
      Info::kCanAutomate);
    value[ID::invertEachSection] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "invertEachSection", Info::kCanAutomate);

    value[ID::shiftFeedbackCutoff] = std::make_unique<SemitoneValue>(
      Scales::shiftFeedbackCutoff.invmap(40.0), Scales::shiftFeedbackCutoff,
      "shiftFeedbackCutoff", Info::kCanAutomate);
    value[ID::lfoToFeedbackCutoff] = std::make_unique<LinearValue>(
      Scales::lfoToFeedbackCutoff.invmap(0.0), Scales::lfoToFeedbackCutoff,
      "lfoToFeedbackCutoff", Info::kCanAutomate);

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
