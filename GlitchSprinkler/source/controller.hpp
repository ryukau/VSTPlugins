// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/plugcontroller.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Synth {

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API PlugController<EditorType, ParameterType>::getMidiControllerAssignment(
  int32 busIndex, int16 channel, Vst::CtrlNumber midiControllerNumber, Vst::ParamID &id)
{
  // switch (midiControllerNumber) {
  //   case Vst::kPitchBend:
  //     id = ParameterID::pitchBend;
  //     return kResultOk;
  // }
  return kResultFalse;
}

template<typename EditorType, typename ParameterType>
int32 PLUGIN_API PlugController<EditorType, ParameterType>::getNoteExpressionCount(
  int32 busIndex, int16 channel)
{
  return 0;
}

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API PlugController<EditorType, ParameterType>::getNoteExpressionInfo(
  int32 busIndex,
  int16 channel,
  int32 noteExpressionIndex,
  Vst::NoteExpressionTypeInfo &info)
{
  return kResultFalse;
}

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API
PlugController<EditorType, ParameterType>::getNoteExpressionStringByValue(
  int32 busIndex,
  int16 channel,
  Vst::NoteExpressionTypeID id,
  Vst::NoteExpressionValue valueNormalized,
  Vst::String128 string)
{
  return kResultFalse;
}

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API
PlugController<EditorType, ParameterType>::getNoteExpressionValueByString(
  int32 busIndex,
  int16 channel,
  Vst::NoteExpressionTypeID id,
  const Vst::TChar *string,
  Vst::NoteExpressionValue &valueNormalized)
{
  return kResultFalse;
}

} // namespace Synth
} // namespace Steinberg
