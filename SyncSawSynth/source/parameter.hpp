// (c) 2019 Takamitsu Endo
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

#include "value.hpp"

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
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> oscGain;
  static SomeDSP::LinearScale<double> semi;
  static SomeDSP::LinearScale<double> cent;
  static SomeDSP::LinearScale<double> sync;

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
  static SomeDSP::LinearScale<double> filterCutoffAmount;
  static SomeDSP::LinearScale<double> filterKeyMod;

  static SomeDSP::LogScale<double> modEnvelopeA;
  static SomeDSP::LogScale<double> modEnvelopeCurve;
  static SomeDSP::LogScale<double> modLFOFrequency;
  static SomeDSP::LogScale<double> modToFreq;
  static SomeDSP::LogScale<double> modToSync;
};

struct GlobalParameter {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using ValueLinear = FloatValue<SomeDSP::LinearScale<double>>;
    using ValueLog = FloatValue<SomeDSP::LogScale<double>>;

    value[ID::bypass] = std::make_unique<IntValue>(
      0, 1, "Bypass", Info::kCanAutomate | Info::kIsBypass, ID::bypass);

    value[ID::unison]
      = std::make_unique<IntValue>(0, 1, "Unison", Info::kCanAutomate, ID::unison);

    value[ID::osc1Gain] = std::make_unique<ValueLog>(
      1.0, Scales::oscGain, "Osc1Gain", Info::kCanAutomate, ID::osc1Gain);
    value[ID::osc1Semi] = std::make_unique<ValueLinear>(
      0.5, Scales::semi, "Osc1Semi", Info::kCanAutomate, ID::osc1Semi);
    value[ID::osc1Cent] = std::make_unique<ValueLinear>(
      0.5, Scales::cent, "Osc1Cent", Info::kCanAutomate, ID::osc1Cent);
    value[ID::osc1Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, Scales::sync, "Osc1Sync", Info::kCanAutomate, ID::osc1Sync);
    value[ID::osc1SyncType] = std::make_unique<IntValue>(
      0, 3, "Osc1SyncType", Info::kCanAutomate, ID::osc1SyncType);
    value[ID::osc1PTROrder] = std::make_unique<IntValue>(
      16, 16, "Osc1PTROrder", Info::kCanAutomate, ID::osc1PTROrder);
    value[ID::osc1Phase] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Osc1Phase", Info::kCanAutomate, ID::osc1Phase);
    value[ID::osc1PhaseLock] = std::make_unique<IntValue>(
      0, 1, "Osc1PhaseLock", Info::kCanAutomate, ID::osc1PhaseLock);

