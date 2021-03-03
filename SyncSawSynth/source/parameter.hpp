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
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , oscGain(0.0, 1.0, 0.75, 0.5)
    , semi(-24.0, 24.0)
    , cent(-100.0, 100.0)
    , sync(0.01, 16.0)
    , osc1SyncType(3)
    , osc1PTROrder(16)
    , fmToSync(0.0, 1.0, 0.5, 0.1)
    , fmToFreq(0.0, 1.0, 0.5, 0.1)
    , gain(0.0, 4.0, 0.75, 1.0)
    , envelopeA(0.0001, 16.0, 0.5, 1.0)
    , envelopeD(0.0001, 16.0, 0.5, 1.0)
    , envelopeS(0.0, 0.9995, 0.5, 0.3)
    , envelopeR(0.001, 16.0, 0.5, 2.0)
    , filterCutoff(20.0, 20000.0, 0.5, 800.0)
    , filterResonance(0.001, 1.0, 0.5, 0.3)
    , filterFeedback(0.0, 1.0, 0.5, 0.3)
    , filterSaturation(0.01, 8.0, 0.3, 0.5)
    , filterType(4)
    , filterShaper(3)
    , filterCutoffAmount(-1.0, 1.0)
    , filterKeyMod(-1.0, 1.0)
    , modEnvelopeA(0.0, 4.0, 0.5, 0.5)
    , modEnvelopeCurve(1.0, 96.0, 0.5, 8.0)
    , modLFOFrequency(0.01, 20.0, 0.5, 1.0)
    , modToFreq(0.0, 16.0, 0.5, 1.0)
    , modToSync(0.0, 16.0, 0.5, 1.0)
    , nVoice(5)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LogScale<double> oscGain;
  SomeDSP::LinearScale<double> semi;
  SomeDSP::LinearScale<double> cent;
  SomeDSP::LinearScale<double> sync;
  SomeDSP::UIntScale<double> osc1SyncType;
  SomeDSP::UIntScale<double> osc1PTROrder;

  SomeDSP::LogScale<double> fmToSync;
  SomeDSP::LogScale<double> fmToFreq;

  SomeDSP::LogScale<double> gain;

  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::LogScale<double> envelopeS;
  SomeDSP::LogScale<double> envelopeR;

  SomeDSP::LogScale<double> filterCutoff;
  SomeDSP::LogScale<double> filterResonance;
  SomeDSP::LogScale<double> filterFeedback;
  SomeDSP::LogScale<double> filterSaturation;
  SomeDSP::UIntScale<double> filterType;
  SomeDSP::UIntScale<double> filterShaper;
  SomeDSP::LinearScale<double> filterCutoffAmount;
  SomeDSP::LinearScale<double> filterKeyMod;

  SomeDSP::LogScale<double> modEnvelopeA;
  SomeDSP::LogScale<double> modEnvelopeCurve;
  SomeDSP::LogScale<double> modLFOFrequency;
  SomeDSP::LogScale<double> modToFreq;
  SomeDSP::LogScale<double> modToSync;

  SomeDSP::UIntScale<double> nVoice;
};

struct PlugParameter {
  Scales scale;
  std::vector<std::unique_ptr<ValueInterface>> value;

  PlugParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using ValueLinear = FloatValue<SomeDSP::LinearScale<double>>;
    using ValueLog = FloatValue<SomeDSP::LogScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, scale.boolScale, "Bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::osc1Gain]
      = std::make_unique<ValueLog>(1.0, scale.oscGain, "Osc1Gain", Info::kCanAutomate);
    value[ID::osc1Semi]
      = std::make_unique<ValueLinear>(0.5, scale.semi, "Osc1Semi", Info::kCanAutomate);
    value[ID::osc1Cent]
      = std::make_unique<ValueLinear>(0.5, scale.cent, "Osc1Cent", Info::kCanAutomate);
    value[ID::osc1Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, scale.sync, "Osc1Sync", Info::kCanAutomate);
    value[ID::osc1SyncType] = std::make_unique<UIntValue>(
      0, scale.osc1SyncType, "Osc1SyncType", Info::kCanAutomate);
    value[ID::osc1PTROrder] = std::make_unique<UIntValue>(
      16, scale.osc1PTROrder, "Osc1PTROrder", Info::kCanAutomate);
    value[ID::osc1Phase] = std::make_unique<ValueLinear>(
      0.0, scale.defaultScale, "Osc1Phase", Info::kCanAutomate);
    value[ID::osc1PhaseLock] = std::make_unique<UIntValue>(
      0, scale.boolScale, "Osc1PhaseLock", Info::kCanAutomate);

