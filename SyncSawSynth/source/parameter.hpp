// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <vector>

#include "../../common/parameterInterface.hpp"
#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

namespace ParameterID {
enum ID : Vst::ParamID {
  bypass,
  osc1Gain,
  osc1Semi,
  osc1Cent,
  osc1Sync,
  osc1SyncType,
  osc1PTROrder,
  osc1Phase,
  osc1PhaseLock,

  osc2Gain,
  osc2Semi,
  osc2Cent,
  osc2Sync,
  osc2SyncType,
  osc2PTROrder,
  osc2Invert,
  osc2Phase,
  osc2PhaseLock,

  fmOsc1ToSync1,
  fmOsc1ToFreq2,
  fmOsc2ToSync1,

  gain,
  gainA,
  gainD,
  gainS,
  gainR,
  gainEnvelopeCurve,

  filterCutoff,
  filterResonance,
  filterFeedback,
  filterSaturation,
  filterDirty,
  filterType,
  filterShaper,
  filterA,
  filterD,
  filterS,
  filterR,
  filterCutoffAmount,
  filterResonanceAmount,
  filterKeyToCutoff,
  filterKeyToFeedback,

  modEnvelopeA,
  modEnvelopeCurve,
  modEnvelopeToFreq1,
  modEnvelopeToSync1,
  modEnvelopeToFreq2,
  modEnvelopeToSync2,
  modLFOFrequency,
  modLFONoiseMix,
  modLFOToFreq1,
  modLFOToSync1,
  modLFOToFreq2,
  modLFOToSync2,

  pitchBend,
  unison,
  nVoice,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> oscGain;
  static SomeDSP::LinearScale<double> semi;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::LinearScale<double> sync;
  static SomeDSP::UIntScale<double> osc1SyncType;
  static SomeDSP::UIntScale<double> osc1PTROrder;

  static SomeDSP::LogScale<double> fmToSync;
  static SomeDSP::LogScale<double> fmToFreq;

  static SomeDSP::LogScale<double> gain;

  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::LogScale<double> envelopeS;
  static SomeDSP::LogScale<double> envelopeR;

  static SomeDSP::LogScale<double> filterCutoff;
  static SomeDSP::LogScale<double> filterResonance;
  static SomeDSP::LogScale<double> filterFeedback;
  static SomeDSP::LogScale<double> filterSaturation;
  static SomeDSP::UIntScale<double> filterType;
  static SomeDSP::UIntScale<double> filterShaper;
  static SomeDSP::LinearScale<double> filterCutoffAmount;
  static SomeDSP::LinearScale<double> filterKeyMod;

  static SomeDSP::LogScale<double> modEnvelopeA;
  static SomeDSP::LogScale<double> modEnvelopeCurve;
  static SomeDSP::LogScale<double> modLFOFrequency;
  static SomeDSP::LogScale<double> modToFreq;
  static SomeDSP::LogScale<double> modToSync;

  static SomeDSP::UIntScale<double> nVoice;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using ValueLinear = FloatValue<SomeDSP::LinearScale<double>>;
    using ValueLog = FloatValue<SomeDSP::LogScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "Bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::osc1Gain]
      = std::make_unique<ValueLog>(1.0, Scales::oscGain, "Osc1Gain", Info::kCanAutomate);
    value[ID::osc1Semi]
      = std::make_unique<ValueLinear>(0.5, Scales::semi, "Osc1Semi", Info::kCanAutomate);
    value[ID::osc1Cent]
      = std::make_unique<ValueLinear>(0.5, Scales::cent, "Osc1Cent", Info::kCanAutomate);
    value[ID::osc1Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, Scales::sync, "Osc1Sync", Info::kCanAutomate);
    value[ID::osc1SyncType] = std::make_unique<UIntValue>(
      0, Scales::osc1SyncType, "Osc1SyncType", Info::kCanAutomate);
    value[ID::osc1PTROrder] = std::make_unique<UIntValue>(
      16, Scales::osc1PTROrder, "Osc1PTROrder", Info::kCanAutomate);
    value[ID::osc1Phase] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Osc1Phase", Info::kCanAutomate);
    value[ID::osc1PhaseLock] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "Osc1PhaseLock", Info::kCanAutomate);

