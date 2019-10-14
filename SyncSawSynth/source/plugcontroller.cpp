// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
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

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"

#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"

#include "gui/plugeditor.hpp"
#include "parameter.hpp"
#include "plugcontroller.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

tresult PLUGIN_API PlugController::initialize(FUnknown *context)
{
  if (EditController::initialize(context) != kResultOk) return kResultOk;

  GlobalParameter param;

  parameters.addParameter(USTRING("Bypass"), nullptr, 1, param.bypass,
    Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, ParameterID::bypass,
    Vst::kRootUnitId, USTRING("Bypass"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Osc1Gain"), ParameterID::osc1Gain, GlobalParameter::scaleOscGain,
    param.osc1Gain, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc1Semi"), ParameterID::osc1Semi, GlobalParameter::scaleSemi,
    param.osc1Semi, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc1Cent"), ParameterID::osc1Cent, GlobalParameter::scaleCent,
    param.osc1Cent, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc1Sync"), ParameterID::osc1Sync, GlobalParameter::scaleSync,
    param.osc1Sync, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("Osc1SyncType"), nullptr, 3, param.osc1SyncType,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc1SyncType, Vst::kRootUnitId,
    USTRING("Osc1SyncType"));
  parameters.addParameter(USTRING("Osc1PTROrder"), nullptr, 16, param.osc1PTROrder / 16.0,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc1PTROrder, Vst::kRootUnitId,
    USTRING("Osc1PTROrder"));
  parameters.addParameter(USTRING("Osc1Phase"), nullptr, 0, param.osc1Phase,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc1Phase, Vst::kRootUnitId,
    USTRING("Osc1Phase"));
  parameters.addParameter(USTRING("Osc1PhaseLock"), nullptr, 1, param.osc1PhaseLock,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc1PhaseLock, Vst::kRootUnitId,
    USTRING("Osc1PhaseLock"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Osc2Gain"), ParameterID::osc2Gain, GlobalParameter::scaleOscGain,
    param.osc2Gain, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc2Semi"), ParameterID::osc2Semi, GlobalParameter::scaleSemi,
    param.osc2Semi, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc2Cent"), ParameterID::osc2Cent, GlobalParameter::scaleCent,
    param.osc2Cent, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Osc2Sync"), ParameterID::osc2Sync, GlobalParameter::scaleSync,
    param.osc2Sync, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("Osc2SyncType"), nullptr, 3, param.osc2SyncType,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc2SyncType, Vst::kRootUnitId,
    USTRING("Osc2SyncType"));
  parameters.addParameter(USTRING("Osc2PTROrder"), nullptr, 16, param.osc2PTROrder / 16.0,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc2PTROrder, Vst::kRootUnitId,
    USTRING("Osc2PTROrder"));
  parameters.addParameter(USTRING("Osc2Phase"), nullptr, 0, param.osc2Phase,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc2Phase, Vst::kRootUnitId,
    USTRING("Osc2Phase"));
  parameters.addParameter(USTRING("Osc2PhaseLock"), nullptr, 1, param.osc2PhaseLock,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc2PhaseLock, Vst::kRootUnitId,
    USTRING("Osc2PhaseLock"));
  parameters.addParameter(USTRING("Osc2Invert"), nullptr, 1, param.osc2Invert,
    Vst::ParameterInfo::kCanAutomate, ParameterID::osc2Invert, Vst::kRootUnitId,
    USTRING("Osc2Invert"));

  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("FM Osc1 to Sync1"),
      ParameterID::fmOsc1ToSync1, GlobalParameter::scaleFMToSync, param.fmOsc1ToSync1,
      nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("FM Osc1 to Freq2"),
      ParameterID::fmOsc1ToFreq2, GlobalParameter::scaleFMToFreq, param.fmOsc1ToFreq2,
      nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("FM Osc2 to Sync1"),
      ParameterID::fmOsc2ToSync1, GlobalParameter::scaleFMToSync, param.fmOsc2ToSync1,
      nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Gain"), ParameterID::gain, GlobalParameter::scaleGain, param.gain, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("GainAttack"), ParameterID::gainA, GlobalParameter::scaleEnvelopeA,
    param.gainA, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("GainDecay"), ParameterID::gainD, GlobalParameter::scaleEnvelopeD,
    param.gainD, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("GainSustain"), ParameterID::gainS, GlobalParameter::scaleEnvelopeS,
    param.gainS, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("GainRelease"), ParameterID::gainR, GlobalParameter::scaleEnvelopeR,
    param.gainR, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("GainEnvCurve"), nullptr, 0, param.gainEnvelopeCurve,
    Vst::ParameterInfo::kCanAutomate, ParameterID::gainEnvelopeCurve, Vst::kRootUnitId,
    USTRING("GainEnvCurve"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Cutoff"), ParameterID::filterCutoff, GlobalParameter::scaleFilterCutoff,
    param.filterCutoff, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Resonance"), ParameterID::filterResonance,
    GlobalParameter::scaleFilterResonance, param.filterResonance, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("Feedback"),
      ParameterID::filterFeedback, GlobalParameter::scaleFilterFeedback,
      param.filterFeedback, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Saturation"), ParameterID::filterSaturation,
    GlobalParameter::scaleFilterSaturation, param.filterSaturation, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("DirtyBuffer"), nullptr, 1, param.filterDirty,
    Vst::ParameterInfo::kCanAutomate, ParameterID::filterDirty, Vst::kRootUnitId,
    USTRING("DirtyBuffer"));
  parameters.addParameter(USTRING("FilterType"), nullptr, 4, param.filterType,
    Vst::ParameterInfo::kCanAutomate, ParameterID::filterType, Vst::kRootUnitId,
    USTRING("FilterType"));
  parameters.addParameter(USTRING("FilterShaper"), nullptr, 3, param.filterShaper,
    Vst::ParameterInfo::kCanAutomate, ParameterID::filterShaper, Vst::kRootUnitId,
    USTRING("FilterShaper"));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("FilterEnvAttack"), ParameterID::filterA, GlobalParameter::scaleEnvelopeA,
    param.filterA, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("FilterEnvDecay"), ParameterID::filterD, GlobalParameter::scaleEnvelopeD,
    param.filterD, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("FilterEnvSustain"), ParameterID::filterS, GlobalParameter::scaleEnvelopeS,
    param.filterS, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("FilterEnvRelease"), ParameterID::filterR, GlobalParameter::scaleEnvelopeR,
    param.filterR, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("CutoffAmount"), ParameterID::filterCutoffAmount,
    GlobalParameter::scaleFilterCutoffAmount, param.filterCutoffAmount, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("ResonanceAmount"), nullptr, 0,
    param.filterResonanceAmount, Vst::ParameterInfo::kCanAutomate,
    ParameterID::filterResonanceAmount, Vst::kRootUnitId, USTRING("ResonanceAmount"));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Key to Cutoff"), ParameterID::filterKeyToCutoff,
    GlobalParameter::scaleFilterKeyMod, param.filterKeyToCutoff, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("Key to Feedback"), ParameterID::filterKeyToFeedback,
    GlobalParameter::scaleFilterKeyMod, param.filterKeyToFeedback, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("ModEnvAttack"),
      ParameterID::modEnvelopeA, GlobalParameter::scaleModEnvelopeA, param.modEnvelopeA,
      nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("ModEnvCurve"), ParameterID::modEnvelopeCurve,
    GlobalParameter::scaleModEnvelopeCurve, param.modEnvelopeCurve, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("ModEnv to Freq1"), ParameterID::modEnvelopeToFreq1,
    GlobalParameter::scaleModToFreq, param.modEnvelopeToFreq1, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("ModEnv to Sync1"), ParameterID::modEnvelopeToSync1,
    GlobalParameter::scaleModToSync, param.modEnvelopeToSync1, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("ModEnv to Freq2"), ParameterID::modEnvelopeToFreq2,
    GlobalParameter::scaleModToFreq, param.modEnvelopeToFreq2, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("ModEnv to Sync2"), ParameterID::modEnvelopeToSync2,
    GlobalParameter::scaleModToSync, param.modEnvelopeToSync2, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO Frequency"), ParameterID::modLFOFrequency,
    GlobalParameter::scaleModLFOFrequency, param.modLFOFrequency, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(USTRING("ModNoise"), nullptr, 0, param.modLFONoiseMix,
    Vst::ParameterInfo::kCanAutomate, ParameterID::modLFONoiseMix, Vst::kRootUnitId,
    USTRING("LFO/Noise Mix"));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO to Freq1"), ParameterID::modLFOToFreq1, GlobalParameter::scaleModToFreq,
    param.modLFOToFreq1, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO to Sync1"), ParameterID::modLFOToSync1, GlobalParameter::scaleModToSync,
    param.modLFOToSync1, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO to Freq2"), ParameterID::modLFOToFreq2, GlobalParameter::scaleModToFreq,
    param.modLFOToFreq2, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));
  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO to Sync2"), ParameterID::modLFOToSync2, GlobalParameter::scaleModToSync,
    param.modLFOToSync2, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(USTRING("PitchBend"), nullptr, 0, param.pitchBend,
    Vst::ParameterInfo::kCanAutomate, ParameterID::pitchBend, Vst::kRootUnitId,
    USTRING("PitchBend"));

  parameters.addParameter(USTRING("Unison"), nullptr, 1, param.unison,
    Vst::ParameterInfo::kCanAutomate, ParameterID::unison, Vst::kRootUnitId,
    USTRING("Unison"));

  // Add parameter here.

  return kResultOk;
}