    value[ID::osc2Gain]
      = std::make_unique<ValueLog>(1.0, scale.oscGain, "Osc2Gain", Info::kCanAutomate);
    value[ID::osc2Semi]
      = std::make_unique<ValueLinear>(0.5, scale.semi, "Osc2Semi", Info::kCanAutomate);
    value[ID::osc2Cent]
      = std::make_unique<ValueLinear>(0.5, scale.cent, "Osc2Cent", Info::kCanAutomate);
    value[ID::osc2Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, scale.sync, "Osc2Sync", Info::kCanAutomate);
    value[ID::osc2SyncType] = std::make_unique<UIntValue>(
      0, scale.osc1SyncType, "Osc2SyncType", Info::kCanAutomate);
    value[ID::osc2PTROrder] = std::make_unique<UIntValue>(
      16, scale.osc1PTROrder, "Osc2PTROrder", Info::kCanAutomate);
    value[ID::osc2Invert]
      = std::make_unique<UIntValue>(0, scale.boolScale, "Osc2Invert", Info::kCanAutomate);
    value[ID::osc2Phase] = std::make_unique<ValueLinear>(
      0.0, scale.defaultScale, "Osc2Phase", Info::kCanAutomate);
    value[ID::osc2PhaseLock] = std::make_unique<UIntValue>(
      0, scale.boolScale, "Osc2PhaseLock", Info::kCanAutomate);

