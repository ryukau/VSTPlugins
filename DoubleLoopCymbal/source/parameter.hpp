// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr uint32_t nAllpass = 16;

constexpr size_t nReservedParameter = 64;
constexpr size_t nReservedGuiParameter = 16;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  overSampling,
  resetSeedAtNoteOn,
  release,

  spreaderSpread,
  spreaderSplitHz,

  useExternalInput,
  externalInputGain,

  notePitchAmount,
  transposeSemitone,
  pitchBend,
  pitchBendRange,
  noteSlideTimeSecond,

  seed,
  impactTextureMix,
  impactGain,
  impactDecaySeconds,
  impactHighpassHz,

  halfClosedGain,
  halfClosedDecaySecond,
  halfClosedSustainLevel,
  halfClosedPulseSecond,
  halfClosedDensityHz,
  halfClosedHighpassHz,

  closingGain,
  closingAttackSecond,
  closingReleaseRatio,
  closingHighpassHz,

  delayTimeRandomRatio,
  delayTimeLoopRatio,
  delayTimeModAmount,
  allpassDelayCount1,
  allpassDelayCount2,
  allpassFeed1,
  allpassFeed2,
  allpassMixSpike,
  allpassMixAltSign,

  highShelfFrequencyHz,
  highShelfGain,
  lowShelfFrequencyHz,
  lowShelfGain,

  velocityToImpactGain,
  velocityToImpactHighpass,
  velocityToHalfClosedDensity,
  velocityToHalfClosedHighpass,
  velocityToDelayTimeMod,

  useNoteOffVelocityForClosing,
  noteOffVelocityToClosingGain,
  noteOffVelocityToClosingReleaseRatio,
  noteOffVelocityToClosingHighpass,

  reservedParameter0,
  reservedGuiParameter0 = reservedParameter0 + nReservedParameter,

  ID_ENUM_LENGTH = reservedGuiParameter0 + nReservedGuiParameter,
  // ID_ENUM_GUI_START = reservedGuiParameter0,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::LinearScale<double> semitone;
  static SomeDSP::DecibelScale<double> noteSlideTimeSecond;

  static SomeDSP::DecibelScale<double> noiseDecaySeconds;
  static SomeDSP::DecibelScale<double> noiseGain;
  static SomeDSP::DecibelScale<double> halfClosedDensityHz;
  static SomeDSP::DecibelScale<double> closingReleaseRatio;

  static SomeDSP::DecibelScale<double> delayTimeModAmount;

  static SomeDSP::UIntScale<double> allpassDelayCount;

  static SomeDSP::DecibelScale<double> cutoffFrequencyHz;
  static SomeDSP::DecibelScale<double> shelvingGain;

  static SomeDSP::LinearScale<double> velocityRangeDecibel;
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
    using NegativeDoubleExpValue = DoubleValue<SomeDSP::NegativeDoubleExpScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);
    value[ID::resetSeedAtNoteOn] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "resetSeedAtNoteOn", Info::kCanAutomate);
    value[ID::release]
      = std::make_unique<UIntValue>(0, Scales::boolScale, "release", Info::kCanAutomate);

    value[ID::spreaderSpread] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.5), Scales::defaultScale, "spreaderSpread",
      Info::kCanAutomate);
    value[ID::spreaderSplitHz] = std::make_unique<DecibelValue>(
      Scales::cutoffFrequencyHz.invmap(500.0), Scales::cutoffFrequencyHz,
      "spreaderSplitHz", Info::kCanAutomate);

    value[ID::useExternalInput] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "useExternalInput", Info::kCanAutomate);
    value[ID::externalInputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "externalInputGain", Info::kCanAutomate);

    value[ID::notePitchAmount] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "notePitchAmount",
      Info::kCanAutomate);
    value[ID::transposeSemitone] = std::make_unique<LinearValue>(
      Scales::semitone.invmap(0.0), Scales::semitone, "transposeSemitone",
      Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::semitone.invmap(12.0), Scales::semitone, "pitchBendRange",
      Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.0001), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::impactTextureMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.666), Scales::defaultScale, "impactTextureMix",
      Info::kCanAutomate);
    value[ID::impactGain] = std::make_unique<DecibelValue>(
      Scales::noiseGain.invmap(0.25), Scales::noiseGain, "impactGain",
      Info::kCanAutomate);
    value[ID::impactDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(0.025), Scales::noiseDecaySeconds,
      "impactDecaySeconds", Info::kCanAutomate);
    value[ID::impactHighpassHz] = std::make_unique<DecibelValue>(
      Scales::halfClosedDensityHz.invmap(400), Scales::halfClosedDensityHz,
      "impactHighpassHz", Info::kCanAutomate);

    value[ID::halfClosedGain] = std::make_unique<DecibelValue>(
      Scales::noiseGain.invmapDB(-20), Scales::noiseGain, "halfClosedGain",
      Info::kCanAutomate);
    value[ID::halfClosedDecaySecond] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(1.0), Scales::noiseDecaySeconds,
      "halfCloseDecaySecond", Info::kCanAutomate);
    value[ID::halfClosedSustainLevel] = std::make_unique<DecibelValue>(
      Scales::noiseGain.invmapDB(-20), Scales::noiseGain, "halfCloseSustainLevel",
      Info::kCanAutomate);
    value[ID::halfClosedPulseSecond] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(0.01), Scales::noiseDecaySeconds,
      "halfClosedPulseSecond", Info::kCanAutomate);
    value[ID::halfClosedDensityHz] = std::make_unique<DecibelValue>(
      Scales::halfClosedDensityHz.invmap(100), Scales::halfClosedDensityHz,
      "halfClosedDensityHz", Info::kCanAutomate);
    value[ID::halfClosedHighpassHz] = std::make_unique<DecibelValue>(
      Scales::halfClosedDensityHz.invmap(3000), Scales::halfClosedDensityHz,
      "halfClosedHighpassHz", Info::kCanAutomate);

    value[ID::closingGain] = std::make_unique<DecibelValue>(
      Scales::noiseGain.invmapDB(-30), Scales::noiseGain, "closingGain",
      Info::kCanAutomate);
    value[ID::closingAttackSecond] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(0.025), Scales::noiseDecaySeconds,
      "closingAttackSecond", Info::kCanAutomate);
    value[ID::closingReleaseRatio] = std::make_unique<DecibelValue>(
      Scales::closingReleaseRatio.invmap(30.0001), Scales::closingReleaseRatio,
      "closingReleaseRatio", Info::kCanAutomate);
    value[ID::closingHighpassHz] = std::make_unique<DecibelValue>(
      Scales::halfClosedDensityHz.invmap(100), Scales::halfClosedDensityHz,
      "closingHighpassHz", Info::kCanAutomate);

    value[ID::delayTimeRandomRatio] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.5), Scales::defaultScale, "delayTimeRandomRatio",
      Info::kCanAutomate);
    value[ID::delayTimeLoopRatio] = std::make_unique<LinearValue>(
      Scales::semitone.invmap(0.0), Scales::semitone, "delayTimeLoopRatio",
      Info::kCanAutomate);
    value[ID::delayTimeModAmount] = std::make_unique<DecibelValue>(
      Scales::delayTimeModAmount.invmap(0.0), Scales::delayTimeModAmount,
      "delayTimeModAmount", Info::kCanAutomate);

    value[ID::allpassDelayCount1] = std::make_unique<UIntValue>(
      Scales::allpassDelayCount.getMax(), Scales::allpassDelayCount, "allpassDelayCount1",
      Info::kCanAutomate);
    value[ID::allpassDelayCount2] = std::make_unique<UIntValue>(
      Scales::allpassDelayCount.getMax(), Scales::allpassDelayCount, "allpassDelayCount2",
      Info::kCanAutomate);
    value[ID::allpassFeed1] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.98), Scales::bipolarScale, "allpassFeed1",
      Info::kCanAutomate);
    value[ID::allpassFeed2] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(-0.5), Scales::bipolarScale, "allpassFeed2",
      Info::kCanAutomate);
    value[ID::allpassMixSpike] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(2.0 / 3.0), Scales::defaultScale, "allpassMixSpike",
      Info::kCanAutomate);
    value[ID::allpassMixAltSign] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "allpassMixAltSign",
      Info::kCanAutomate);

    value[ID::highShelfFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::cutoffFrequencyHz.invmap(12000.0), Scales::cutoffFrequencyHz,
      "highShelfFrequencyHz", Info::kCanAutomate);
    value[ID::highShelfGain] = std::make_unique<DecibelValue>(
      Scales::shelvingGain.invmapDB(-0.5), Scales::shelvingGain, "highShelfGain",
      Info::kCanAutomate);
    value[ID::lowShelfFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::cutoffFrequencyHz.invmap(20.0), Scales::cutoffFrequencyHz,
      "lowShelfFrequencyHz", Info::kCanAutomate);
    value[ID::lowShelfGain] = std::make_unique<DecibelValue>(
      Scales::shelvingGain.invmapDB(-1.0), Scales::shelvingGain, "lowShelfGain",
      Info::kCanAutomate);

    value[ID::velocityToImpactGain] = std::make_unique<LinearValue>(
      Scales::velocityRangeDecibel.invmap(-60.0), Scales::velocityRangeDecibel,
      "velocityToImpactGain", Info::kCanAutomate);
    value[ID::velocityToImpactHighpass] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.333), Scales::bipolarScale,
      "velocityToImpactHighpass", Info::kCanAutomate);
    value[ID::velocityToHalfClosedDensity] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.25), Scales::bipolarScale,
      "velocityToHalfClosedDensity", Info::kCanAutomate);
    value[ID::velocityToHalfClosedHighpass] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.4), Scales::bipolarScale,
      "velocityToHalfClosedHighpass", Info::kCanAutomate);
    value[ID::velocityToDelayTimeMod] = std::make_unique<DecibelValue>(
      Scales::delayTimeModAmount.invmap(0.5), Scales::delayTimeModAmount,
      "velocityToDelayTimeMod", Info::kCanAutomate);

    value[ID::useNoteOffVelocityForClosing] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "useNoteOffVelocityForClosing", Info::kCanAutomate);
    value[ID::noteOffVelocityToClosingGain] = std::make_unique<LinearValue>(
      Scales::velocityRangeDecibel.invmap(-60.0), Scales::velocityRangeDecibel,
      "noteOffVelocityToClosingGain", Info::kCanAutomate);
    value[ID::noteOffVelocityToClosingReleaseRatio] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
      "noteOffVelocityToClosingReleaseRatio", Info::kCanAutomate);
    value[ID::noteOffVelocityToClosingHighpass] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
      "noteOffVelocityToClosingHighpass", Info::kCanAutomate);

    for (size_t idx = 0; idx < nReservedParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedParameter" + indexStr).c_str(), Info::kIsHidden);
    }

    for (size_t idx = 0; idx < nReservedGuiParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedGuiParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedGuiParameter" + indexStr).c_str(), Info::kIsHidden);
    }

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