    value[ID::osc2Gain] = std::make_unique<ValueLog>(
      1.0, Scales::oscGain, "Osc2Gain", Info::kCanAutomate, ID::osc2Gain);
    value[ID::osc2Semi] = std::make_unique<ValueLinear>(
      0.5, Scales::semi, "Osc2Semi", Info::kCanAutomate, ID::osc2Semi);
    value[ID::osc2Cent] = std::make_unique<ValueLinear>(
      0.5, Scales::cent, "Osc2Cent", Info::kCanAutomate, ID::osc2Cent);
    value[ID::osc2Sync] = std::make_unique<ValueLinear>(
      1.0 / 16.0, Scales::sync, "Osc2Sync", Info::kCanAutomate, ID::osc2Sync);
    value[ID::osc2SyncType] = std::make_unique<IntValue>(
      0, 3, "Osc2SyncType", Info::kCanAutomate, ID::osc2SyncType);
    value[ID::osc2PTROrder] = std::make_unique<IntValue>(
      16, 16, "Osc2PTROrder", Info::kCanAutomate, ID::osc2PTROrder);
    value[ID::osc2Phase] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "Osc2Phase", Info::kCanAutomate, ID::osc2Phase);
    value[ID::osc2PhaseLock] = std::make_unique<IntValue>(
      0, 1, "Osc2PhaseLock", Info::kCanAutomate, ID::osc2PhaseLock);
    value[ID::osc2Invert] = std::make_unique<IntValue>(
      0, 1, "Osc2Invert", Info::kCanAutomate, ID::osc2Invert);

    value[ID::fmOsc1ToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::fmToSync, "FM_Osc1_to_Sync1", Info::kCanAutomate, ID::fmOsc1ToSync1);
    value[ID::fmOsc1ToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::fmToFreq, "FM_Osc1_to_Freq2", Info::kCanAutomate, ID::fmOsc1ToFreq2);
    value[ID::fmOsc2ToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::fmToSync, "FM_Osc2_to_Sync1", Info::kCanAutomate, ID::fmOsc2ToSync1);

    value[ID::gain] = std::make_unique<ValueLog>(
      0.5, Scales::gain, "Gain", Info::kCanAutomate, ID::gain);
    value[ID::gainA] = std::make_unique<ValueLog>(
      0.05, Scales::envelopeA, "GainAttack", Info::kCanAutomate, ID::gainA);
    value[ID::gainD] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeD, "GainDecay", Info::kCanAutomate, ID::gainD);
    value[ID::gainS] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeS, "GainSustain", Info::kCanAutomate, ID::gainS);
    value[ID::gainR] = std::make_unique<ValueLog>(
      0.1, Scales::envelopeR, "GainRelease", Info::kCanAutomate, ID::gainR);
    value[ID::gainEnvelopeCurve] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "GainEnvCurve", Info::kCanAutomate,
      ID::gainEnvelopeCurve);

    value[ID::filterCutoff] = std::make_unique<ValueLog>(
      1.0, Scales::filterCutoff, "Cutoff", Info::kCanAutomate, ID::filterCutoff);
    value[ID::filterResonance] = std::make_unique<ValueLog>(
      0.5, Scales::filterResonance, "Resonance", Info::kCanAutomate, ID::filterResonance);
    value[ID::filterFeedback] = std::make_unique<ValueLog>(
      0.0, Scales::filterFeedback, "Feedback", Info::kCanAutomate, ID::filterFeedback);
    value[ID::filterSaturation] = std::make_unique<ValueLog>(
      0.3, Scales::filterSaturation, "Saturation", Info::kCanAutomate,
      ID::filterSaturation);
    value[ID::filterDirty] = std::make_unique<IntValue>(
      0, 1, "DirtyBuffer", Info::kCanAutomate, ID::filterDirty);
    value[ID::filterType] = std::make_unique<IntValue>(
      0, 4, "FilterType", Info::kCanAutomate, ID::filterType);
    value[ID::filterShaper] = std::make_unique<IntValue>(
      1, 3, "FilterShaper", Info::kCanAutomate, ID::filterShaper);
    value[ID::filterA] = std::make_unique<ValueLog>(
      0.0, Scales::envelopeA, "FilterEnvAttack", Info::kCanAutomate, ID::filterA);
    value[ID::filterD] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeD, "FilterEnvDecay", Info::kCanAutomate, ID::filterD);
    value[ID::filterS] = std::make_unique<ValueLog>(
      0.0, Scales::envelopeS, "FilterEnvSustain", Info::kCanAutomate, ID::filterS);
    value[ID::filterR] = std::make_unique<ValueLog>(
      0.5, Scales::envelopeR, "FilterEnvRelease", Info::kCanAutomate, ID::filterR);
    value[ID::filterCutoffAmount] = std::make_unique<ValueLinear>(
      0.5, Scales::filterCutoffAmount, "CutoffAmount", Info::kCanAutomate,
      ID::filterCutoffAmount);
    value[ID::filterResonanceAmount] = std::make_unique<ValueLinear>(
      0.0, Scales::defaultScale, "ResonanceAmount", Info::kCanAutomate,
      ID::filterResonanceAmount);
    value[ID::filterKeyToCutoff] = std::make_unique<ValueLinear>(
      0.5, Scales::filterKeyMod, "Key_to_Cutoff", Info::kCanAutomate,
      ID::filterKeyToCutoff);
    value[ID::filterKeyToFeedback] = std::make_unique<ValueLinear>(
      0.5, Scales::filterKeyMod, "Key_to_Feedback", Info::kCanAutomate,
      ID::filterKeyToFeedback);

    value[ID::modEnvelopeA] = std::make_unique<ValueLog>(
      0.0, Scales::modEnvelopeA, "ModEnvAttack", Info::kCanAutomate, ID::modEnvelopeA);
    value[ID::modEnvelopeCurve] = std::make_unique<ValueLog>(
      0.5, Scales::modEnvelopeCurve, "ModEnvCurve", Info::kCanAutomate,
      ID::modEnvelopeCurve);
    value[ID::modEnvelopeToFreq1] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "ModEnv_to_Freq1", Info::kCanAutomate,
      ID::modEnvelopeToFreq1);
    value[ID::modEnvelopeToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "ModEnv_to_Sync1", Info::kCanAutomate,
      ID::modEnvelopeToSync1);
    value[ID::modEnvelopeToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "ModEnv_to_Freq2", Info::kCanAutomate,
      ID::modEnvelopeToFreq2);
    value[ID::modEnvelopeToSync2] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "ModEnv_to_Sync2", Info::kCanAutomate,
      ID::modEnvelopeToSync2);
    value[ID::modLFOFrequency] = std::make_unique<ValueLog>(
      0.5, Scales::modLFOFrequency, "LFO_Frequency", Info::kCanAutomate,
      ID::modLFOFrequency);
    value[ID::modLFONoiseMix] = std::make_unique<ValueLinear>(
      0.01, Scales::defaultScale, "LFO_Noise_Mix", Info::kCanAutomate,
      ID::modLFONoiseMix);
    value[ID::modLFOToFreq1] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "LFO_to_Freq1", Info::kCanAutomate, ID::modLFOToFreq1);
    value[ID::modLFOToSync1] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "LFO_to_Sync1", Info::kCanAutomate, ID::modLFOToSync1);
    value[ID::modLFOToFreq2] = std::make_unique<ValueLog>(
      0.0, Scales::modToFreq, "LFO_to_Freq2", Info::kCanAutomate, ID::modLFOToFreq2);
    value[ID::modLFOToSync2] = std::make_unique<ValueLog>(
      0.0, Scales::modToSync, "LFO_to_Sync2", Info::kCanAutomate, ID::modLFOToSync2);

    value[ID::pitchBend] = std::make_unique<ValueLinear>(
      0.5, Scales::defaultScale, "PitchBend", Info::kCanAutomate, ID::pitchBend);
    value[ID::nVoice]
      = std::make_unique<IntValue>(5, 5, "nVoice", Info::kCanAutomate, ID::nVoice);
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
};

} // namespace Synth
} // namespace Steinberg