tresult PLUGIN_API PlugController::setComponentState(IBStream *state)
{
  if (!state) return kResultFalse;

  GlobalParameter param;
  if (param.setState(state) == kResultFalse) return kResultFalse;

  setParamNormalized(ParameterID::bypass, param.bypass);

  setParamNormalized(ParameterID::osc1Gain, param.osc1Gain);
  setParamNormalized(ParameterID::osc1Semi, param.osc1Semi);
  setParamNormalized(ParameterID::osc1Cent, param.osc1Cent);
  setParamNormalized(ParameterID::osc1Sync, param.osc1Sync);
  setParamNormalized(ParameterID::osc1SyncType, param.osc1SyncType / 3.0);
  setParamNormalized(ParameterID::osc1PTROrder, param.osc1PTROrder / 16.0);
  setParamNormalized(ParameterID::osc1Phase, param.osc1Phase);
  setParamNormalized(ParameterID::osc1PhaseLock, param.osc1PhaseLock);

  setParamNormalized(ParameterID::osc2Gain, param.osc2Gain);
  setParamNormalized(ParameterID::osc2Semi, param.osc2Semi);
  setParamNormalized(ParameterID::osc2Cent, param.osc2Cent);
  setParamNormalized(ParameterID::osc2Sync, param.osc2Sync);
  setParamNormalized(ParameterID::osc2SyncType, param.osc2SyncType / 3.0);
  setParamNormalized(ParameterID::osc2PTROrder, param.osc2PTROrder / 16.0);
  setParamNormalized(ParameterID::osc2Phase, param.osc2Phase);
  setParamNormalized(ParameterID::osc2PhaseLock, param.osc2PhaseLock);
  setParamNormalized(ParameterID::osc2Invert, param.osc2Invert);

  setParamNormalized(ParameterID::fmOsc1ToSync1, param.fmOsc1ToSync1);
  setParamNormalized(ParameterID::fmOsc1ToFreq2, param.fmOsc1ToFreq2);
  setParamNormalized(ParameterID::fmOsc2ToSync1, param.fmOsc2ToSync1);

  setParamNormalized(ParameterID::gain, param.gain);
  setParamNormalized(ParameterID::gainA, param.gainA);
  setParamNormalized(ParameterID::gainD, param.gainD);
  setParamNormalized(ParameterID::gainS, param.gainS);
  setParamNormalized(ParameterID::gainR, param.gainR);
  setParamNormalized(ParameterID::gainEnvelopeCurve, param.gainEnvelopeCurve);

  setParamNormalized(ParameterID::filterCutoff, param.filterCutoff);
  setParamNormalized(ParameterID::filterResonance, param.filterResonance);
  setParamNormalized(ParameterID::filterFeedback, param.filterFeedback);
  setParamNormalized(ParameterID::filterSaturation, param.filterSaturation);
  setParamNormalized(ParameterID::filterDirty, param.filterDirty);
  setParamNormalized(ParameterID::filterType, param.filterType / 4.0);
  setParamNormalized(ParameterID::filterShaper, param.filterShaper / 3.0);
  setParamNormalized(ParameterID::filterA, param.filterA);
  setParamNormalized(ParameterID::filterD, param.filterD);
  setParamNormalized(ParameterID::filterS, param.filterS);
  setParamNormalized(ParameterID::filterR, param.filterR);
  setParamNormalized(ParameterID::filterCutoffAmount, param.filterCutoffAmount);
  setParamNormalized(ParameterID::filterResonanceAmount, param.filterResonanceAmount);
  setParamNormalized(ParameterID::filterKeyToCutoff, param.filterKeyToCutoff);
  setParamNormalized(ParameterID::filterKeyToFeedback, param.filterKeyToFeedback);

  setParamNormalized(ParameterID::modEnvelopeA, param.modEnvelopeA);
  setParamNormalized(ParameterID::modEnvelopeCurve, param.modEnvelopeCurve);
  setParamNormalized(ParameterID::modEnvelopeToFreq1, param.modEnvelopeToFreq1);
  setParamNormalized(ParameterID::modEnvelopeToSync1, param.modEnvelopeToSync1);
  setParamNormalized(ParameterID::modEnvelopeToFreq2, param.modEnvelopeToFreq2);
  setParamNormalized(ParameterID::modEnvelopeToSync2, param.modEnvelopeToSync2);
  setParamNormalized(ParameterID::modLFOFrequency, param.modLFOFrequency);
  setParamNormalized(ParameterID::modLFONoiseMix, param.modLFONoiseMix);
  setParamNormalized(ParameterID::modLFOToFreq1, param.modLFOToFreq1);
  setParamNormalized(ParameterID::modLFOToSync1, param.modLFOToSync1);
  setParamNormalized(ParameterID::modLFOToFreq2, param.modLFOToFreq2);
  setParamNormalized(ParameterID::modLFOToSync2, param.modLFOToSync2);

  setParamNormalized(ParameterID::pitchBend, param.pitchBend);

  setParamNormalized(ParameterID::unison, param.unison);

  // Add parameter here.

  return kResultOk;
}

tresult PLUGIN_API PlugController::getMidiControllerAssignment(
  int32 busIndex, int16 channel, Vst::CtrlNumber midiControllerNumber, Vst::ParamID &id)
{
  switch (midiControllerNumber) {
    case Vst::kCtrlExpression:
    case Vst::kCtrlVolume:
      id = ParameterID::gain;
      return kResultOk;

    case Vst::kPitchBend:
      id = ParameterID::pitchBend;
      return kResultOk;
  }
  return kResultFalse;
}

IPlugView *PLUGIN_API PlugController::createView(const char *name)
{
#ifndef LINUX
  if (name && strcmp(name, "editor") == 0) return new Vst::PlugEditor(this);
#endif
  return 0;
}

} // namespace Synth
} // namespace Steinberg