    value[ID::osc2Gain]
      = std::make_unique<ValueLog>(1.0, Scales::oscGain, "Osc2Gain", Info::kCanAutomate);
    value[ID::osc2Semi]
      = std::make_unique<ValueLinear>(0.5, Scales::semi, "Osc2Semi", Info::kCanAutomate);
    value[ID::osc2Cent]
      = std::make_unique<ValueLinear>(0.5, Scales::cent, "Osc2Cent", Info::kCanAutomate);
    value[ID::osc2Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, Scales::sync, "Osc2Sync", Info::kCanAutomate);
    value[ID::osc2SyncType] = std::make_unique<UIntValue>(
      0, Scales::osc1SyncType, "Osc2SyncType", Info::kCanAutomate);
    value[ID::osc2PTROrder] = std::make_unique<UIntValue>(
      16, Scales::osc1PTROrder, "Osc2PTROrder", Info::kCanAutomate);
    value[ID::osc2Invert] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "Osc2Invert", Info::kCanAutomate);
    value[ID::osc2Phase] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Osc2Phase", Info::kCanAutomate);
    value[ID::osc2PhaseLock] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "Osc2PhaseLock", Info::kCanAutomate);

    value[ID::fmOsc1ToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::fmToSync, "FM_Osc1_to_Sync1", Info::kCanAutomate);
    value[ID::fmOsc1ToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::fmToFreq, "FM_Osc1_to_Freq2", Info::kCanAutomate);
    value[ID::fmOsc2ToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::fmToSync, "FM_Osc2_to_Sync1", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<ValueLog>(0.5, Scales::gain, "Gain", Info::kCanAutomate);
    value[ID::gainA] = std::make_unique<ValueLog>(
      0.05, Scales::envelopeA, "GainAttack", Info::kCanAutomate);
    value[ID::gainD] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeD, "GainDecay", Info::kCanAutomate);
    value[ID::gainS] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeS, "GainSustain", Info::kCanAutomate);
    value[ID::gainR] = std::make_unique<ValueLog>(
      0.1, Scales::envelopeR, "GainRelease", Info::kCanAutomate);
    value[ID::gainEnvelopeCurve] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "GainEnvCurve", Info::kCanAutomate);

    value[ID::filterCutoff] = std::make_unique<ValueLog>(
      1.0, Scales::filterCutoff, "Cutoff", Info::kCanAutomate);
    value[ID::filterResonance] = std::make_unique<ValueLog>(
      0.5, Scales::filterResonance, "Resonance", Info::kCanAutomate);
    value[ID::filterFeedback] = std::make_unique<ValueLog>(
      0.0, Scales::filterFeedback, "Feedback", Info::kCanAutomate);
    value[ID::filterSaturation] = std::make_unique<ValueLog>(
      0.3, Scales::filterSaturation, "Saturation", Info::kCanAutomate);
    value[ID::filterDirty] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "DirtyBuffer", Info::kCanAutomate);
    value[ID::filterType] = std::make_unique<UIntValue>(
      0, Scales::filterType, "FilterType", Info::kCanAutomate);
    value[ID::filterShaper] = std::make_unique<UIntValue>(
      1, Scales::filterShaper, "FilterShaper", Info::kCanAutomate);
    value[ID::filterA] = std::make_unique<ValueLog>(
      0.0, Scales::envelopeA, "FilterEnvAttack", Info::kCanAutomate);
    value[ID::filterD] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeD, "FilterEnvDecay", Info::kCanAutomate);
    value[ID::filterS] = std::make_unique<ValueLog>(
      0.0, Scales::envelopeS, "FilterEnvSustain", Info::kCanAutomate);
    value[ID::filterR] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeR, "FilterEnvRelease", Info::kCanAutomate);
    value[ID::filterCutoffAmount] = std::make_unique<ValueLinear>(
      0.5, Scales::filterCutoffAmount, "CutoffAmount", Info::kCanAutomate);
    value[ID::filterResonanceAmount] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "ResonanceAmount", Info::kCanAutomate);
    value[ID::filterKeyToCutoff] = std::make_unique<ValueLinear>(
      0.5, Scales::filterKeyMod, "Key_to_Cutoff", Info::kCanAutomate);
    value[ID::filterKeyToFeedback] = std::make_unique<ValueLinear>(
      0.5, Scales::filterKeyMod, "Key_to_Feedback", Info::kCanAutomate);

    value[ID::modEnvelopeA] = std::make_unique<ValueLog>(
      0.0, Scales::modEnvelopeA, "ModEnvAttack", Info::kCanAutomate);
    value[ID::modEnvelopeCurve] = std::make_unique<ValueLog>(
      0.5, Scales::modEnvelopeCurve, "ModEnvCurve", Info::kCanAutomate);
    value[ID::modEnvelopeToFreq1] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "ModEnv_to_Freq1", Info::kCanAutomate);
    value[ID::modEnvelopeToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "ModEnv_to_Sync1", Info::kCanAutomate);
    value[ID::modEnvelopeToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "ModEnv_to_Freq2", Info::kCanAutomate);
    value[ID::modEnvelopeToSync2] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "ModEnv_to_Sync2", Info::kCanAutomate);
    value[ID::modLFOFrequency] = std::make_unique<ValueLog>(
      0.5, Scales::modLFOFrequency, "LFO_Frequency", Info::kCanAutomate);
    value[ID::modLFONoiseMix] = std::make_unique<ValueLinear>(
      0.01, Scales::defaultScale, "LFO_Noise_Mix", Info::kCanAutomate);
    value[ID::modLFOToFreq1] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "LFO_to_Freq1", Info::kCanAutomate);
    value[ID::modLFOToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "LFO_to_Sync1", Info::kCanAutomate);
    value[ID::modLFOToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "LFO_to_Freq2", Info::kCanAutomate);
    value[ID::modLFOToSync2] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "LFO_to_Sync2", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, "PitchBend", Info::kCanAutomate);
    value[ID::unison]
      = std::make_unique<UIntValue>(0, Scales::boolScale, "Unison", Info::kCanAutomate);
    value[ID::nVoice]
      = std::make_unique<UIntValue>(5, Scales::nVoice, "nVoice", Info::kCanAutomate);

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

  double getDefaultNormalized(int32_t tag) override
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
};

} // namespace Synth
} // namespace Steinberg
