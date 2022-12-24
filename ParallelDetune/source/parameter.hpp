// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelDetune.
//
// ParallelDetune is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelDetune is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelDetune.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t nShifter = 8; // Must be even.
constexpr size_t nLfoWavetable = 64;
constexpr double maxDelayTime = 1.0;

namespace ParameterID {
enum ID {
  bypass,

  dryGain,
  wetGain,

  panSpread,
  lfoToPan,
  tremoloMix,
  tremoloLean,

  feed,
  delayTimeSeconds,
  shiftTransposeSemitone,
  shiftSemitone0,
  shiftFineTuningCent0 = shiftSemitone0 + nShifter,
  shifterGain0 = shiftFineTuningCent0 + nShifter,
  shifterDelayTimeMultiplier0 = shifterGain0 + nShifter,
  shifterHighpassOffset0 = shifterDelayTimeMultiplier0 + nShifter,
  shifterLowpassOffset0 = shifterHighpassOffset0 + nShifter,
  highpassHz = shifterLowpassOffset0 + nShifter,
  lowpassNormalizedCutoff,

  lfoToDelayTime,
  lfoToShiftPitch,

  lfoWavetable0,
  lfoInterpolation = lfoWavetable0 + nLfoWavetable,
  lfoTempoSync,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoPhaseOffset,
  lfoPhaseConstant,

  parameterSmoothingSecond,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> tremoloMix;
  static SomeDSP::DecibelScale<double> delayTimeSeconds;
  static SomeDSP::DecibelScale<double> delayTimeMultiplier;
  static SomeDSP::LinearScale<double> shiftOctave;
  static SomeDSP::LinearScale<double> shiftSemitone;
  static SomeDSP::LinearScale<double> shiftFineTuningCent;
  static SomeDSP::DecibelScale<double> cutoffHz;
  static SomeDSP::DecibelScale<double> normalizedCutoff;
  static SomeDSP::LinearScale<double> lfoOctaveAmount;

  static SomeDSP::UIntScale<double> lfoInterpolation;
  static SomeDSP::UIntScale<double> lfoTempoUpper;
  static SomeDSP::UIntScale<double> lfoTempoLower;
  static SomeDSP::DecibelScale<double> lfoRate;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
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

    value[ID::dryGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "dryGain", Info::kCanAutomate);
    value[ID::wetGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "wetGain", Info::kCanAutomate);

    value[ID::panSpread] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "panSpread", Info::kCanAutomate);
    value[ID::lfoToPan] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoToPan", Info::kCanAutomate);
    value[ID::tremoloMix] = std::make_unique<DecibelValue>(
      0.0, Scales::tremoloMix, "tremoloMix", Info::kCanAutomate);
    value[ID::tremoloLean] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "tremoloLean", Info::kCanAutomate);

    value[ID::feed] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "feed", Info::kCanAutomate);
    value[ID::delayTimeSeconds] = std::make_unique<DecibelValue>(
      Scales::delayTimeSeconds.invmap(0.1), Scales::delayTimeSeconds, "delayTimeSeconds",
      Info::kCanAutomate);
    value[ID::shiftTransposeSemitone] = std::make_unique<LinearValue>(
      Scales::shiftSemitone.invmap(0.0), Scales::shiftSemitone, "shiftTransposeSemitone",
      Info::kCanAutomate);
    std::string shiftSemitoneLabel("shiftSemitone");
    std::string shiftFineTuningCentLabel("shiftFineTuningCent");
    std::string shifterDelayTimeOffsetLabel("shifterDelayTimeOffset");
    std::string shifterGainLabel("shifterGain");
    std::string shifterHighpassOffsetLabel("shifterHighpassOffset");
    std::string shifterLowpassOffsetLabel("shifterLowpassOffset");
    for (size_t idx = 0; idx < nShifter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::shiftSemitone0 + idx] = std::make_unique<LinearValue>(
        Scales::shiftSemitone.invmap(0.0), Scales::shiftSemitone,
        (shiftSemitoneLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::shiftFineTuningCent0 + idx] = std::make_unique<LinearValue>(
        Scales::shiftFineTuningCent.invmap(0.0), Scales::shiftFineTuningCent,
        (shiftFineTuningCentLabel + indexStr).c_str(), Info::kCanAutomate);

      value[ID::shifterGain0 + idx] = std::make_unique<DecibelValue>(
        Scales::tremoloMix.invmap(1.0), Scales::tremoloMix,
        (shifterGainLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::shifterDelayTimeMultiplier0 + idx] = std::make_unique<DecibelValue>(
        Scales::delayTimeMultiplier.invmapDB(0.0), Scales::delayTimeMultiplier,
        (shifterDelayTimeOffsetLabel + indexStr).c_str(), Info::kCanAutomate);

      value[ID::shifterHighpassOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::shiftOctave.invmap(0.0), Scales::shiftOctave,
        (shifterHighpassOffsetLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::shifterLowpassOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::shiftOctave.invmap(0.0), Scales::shiftOctave,
        (shifterLowpassOffsetLabel + indexStr).c_str(), Info::kCanAutomate);
    }
    value[ID::highpassHz] = std::make_unique<DecibelValue>(
      0.0, Scales::cutoffHz, "highpassHz", Info::kCanAutomate);
    value[ID::lowpassNormalizedCutoff] = std::make_unique<DecibelValue>(
      1.0, Scales::normalizedCutoff, "lowpassNormalizedCutoff", Info::kCanAutomate);

    value[ID::lfoToDelayTime] = std::make_unique<LinearValue>(
      0.5, Scales::lfoOctaveAmount, "lfoToDelayTime", Info::kCanAutomate);
    value[ID::lfoToShiftPitch] = std::make_unique<LinearValue>(
      0.5, Scales::lfoOctaveAmount, "lfoToShiftPitch", Info::kCanAutomate);

    std::string lfoWavetableLabel("lfoWavetable");
    for (size_t idx = 0; idx < nLfoWavetable; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::lfoWavetable0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(sin(SomeDSP::twopi * idx / double(nLfoWavetable))),
        Scales::bipolarScale, (lfoWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
    }
    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoInterpolation] = std::make_unique<UIntValue>(
      2, Scales::lfoInterpolation, "lfoInterpolation", Info::kCanAutomate);
    value[ID::lfoTempoSync] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "lfoTempoSync", Info::kCanAutomate);
    value[ID::lfoTempoUpper] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoUpper, "lfoTempoUpper", Info::kCanAutomate);
    value[ID::lfoTempoLower] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoLower, "lfoTempoLower", Info::kCanAutomate);
    value[ID::lfoRate] = std::make_unique<DecibelValue>(
      Scales::lfoRate.invmap(1.0), Scales::lfoRate, "lfoRate", Info::kCanAutomate);
    value[ID::lfoPhaseOffset] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseOffset", Info::kCanAutomate);
    value[ID::lfoPhaseConstant] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "lfoPhaseConstant", Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.2), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);

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