    value[ID::fmOsc1ToSync1] = std::make_unique<ValueLog>(
      0.0, scale.fmToSync, "FM_Osc1_to_Sync1", Info::kCanAutomate);
    value[ID::fmOsc1ToFreq2] = std::make_unique<ValueLog>(
      0.0, scale.fmToFreq, "FM_Osc1_to_Freq2", Info::kCanAutomate);
    value[ID::fmOsc2ToSync1] = std::make_unique<ValueLog>(
      0.0, scale.fmToSync, "FM_Osc2_to_Sync1", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<ValueLog>(0.5, scale.gain, "Gain", Info::kCanAutomate);
    value[ID::gainA] = std::make_unique<ValueLog>(
      0.05, scale.envelopeA, "GainAttack", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<ValueLog>(0.5, scale.envelopeD, "GainDecay", Info::kCanAutomate);
    value[ID::gainS] = std::make_unique<ValueLog>(
      0.5, scale.envelopeS, "GainSustain", Info::kCanAutomate);
    value[ID::gainR] = std::make_unique<ValueLog>(
      0.1, scale.envelopeR, "GainRelease", Info::kCanAutomate);
    value[ID::gainEnvelopeCurve] = std::make_unique<ValueLinear>(
      0.0, scale.defaultScale, "GainEnvCurve", Info::kCanAutomate);

    value[ID::filterCutoff]
      = std::make_unique<ValueLog>(1.0, scale.filterCutoff, "Cutoff", Info::kCanAutomate);
    value[ID::filterResonance] = std::make_unique<ValueLog>(
      0.5, scale.filterResonance, "Resonance", Info::kCanAutomate);
    value[ID::filterFeedback] = std::make_unique<ValueLog>(
      0.0, scale.filterFeedback, "Feedback", Info::kCanAutomate);
    value[ID::filterSaturation] = std::make_unique<ValueLog>(
      0.3, scale.filterSaturation, "Saturation", Info::kCanAutomate);
    value[ID::filterDirty] = std::make_unique<UIntValue>(
      0, scale.boolScale, "DirtyBuffer", Info::kCanAutomate);
    value[ID::filterType] = std::make_unique<UIntValue>(
      0, scale.filterType, "FilterType", Info::kCanAutomate);
    value[ID::filterShaper] = std::make_unique<UIntValue>(
      1, scale.filterShaper, "FilterShaper", Info::kCanAutomate);
    value[ID::filterA] = std::make_unique<ValueLog>(
      0.0, scale.envelopeA, "FilterEnvAttack", Info::kCanAutomate);
    value[ID::filterD] = std::make_unique<ValueLog>(
      0.5, scale.envelopeD, "FilterEnvDecay", Info::kCanAutomate);
    value[ID::filterS] = std::make_unique<ValueLog>(
      0.0, scale.envelopeS, "FilterEnvSustain", Info::kCanAutomate);
    value[ID::filterR] = std::make_unique<ValueLog>(
      0.5, scale.envelopeR, "FilterEnvRelease", Info::kCanAutomate);
    value[ID::filterCutoffAmount] = std::make_unique<ValueLinear>(
      0.5, scale.filterCutoffAmount, "CutoffAmount", Info::kCanAutomate);
    value[ID::filterResonanceAmount] = std::make_unique<ValueLinear>(
      0.0, scale.defaultScale, "ResonanceAmount", Info::kCanAutomate);
    value[ID::filterKeyToCutoff] = std::make_unique<ValueLinear>(
      0.5, scale.filterKeyMod, "Key_to_Cutoff", Info::kCanAutomate);
    value[ID::filterKeyToFeedback] = std::make_unique<ValueLinear>(
      0.5, scale.filterKeyMod, "Key_to_Feedback", Info::kCanAutomate);

    value[ID::modEnvelopeA] = std::make_unique<ValueLog>(
      0.0, scale.modEnvelopeA, "ModEnvAttack", Info::kCanAutomate);
    value[ID::modEnvelopeCurve] = std::make_unique<ValueLog>(
      0.5, scale.modEnvelopeCurve, "ModEnvCurve", Info::kCanAutomate);
    value[ID::modEnvelopeToFreq1] = std::make_unique<ValueLog>(
      0.0, scale.modToFreq, "ModEnv_to_Freq1", Info::kCanAutomate);
    value[ID::modEnvelopeToSync1] = std::make_unique<ValueLog>(
      0.0, scale.modToSync, "ModEnv_to_Sync1", Info::kCanAutomate);
    value[ID::modEnvelopeToFreq2] = std::make_unique<ValueLog>(
      0.0, scale.modToFreq, "ModEnv_to_Freq2", Info::kCanAutomate);
    value[ID::modEnvelopeToSync2] = std::make_unique<ValueLog>(
      0.0, scale.modToSync, "ModEnv_to_Sync2", Info::kCanAutomate);
    value[ID::modLFOFrequency] = std::make_unique<ValueLog>(
      0.5, scale.modLFOFrequency, "LFO_Frequency", Info::kCanAutomate);
    value[ID::modLFONoiseMix] = std::make_unique<ValueLinear>(
      0.01, scale.defaultScale, "LFO_Noise_Mix", Info::kCanAutomate);
    value[ID::modLFOToFreq1] = std::make_unique<ValueLog>(
      0.0, scale.modToFreq, "LFO_to_Freq1", Info::kCanAutomate);
    value[ID::modLFOToSync1] = std::make_unique<ValueLog>(
      0.0, scale.modToSync, "LFO_to_Sync1", Info::kCanAutomate);
    value[ID::modLFOToFreq2] = std::make_unique<ValueLog>(
      0.0, scale.modToFreq, "LFO_to_Freq2", Info::kCanAutomate);
    value[ID::modLFOToSync2] = std::make_unique<ValueLog>(
      0.0, scale.modToSync, "LFO_to_Sync2", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<ValueLinear>(
      0.5, scale.defaultScale, "PitchBend", Info::kCanAutomate);
    value[ID::unison]
      = std::make_unique<UIntValue>(0, scale.boolScale, "Unison", Info::kCanAutomate);
    value[ID::nVoice]
      = std::make_unique<UIntValue>(5, scale.nVoice, "nVoice", Info::kCanAutomate);

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
