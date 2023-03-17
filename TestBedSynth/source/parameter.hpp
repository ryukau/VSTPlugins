// (c) 2023 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"
#include "dsp/modulationenum.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr size_t nLfoWavetable = 64;
constexpr static size_t maxVoice = 4;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  osc0Waveform0,
  osc1Waveform0 = osc0Waveform0 + nOscWavetable,

  oscMix = osc1Waveform0 + nOscWavetable,

  oscPitchSemitone0,
  sumMix0 = oscPitchSemitone0 + nOscillator,
  feedbackLowpassHz0 = sumMix0 + nOscillator,
  hardSync0 = feedbackLowpassHz0 + nOscillator,
  spectralSpread0 = hardSync0 + nOscillator,
  phaseSlope0 = spectralSpread0 + nOscillator,
  spectralLowpass0 = phaseSlope0 + nOscillator,
  spectralHighpass0 = spectralLowpass0 + nOscillator,

  modPitch0 = spectralHighpass0 + nOscillator,
  modFeedbackMix0 = modPitch0 + nModulation,
  modImmediatePm0 = modFeedbackMix0 + nModulation,
  modAccumulatePm0 = modImmediatePm0 + nModulation,
  modFm0 = modAccumulatePm0 + nModulation,
  modHardSync0 = modFm0 + nModulation,
  modSpectralSpread0 = modHardSync0 + nModulation,
  modPhaseSlope0 = modSpectralSpread0 + nModulation,
  modSpectralLowpass0 = modPhaseSlope0 + nModulation,
  modSpectralHighpass0 = modSpectralLowpass0 + nModulation,

  sumToImmediatePm0 = modSpectralHighpass0 + nModulation,
  sumToAccumulatePm0 = sumToImmediatePm0 + nOscillator,
  sumToFm0 = sumToAccumulatePm0 + nOscillator,

  envelopeAttackSecond0 = sumToFm0 + nOscillator,
  envelopeDecaySecond0 = envelopeAttackSecond0 + nOscillator,
  envelopeSustainAmplitude0 = envelopeDecaySecond0 + nOscillator,
  envelopeReleaseSecond0 = envelopeSustainAmplitude0 + nOscillator,

  lfoRate0 = envelopeReleaseSecond0 + nOscillator,
  lfo0Waveform0 = lfoRate0 + nOscillator,
  lfo1Waveform0 = lfo0Waveform0 + nLfoWavetable,

  gain = lfo1Waveform0 + nLfoWavetable,
  gainAttackSecond,
  gainDecaySecond,
  gainSustainAmplitude,
  gainReleaseSecond,

  dcHighpassEnable,
  dcHighpassCutoffHz,

  octave,
  semitone,
  milli,
  equalTemperament,
  pitchA4Hz,
  pitchBend,
  pitchBendRange,

  parameterSmoothingSecond,
  oversampling,
  nVoice,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::LinearScale<double> waveform;

  static SomeDSP::LinearScale<double> oscPitchSemitone;
  static SomeDSP::DecibelScale<double> hardSync;

  static SomeDSP::DecibelScale<double> spectralSpread;
  static SomeDSP::DecibelScale<double> phaseSlope;
  static SomeDSP::DecibelScale<double> spectralFilterCutoff;

  static SomeDSP::LinearScale<double> modSpectralFilterCutoff;

  static SomeDSP::BipolarDecibelScale<double> sumToPm;
  static SomeDSP::LinearScale<double> sumToFm;

  static SomeDSP::DecibelScale<double> envelopeSecond;
  static SomeDSP::DecibelScale<double> envelopeSustainAmplitude;

  static SomeDSP::DecibelScale<double> lfoRate;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> cutoffHz;

  static SomeDSP::UIntScale<double> octave;
  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::UIntScale<double> milli;
  static SomeDSP::UIntScale<double> equalTemperament;
  static SomeDSP::UIntScale<double> pitchA4Hz;
  static SomeDSP::LinearScale<double> pitchBendRange;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;
  static SomeDSP::UIntScale<double> oversampling;
  static SomeDSP::UIntScale<double> nVoice;
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
    using BipolarDecibelValue = DoubleValue<SomeDSP::BipolarDecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    for (size_t idx = 0; idx < nOscWavetable; ++idx) {
      auto indexStr = std::to_string(idx);

      value[ID::osc0Waveform0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(
          std::sin(SomeDSP::twopi * idx / double(nOscWavetable))),
        Scales::bipolarScale, ("lfo0Waveform" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::osc1Waveform0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(
          std::sin(SomeDSP::twopi * idx / double(nOscWavetable))),
        Scales::bipolarScale, ("lfo1Waveform" + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::oscMix] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "oscMix", Info::kCanAutomate);

    for (size_t idx = 0; idx < nOscillator; ++idx) {
      auto indexStr = std::to_string(idx);

      value[ID::oscPitchSemitone0 + idx] = std::make_unique<LinearValue>(
        Scales::oscPitchSemitone.invmap(0.0), Scales::oscPitchSemitone,
        ("oscPitchSemitone" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::sumMix0 + idx] = std::make_unique<LinearValue>(
        0.5, Scales::defaultScale, ("sumMix" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::feedbackLowpassHz0 + idx] = std::make_unique<DecibelValue>(
        Scales::cutoffHz.invmap(100.0), Scales::cutoffHz,
        ("feedbackLowpassHz" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::hardSync0 + idx] = std::make_unique<DecibelValue>(
        Scales::hardSync.invmap(1.0), Scales::hardSync, ("hardSync" + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::spectralSpread0 + idx] = std::make_unique<DecibelValue>(
        Scales::spectralSpread.invmap(1.0), Scales::spectralSpread,
        ("spectralSpread" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::phaseSlope0 + idx] = std::make_unique<DecibelValue>(
        0.0, Scales::phaseSlope, ("phaseSlope" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::spectralLowpass0 + idx] = std::make_unique<DecibelValue>(
        1.0, Scales::spectralFilterCutoff, ("spectralLowpass" + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::spectralHighpass0 + idx] = std::make_unique<DecibelValue>(
        0.0, Scales::spectralFilterCutoff, ("spectralHighpass" + indexStr).c_str(),
        Info::kCanAutomate);
    }

    std::array<std::string, ModID::MODID_ENUM_LENGTH> modSourceName{
      "env0To", "env1To", "lfo0To", "lfo1To", "ext0To", "ext1To"};
    for (size_t idx = 0; idx < nModulation; ++idx) {
      const auto indexStr = std::to_string(idx / ModID::MODID_ENUM_LENGTH);
      const auto &prefix = modSourceName[idx % ModID::MODID_ENUM_LENGTH];

      value[ID::modPitch0 + idx] = std::make_unique<LinearValue>(
        Scales::sumToFm.invmap(0.0), Scales::sumToFm,
        (prefix + "Pitch" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modFeedbackMix0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
        (prefix + "FeedbackMix" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modImmediatePm0 + idx] = std::make_unique<BipolarDecibelValue>(
        Scales::sumToPm.invmap(0.0), Scales::sumToPm,
        (prefix + "ImmediatePm" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modAccumulatePm0 + idx] = std::make_unique<BipolarDecibelValue>(
        Scales::sumToPm.invmap(0.0), Scales::sumToPm,
        (prefix + "AccumulatePm" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modFm0 + idx] = std::make_unique<LinearValue>(
        Scales::sumToFm.invmap(0.0), Scales::sumToFm, (prefix + "Fm" + indexStr).c_str(),
        Info::kCanAutomate);
      value[ID::modHardSync0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
        (prefix + "HardSync" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modSpectralSpread0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
        (prefix + "SpectralSpread" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modPhaseSlope0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
        (prefix + "PhaseSlope" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modSpectralLowpass0 + idx] = std::make_unique<LinearValue>(
        Scales::modSpectralFilterCutoff.invmap(0.0), Scales::modSpectralFilterCutoff,
        (prefix + "SpectralLowpass" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::modSpectralHighpass0 + idx] = std::make_unique<LinearValue>(
        Scales::modSpectralFilterCutoff.invmap(0.0), Scales::modSpectralFilterCutoff,
        (prefix + "SpectralHighpass" + indexStr).c_str(), Info::kCanAutomate);
    }

    for (size_t idx = 0; idx < nOscillator; ++idx) {
      auto indexStr = std::to_string(idx);

      value[ID::sumToImmediatePm0 + idx] = std::make_unique<BipolarDecibelValue>(
        Scales::sumToPm.invmap(0.0), Scales::sumToPm,
        ("sumToImmediatePm" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::sumToAccumulatePm0 + idx] = std::make_unique<BipolarDecibelValue>(
        Scales::sumToPm.invmap(0.0), Scales::sumToPm,
        ("sumToAccumulatePm" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::sumToFm0 + idx] = std::make_unique<LinearValue>(
        Scales::sumToFm.invmap(0.0), Scales::sumToFm, ("sumToFm" + indexStr).c_str(),
        Info::kCanAutomate);

      value[ID::envelopeAttackSecond0 + idx] = std::make_unique<DecibelValue>(
        Scales::envelopeSecond.invmap(0.002), Scales::envelopeSecond,
        ("envelopeAttackSecond" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::envelopeDecaySecond0 + idx] = std::make_unique<DecibelValue>(
        Scales::envelopeSecond.invmap(1.0), Scales::envelopeSecond,
        ("envelopeDecaySecond" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::envelopeSustainAmplitude0 + idx] = std::make_unique<DecibelValue>(
        Scales::envelopeSustainAmplitude.invmap(0.25), Scales::envelopeSustainAmplitude,
        ("envelopeSustainAmplitude" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::envelopeReleaseSecond0 + idx] = std::make_unique<DecibelValue>(
        1.0, Scales::envelopeSecond, ("envelopeReleaseSecond" + indexStr).c_str(),
        Info::kCanAutomate);

      value[ID::lfoRate0 + idx] = std::make_unique<DecibelValue>(
        Scales::lfoRate.invmap(1.0), Scales::lfoRate, ("lfoRate" + indexStr).c_str(),
        Info::kCanAutomate);
    }

    for (size_t idx = 0; idx < nLfoWavetable; ++idx) {
      auto indexStr = std::to_string(idx);

      value[ID::lfo0Waveform0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(
          std::sin(SomeDSP::twopi * idx / double(nLfoWavetable))),
        Scales::bipolarScale, ("lfo0Waveform" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::lfo1Waveform0 + idx] = std::make_unique<LinearValue>(
        Scales::bipolarScale.invmap(
          std::sin(SomeDSP::twopi * idx / double(nLfoWavetable))),
        Scales::bipolarScale, ("lfo1Waveform" + indexStr).c_str(), Info::kCanAutomate);
    }

    value[ID::gain]
      = std::make_unique<DecibelValue>(0.5, Scales::gain, "gain", Info::kCanAutomate);
    value[ID::gainAttackSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.002), Scales::envelopeSecond, "gainAttackSecond",
      Info::kCanAutomate);
    value[ID::gainDecaySecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(1.0), Scales::envelopeSecond, "gainDecaySecond",
      Info::kCanAutomate);
    value[ID::gainSustainAmplitude] = std::make_unique<DecibelValue>(
      Scales::envelopeSustainAmplitude.invmap(0.25), Scales::envelopeSustainAmplitude,
      "gainSustainAmplitude", Info::kCanAutomate);
    value[ID::gainReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::envelopeSecond.invmap(0.002), Scales::envelopeSecond, "gainReleaseSecond",
      Info::kCanAutomate);

    value[ID::dcHighpassEnable] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "dcHighpassEnable", Info::kCanAutomate);
    value[ID::dcHighpassCutoffHz] = std::make_unique<DecibelValue>(
      Scales::cutoffHz.invmap(5.0), Scales::cutoffHz, "dcHighpassCutoffHz",
      Info::kCanAutomate);

    value[ID::octave]
      = std::make_unique<UIntValue>(12, Scales::octave, "octave", Info::kCanAutomate);
    value[ID::semitone] = std::make_unique<UIntValue>(
      120, Scales::semitone, "semitone", Info::kCanAutomate);
    value[ID::milli]
      = std::make_unique<UIntValue>(1000, Scales::milli, "milli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<UIntValue>(
      11, Scales::equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<UIntValue>(
      340, Scales::pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::bipolarScale, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::pitchBendRange.invmap(24.0), Scales::pitchBendRange, "pitchBendRange",
      Info::kCanAutomate);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.01), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);
    value[ID::oversampling] = std::make_unique<UIntValue>(
      0, Scales::oversampling, "oversampling", Info::kCanAutomate);
    value[ID::nVoice] = std::make_unique<UIntValue>(
      Scales::nVoice.getMax(), Scales::nVoice, "nVoice", Info::kCanAutomate);

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
