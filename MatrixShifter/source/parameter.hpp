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

#include "../../common/dsp/constants.hpp"
#include "../../common/value.hpp"

constexpr float maxShiftDelaySeconds = 0.03f;

constexpr float minFeedbackCutoffNote = 3.4868205763524287f; // 10 Hz in midi note.
constexpr float maxFeedbackCutoffNote = 62.36950772365466f;  // 300 Hz in midi note.

constexpr size_t nParallel = 4;
constexpr size_t nSerial = 4;
constexpr size_t nShifter = nParallel * nSerial;

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

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
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , shiftSemi(0.0, 10.0)
    , shiftDelay(0, maxShiftDelaySeconds, 0.5, 0.2 * maxShiftDelaySeconds)
    , shiftGain(-30, 0, true)
    , shiftPhase(-0.5, 0.5)
    , shiftFeedbackGain(-18.0, 0.0, true)
    , shiftFeedbackCutoff(minFeedbackCutoffNote, maxFeedbackCutoffNote, false)
    , shiftSemiMultiplier(0.0, 1.0, 0.5, 0.2)
    , lfoHz(freqToNote(0.01), freqToNote(20.0), true)
    , lfoShiftOffset(-1.0, 1.0)
    , lfoToFeedbackCutoff(-1.0, 1.0)
    , gain(-24, 24, true)
    , smoothness(0.04, 8.0, 0.5, 1.0)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LinearScale<double> shiftSemi;
  SomeDSP::LogScale<double> shiftDelay;
  SomeDSP::DecibelScale<double> shiftGain;
  SomeDSP::LinearScale<double> shiftPhase;
  SomeDSP::DecibelScale<double> shiftFeedbackGain;
  SomeDSP::SemitoneScale<double> shiftFeedbackCutoff;
  SomeDSP::LogScale<double> shiftSemiMultiplier;

  SomeDSP::SemitoneScale<double> lfoHz;
  SomeDSP::LinearScale<double> lfoShiftOffset;
  SomeDSP::LinearScale<double> lfoToFeedbackCutoff;
  SomeDSP::DecibelScale<double> gain;

  SomeDSP::LogScale<double> smoothness;
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
    using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;
    using SemitoneValue = FloatValue<SomeDSP::SemitoneScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    std::string shiftHzLabel("shiftHz");
    for (size_t idx = 0; idx < nShifter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::shiftSemi0 + idx] = std::make_unique<LinearValue>(
        scale.shiftSemi.invmap(0.0), scale.shiftSemi, (shiftHzLabel + indexStr).c_str(),
        Info::kCanAutomate);
    }

    std::string delayLabel("delay");
    std::string gainLabel("gain");
    for (size_t idx = 0; idx < nSerial; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::shiftDelay0 + idx] = std::make_unique<LogValue>(
        scale.shiftDelay.invmap(0.0), scale.shiftDelay, (shiftHzLabel + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::shiftGain0 + idx] = std::make_unique<DecibelValue>(
        scale.shiftGain.invmap(1.0), scale.shiftGain, (shiftHzLabel + indexStr).c_str(),
        Info::kCanAutomate);
    }

    value[ID::lfoHz]
      = std::make_unique<SemitoneValue>(0.5, scale.lfoHz, "lfoHz", Info::kCanAutomate);
    value[ID::lfoAmount] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "lfoAmount", Info::kCanAutomate);
    value[ID::lfoSkew] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "lfoSkew", Info::kCanAutomate);
    value[ID::lfoShiftOffset] = std::make_unique<LinearValue>(
      0.5, scale.lfoShiftOffset, "lfoShiftOffset", Info::kCanAutomate);

    value[ID::shiftMix] = std::make_unique<LinearValue>(
      0.75, scale.defaultScale, "shiftMix", Info::kCanAutomate);
    value[ID::shiftPhase] = std::make_unique<LinearValue>(
      0.5, scale.shiftPhase, "shiftPhase", Info::kCanAutomate);
    value[ID::shiftFeedbackGain] = std::make_unique<DecibelValue>(
      scale.shiftFeedbackGain.invmap(0.0), scale.shiftFeedbackGain, "shiftFeedbackGain",
      Info::kCanAutomate);
    value[ID::shiftSemiMultiplier] = std::make_unique<LogValue>(
      scale.shiftSemiMultiplier.invmap(0.02), scale.shiftSemiMultiplier,
      "shiftSemiMultiplier", Info::kCanAutomate);

    value[ID::gain] = std::make_unique<DecibelValue>(
      scale.gain.invmap(1.0), scale.gain, "gain", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      scale.smoothness.invmap(0.35), scale.smoothness, "smoothness", Info::kCanAutomate);
    value[ID::invertEachSection] = std::make_unique<UIntValue>(
      0, scale.boolScale, "invertEachSection", Info::kCanAutomate);

    value[ID::shiftFeedbackCutoff] = std::make_unique<SemitoneValue>(
      scale.shiftFeedbackCutoff.invmap(40.0), scale.shiftFeedbackCutoff,
      "shiftFeedbackCutoff", Info::kCanAutomate);
    value[ID::lfoToFeedbackCutoff] = std::make_unique<LinearValue>(
      scale.lfoToFeedbackCutoff.invmap(0.0), scale.lfoToFeedbackCutoff,
      "lfoToFeedbackCutoff", Info::kCanAutomate);

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
