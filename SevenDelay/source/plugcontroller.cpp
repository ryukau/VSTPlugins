// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"

#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"

#include "gui/plugeditor.hpp"
#include "parameter.hpp"
#include "plugcontroller.hpp"

namespace Steinberg {
namespace SevenDelay {

tresult PLUGIN_API PlugController::initialize(FUnknown *context)
{
  if (EditController::initialize(context) != kResultTrue) return kResultTrue;

  GlobalParameter param;

  parameters.addParameter(USTRING("Bypass"), nullptr, 1, param.bypass,
    Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, ParameterID::bypass,
    Vst::kRootUnitId, USTRING("Bypass"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Time"), ParameterID::time, GlobalParameter::scaleTime, param.time, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(USTRING("Feedback"), nullptr, 0, param.feedback,
    Vst::ParameterInfo::kCanAutomate, ParameterID::feedback, Vst::kRootUnitId,
    USTRING("Feedback"));

  parameters.addParameter(new Vst::ScaledParameter<SPolyScale<Vst::ParamValue>>(
    USTRING("Stereo"), ParameterID::offset, GlobalParameter::scaleOffset, param.offset,
    nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(USTRING("WetMix"), nullptr, 0, param.wetMix,
    Vst::ParameterInfo::kCanAutomate, ParameterID::wetMix, Vst::kRootUnitId,
    USTRING("Wet"));

  parameters.addParameter(USTRING("DryMix"), nullptr, 0, param.dryMix,
    Vst::ParameterInfo::kCanAutomate, ParameterID::dryMix, Vst::kRootUnitId,
    USTRING("Dry"));

  parameters.addParameter(USTRING("TempoSync"), nullptr, 1, param.tempoSync,
    Vst::ParameterInfo::kCanAutomate, ParameterID::tempoSync, Vst::kRootUnitId,
    USTRING("Sync"));

  parameters.addParameter(USTRING("NegativeFeedback"), nullptr, 1, param.negativeFeedback,
    Vst::ParameterInfo::kCanAutomate, ParameterID::negativeFeedback, Vst::kRootUnitId,
    USTRING("Negative"));

  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("LFO to Time"),
      ParameterID::lfoTimeAmount, GlobalParameter::scaleLfoTimeAmount,
      param.lfoTimeAmount, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("LFO Frequency"),
      ParameterID::lfoFrequency, GlobalParameter::scaleLfoFrequency, param.lfoFrequency,
      nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("LFO Shape"), ParameterID::lfoShape, GlobalParameter::scaleLfoShape,
    param.lfoShape, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(new Vst::ScaledParameter<LinearScale<Vst::ParamValue>>(
    USTRING("LFO Initial Phase"), ParameterID::lfoInitialPhase,
    GlobalParameter::scaleLfoInitialPhase, param.lfoInitialPhase, nullptr,
    Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(USTRING("LFO Phase Hold"), nullptr, 1, param.lfoHold,
    Vst::ParameterInfo::kCanAutomate, ParameterID::lfoHold, Vst::kRootUnitId,
    USTRING("Hold"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Smoothness"), ParameterID::smoothness, GlobalParameter::scaleSmoothness,
    param.smoothness, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(USTRING("Input Stereo Spread"), nullptr, 0, param.inSpread,
    Vst::ParameterInfo::kCanAutomate, ParameterID::inSpread, Vst::kRootUnitId,
    USTRING("inSpread"));

  parameters.addParameter(USTRING("Input Pan"), nullptr, 0, param.inPan,
    Vst::ParameterInfo::kCanAutomate, ParameterID::inPan, Vst::kRootUnitId,
    USTRING("inPan"));

  parameters.addParameter(USTRING("Output Stereo Spread"), nullptr, 0, param.outSpread,
    Vst::ParameterInfo::kCanAutomate, ParameterID::outSpread, Vst::kRootUnitId,
    USTRING("outSpread"));

  parameters.addParameter(USTRING("Output Pan"), nullptr, 0, param.outPan,
    Vst::ParameterInfo::kCanAutomate, ParameterID::outPan, Vst::kRootUnitId,
    USTRING("outPan"));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Allpass Cutoff"), ParameterID::toneCutoff, GlobalParameter::scaleToneCutoff,
    param.toneCutoff, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("Allpass Q"), ParameterID::toneQ, GlobalParameter::scaleToneQ, param.toneQ,
    nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(
    USTRING("DC Kill"), ParameterID::dckill, GlobalParameter::scaleDCKill, param.dckill,
    nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  parameters.addParameter(
    new Vst::ScaledParameter<LogScale<Vst::ParamValue>>(USTRING("LFO to Allpass"),
      ParameterID::lfoToneAmount, GlobalParameter::scaleLfoToneAmount,
      param.lfoToneAmount, nullptr, Vst::ParameterInfo::kCanAutomate, Vst::kRootUnitId));

  // Add parameter here.

  return kResultTrue;
}

tresult PLUGIN_API PlugController::setComponentState(IBStream *state)
{
  if (!state) return kResultFalse;

  GlobalParameter param;
  if (param.setState(state) == kResultFalse) return kResultFalse;

  setParamNormalized(ParameterID::bypass, param.bypass);
  setParamNormalized(ParameterID::time, param.time);
  setParamNormalized(ParameterID::feedback, param.feedback);
  setParamNormalized(ParameterID::offset, param.offset);
  setParamNormalized(ParameterID::wetMix, param.wetMix);
  setParamNormalized(ParameterID::dryMix, param.dryMix);
  setParamNormalized(ParameterID::tempoSync, param.tempoSync);
  setParamNormalized(ParameterID::negativeFeedback, param.negativeFeedback);
  setParamNormalized(ParameterID::lfoTimeAmount, param.lfoTimeAmount);
  setParamNormalized(ParameterID::lfoFrequency, param.lfoFrequency);
  setParamNormalized(ParameterID::lfoShape, param.lfoShape);
  setParamNormalized(ParameterID::lfoInitialPhase, param.lfoInitialPhase);
  setParamNormalized(ParameterID::lfoHold, param.lfoHold);
  setParamNormalized(ParameterID::smoothness, param.smoothness);
  setParamNormalized(ParameterID::inSpread, param.inSpread);
  setParamNormalized(ParameterID::inPan, param.inPan);
  setParamNormalized(ParameterID::outSpread, param.outSpread);
  setParamNormalized(ParameterID::outPan, param.outPan);
  setParamNormalized(ParameterID::toneCutoff, param.toneCutoff);
  setParamNormalized(ParameterID::dckill, param.dckill);
  setParamNormalized(ParameterID::lfoToneAmount, param.lfoToneAmount);
  setParamNormalized(ParameterID::toneQ, param.toneQ);

  // Add parameter here.

  return kResultOk;
}

IPlugView *PLUGIN_API PlugController::createView(const char *name)
{
  // Disabling GUI for Linux build.
#ifndef LINUX
  if (name && strcmp(name, "editor") == 0) return new Vst::PlugEditor(this);
#endif
  return 0;
}

} // namespace SevenDelay
} // namespace Steinberg
