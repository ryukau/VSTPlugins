// (c) 2021 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../common/plugcontroller.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Synth {

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API PlugController<EditorType, ParameterType>::getMidiControllerAssignment(
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
