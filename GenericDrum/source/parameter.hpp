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

constexpr int octaveOffset = 8;
constexpr int semitoneOffset = 96;
constexpr size_t maxFdnSize = 5;

constexpr size_t nReservedParameter = 63;
constexpr size_t nReservedGuiParameter = 16;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  outputGain,
  safetyHighpassEnable,
  safetyHighpassHz,
  overSampling,
  normalizeGainWrtNoiseLowpassHz,
  resetSeedAtNoteOn,
  preventBlowUp,

  stereoUnison,
  stereoBalance,
  stereoMerge,

  useExternalInput,
  externalInputGain,
  useAutomaticTrigger,
  automaticTriggerThreshold,

  notePitchAmount,
  tuningSemitone,
  tuningCent,
  pitchBend,
  pitchBendRange,
  noteSlideTimeSecond,

  seed,
  noiseDecaySeconds,
  noiseLowpassHz,
  noiseAllpassMaxTimeHz,

  impactWireMix,
  membraneWireMix,
  wireFrequencyHz,
  wireDecaySeconds,
  wireDistance,
  wireCollisionTypeMix,

  crossFeedbackGain,
  crossFeedbackRatio0,

  delayTimeSpread = crossFeedbackRatio0 + maxFdnSize,
  bandpassCutSpread,
  pitchRandomCent,

  envelopeAttackSeconds,
  envelopeDecaySeconds,
  envelopeModAmount,

  pitchType,
  delayTimeHz,
  delayTimeModAmount,
  bandpassCutRatio,
  bandpassQ,

  secondaryFdnMix,
  secondaryPitchOffset,
  secondaryQOffset,
  secondaryDistance,

  crossFeedbackConsistency,

  reservedParameter0,

  externalInputAmplitudeMeter = reservedParameter0 + nReservedParameter,
  isWireCollided,
  isSecondaryCollided,

  reservedGuiParameter0,

  ID_ENUM_LENGTH = reservedGuiParameter0 + nReservedGuiParameter,
  ID_ENUM_GUI_START = externalInputAmplitudeMeter,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> safetyHighpassHz;

  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::LinearScale<double> pitchBendRange;
  static SomeDSP::DecibelScale<double> noteSlideTimeSecond;

  static SomeDSP::DecibelScale<double> noiseDecaySeconds;

  static SomeDSP::DecibelScale<double> wireFrequencyHz;
  static SomeDSP::DecibelScale<double> wireDecaySeconds;

  static SomeDSP::DecibelScale<double> crossFeedbackGain;
  static SomeDSP::DecibelScale<double> crossFeedbackConsistency;
  static SomeDSP::DecibelScale<double> feedbackDecaySeconds;

  static SomeDSP::LinearScale<double> pitchRandomCent;
  static SomeDSP::DecibelScale<double> envelopeSeconds;
  static SomeDSP::DecibelScale<double> envelopeModAmount;

  static SomeDSP::UIntScale<double> pitchType;
  static SomeDSP::DecibelScale<double> delayTimeHz;
  static SomeDSP::DecibelScale<double> delayTimeModAmount;
  static SomeDSP::LinearScale<double> bandpassCutRatio;
  static SomeDSP::DecibelScale<double> bandpassQ;

  static SomeDSP::DecibelScale<double> collisionDistance;

  static SomeDSP::LinearScale<double> amplitudeMeter;
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
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::safetyHighpassEnable] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "safetyHighpassEnable", Info::kCanAutomate);
    value[ID::safetyHighpassHz] = std::make_unique<DecibelValue>(
      Scales::safetyHighpassHz.invmap(15.0), Scales::safetyHighpassHz, "safetyHighpassHz",
      Info::kCanAutomate);
    value[ID::overSampling] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "overSampling", Info::kCanAutomate);
    value[ID::normalizeGainWrtNoiseLowpassHz] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "normalizeGainWrtNoiseLowpassHz", Info::kCanAutomate);
    value[ID::resetSeedAtNoteOn] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "resetSeedAtNoteOn", Info::kCanAutomate);
    value[ID::preventBlowUp] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "preventBlowUp", Info::kCanAutomate);

    value[ID::stereoUnison] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "stereoUnison", Info::kCanAutomate);
    value[ID::stereoBalance] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "stereoBalance",
      Info::kCanAutomate);
    value[ID::stereoMerge] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.75), Scales::defaultScale, "stereoMerge",
      Info::kCanAutomate);

    value[ID::useExternalInput] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "useExternalInput", Info::kCanAutomate);
    value[ID::externalInputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "externalInputGain", Info::kCanAutomate);
    value[ID::useAutomaticTrigger] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "useAutomaticTrigger", Info::kCanAutomate);
    value[ID::automaticTriggerThreshold] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(0.1), Scales::gain, "automaticTriggerThreshold",
      Info::kCanAutomate);

    value[ID::notePitchAmount] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "notePitchAmount",
      Info::kCanAutomate);
    value[ID::tuningSemitone] = std::make_unique<UIntValue>(
      semitoneOffset, Scales::semitone, "tuningSemitone", Info::kCanAutomate);
    value[ID::tuningCent] = std::make_unique<LinearValue>(
      Scales::cent.invmap(0.0), Scales::cent, "tuningCent", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::pitchBendRange.invmap(2.0), Scales::pitchBendRange, "pitchBendRange",
      Info::kCanAutomate);
    value[ID::noteSlideTimeSecond] = std::make_unique<DecibelValue>(
      Scales::noteSlideTimeSecond.invmap(0.1), Scales::noteSlideTimeSecond,
      "noteSlideTimeSecond", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::noiseDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::noiseDecaySeconds.invmap(0.08), Scales::noiseDecaySeconds,
      "noiseDecaySeconds", Info::kCanAutomate);
    value[ID::noiseLowpassHz] = std::make_unique<DecibelValue>(
      Scales::delayTimeHz.invmap(200.0), Scales::delayTimeHz, "noiseLowpassHz",
      Info::kCanAutomate);
    value[ID::noiseAllpassMaxTimeHz] = std::make_unique<DecibelValue>(
      Scales::delayTimeHz.invmap(3000.0), Scales::delayTimeHz, "noiseAllpassMaxTimeHz",
      Info::kCanAutomate);

    value[ID::impactWireMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.75), Scales::defaultScale, "impactWireMix",
      Info::kCanAutomate);
    value[ID::membraneWireMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.0), Scales::defaultScale, "membraneWireMix",
      Info::kCanAutomate);
    value[ID::wireFrequencyHz] = std::make_unique<DecibelValue>(
      Scales::wireFrequencyHz.invmap(100.0), Scales::wireFrequencyHz, "wireFrequencyHz",
      Info::kCanAutomate);
    value[ID::wireDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::wireDecaySeconds.invmap(1.0), Scales::wireDecaySeconds, "wireDecaySeconds",
      Info::kCanAutomate);
    value[ID::wireDistance] = std::make_unique<DecibelValue>(
      Scales::collisionDistance.invmap(0.15), Scales::collisionDistance, "wireDistance",
      Info::kCanAutomate);
    value[ID::wireCollisionTypeMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.5), Scales::defaultScale, "wireCollisionTypeMix",
      Info::kCanAutomate);

    value[ID::crossFeedbackGain] = std::make_unique<DecibelValue>(
      Scales::crossFeedbackGain.invmap(0.9), Scales::crossFeedbackGain,
      "crossFeedbackGain", Info::kCanAutomate);
    for (size_t idx = 0; idx < maxFdnSize; ++idx) {
      auto indexStr = std::to_string(idx);

      value[ID::crossFeedbackRatio0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(0.0), Scales::defaultScale,
        ("crossFeedbackRatio" + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::delayTimeSpread] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.5), Scales::defaultScale, "delayTimeSpread",
      Info::kCanAutomate);
    value[ID::bandpassCutSpread] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.5), Scales::defaultScale, "bandpassCutSpread",
      Info::kCanAutomate);
    value[ID::pitchRandomCent] = std::make_unique<LinearValue>(
      Scales::pitchRandomCent.invmap(21.5), Scales::pitchRandomCent, "pitchRandomCent",
      Info::kCanAutomate);

    value[ID::envelopeAttackSeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.01), Scales::envelopeSeconds,
      "envelopeAttackSeconds", Info::kCanAutomate);
    value[ID::envelopeDecaySeconds] = std::make_unique<DecibelValue>(
      Scales::envelopeSeconds.invmap(0.01), Scales::envelopeSeconds,
      "envelopeDecaySeconds", Info::kCanAutomate);
    value[ID::envelopeModAmount] = std::make_unique<DecibelValue>(
      0, Scales::envelopeModAmount, "envelopeModAmount", Info::kCanAutomate);

    value[ID::pitchType] = std::make_unique<UIntValue>(
      0, Scales::pitchType, "pitchType", Info::kCanAutomate);
    value[ID::delayTimeHz] = std::make_unique<DecibelValue>(
      Scales::delayTimeHz.invmap(110.0), Scales::delayTimeHz, "delayTimeHz",
      Info::kCanAutomate);
    value[ID::delayTimeModAmount] = std::make_unique<DecibelValue>(
      Scales::delayTimeModAmount.invmap(0.0), Scales::delayTimeModAmount,
      "delayTimeModAmount", Info::kCanAutomate);
    value[ID::bandpassCutRatio] = std::make_unique<LinearValue>(
      Scales::bandpassCutRatio.invmap(0.0), Scales::bandpassCutRatio, "bandpassCutRatio",
      Info::kCanAutomate);
    value[ID::bandpassQ] = std::make_unique<DecibelValue>(
      Scales::bandpassQ.invmap(std::numbers::sqrt2_v<double> / double(2)),
      Scales::bandpassQ, "bandpassQ", Info::kCanAutomate);

    value[ID::secondaryFdnMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.0), Scales::defaultScale, "secondaryFdnMix",
      Info::kCanAutomate);
    value[ID::secondaryPitchOffset] = std::make_unique<LinearValue>(
      Scales::bandpassCutRatio.invmap(0.0), Scales::bandpassCutRatio,
      "secondaryPitchOffset", Info::kCanAutomate);
    value[ID::secondaryQOffset] = std::make_unique<LinearValue>(
      Scales::bandpassCutRatio.invmap(0), Scales::bandpassCutRatio, "secondaryQOffset",
      Info::kCanAutomate);
    value[ID::secondaryDistance] = std::make_unique<DecibelValue>(
      1.0, Scales::collisionDistance, "secondaryDistance", Info::kCanAutomate);

    value[ID::crossFeedbackConsistency] = std::make_unique<DecibelValue>(
      Scales::crossFeedbackConsistency.invmap(1.0), Scales::crossFeedbackConsistency,
      "crossFeedbackConsistency", Info::kCanAutomate);

    for (size_t idx = 0; idx < nReservedParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedParameter" + indexStr).c_str(), Info::kIsHidden);
    }

    value[ID::externalInputAmplitudeMeter] = std::make_unique<LinearValue>(
      Scales::amplitudeMeter.invmap(0.0), Scales::amplitudeMeter,
      "externalInputAmplitudeMeter", Info::kIsReadOnly);
    value[ID::isWireCollided] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "isWireCollided", Info::kIsReadOnly);
    value[ID::isSecondaryCollided] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "isSecondaryCollided", Info::kIsReadOnly);

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
