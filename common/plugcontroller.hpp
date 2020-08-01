// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/vst/ivstnoteexpression.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"

#include <algorithm>
#include <vector>

namespace Steinberg {
namespace Synth {

template<typename EditorType, typename ParameterType>
class PlugController : public Vst::EditController,
                       public Vst::IMidiMapping,
                       public Vst::INoteExpressionController {
public:
  std::vector<EditorType *> editor;

  ~PlugController()
  {
    for (auto &edi : editor) edi->forget();
  }

  static FUnknown *createInstance(void *)
  {
    return (Vst::IEditController *)new PlugController();
  }

  tresult PLUGIN_API initialize(FUnknown *context) SMTG_OVERRIDE;
  tresult PLUGIN_API setComponentState(IBStream *state) SMTG_OVERRIDE;
  IPlugView *PLUGIN_API createView(const char *name) SMTG_OVERRIDE;
  void editorDestroyed(Vst::EditorView *editorView) SMTG_OVERRIDE;
  tresult PLUGIN_API setParamNormalized(Vst::ParamID id, Vst::ParamValue normalized)
    SMTG_OVERRIDE;

  tresult PLUGIN_API getMidiControllerAssignment(
    int32 busIndex, int16 channel, Vst::CtrlNumber midiControllerNumber, Vst::ParamID &id)
    SMTG_OVERRIDE;

  virtual int32 PLUGIN_API getNoteExpressionCount(int32 busIndex, int16 channel)
    SMTG_OVERRIDE;
  virtual tresult PLUGIN_API getNoteExpressionInfo(
    int32 busIndex,
    int16 channel,
    int32 noteExpressionIndex,
    Vst::NoteExpressionTypeInfo &info) SMTG_OVERRIDE;
  virtual tresult PLUGIN_API getNoteExpressionStringByValue(
    int32 busIndex,
    int16 channel,
    Vst::NoteExpressionTypeID id,
    Vst::NoteExpressionValue valueNormalized,
    Vst::String128 string) SMTG_OVERRIDE;
  virtual tresult PLUGIN_API getNoteExpressionValueByString(
    int32 busIndex,
    int16 channel,
    Vst::NoteExpressionTypeID id,
    const Vst::TChar *string,
    Vst::NoteExpressionValue &valueNormalized) SMTG_OVERRIDE;

  OBJ_METHODS(PlugController, EditController)
  DEFINE_INTERFACES
  DEF_INTERFACE(IMidiMapping)
  DEF_INTERFACE(INoteExpressionController)
  END_DEFINE_INTERFACES(EditController)
  REFCOUNT_METHODS(EditController)
};

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API
PlugController<EditorType, ParameterType>::initialize(FUnknown *context)
{
  if (EditController::initialize(context) != kResultTrue) return kResultTrue;
  ParameterType param;
  return param.addParameter(parameters);
}

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API
PlugController<EditorType, ParameterType>::setComponentState(IBStream *state)
{
  if (!state) return kResultFalse;

  ParameterType param;
  if (param.setState(state)) return kResultFalse;

  for (auto &val : param.value)
    if (setParamNormalized(val->getId(), val->getNormalized())) return kResultFalse;

  return kResultOk;
}

template<typename EditorType, typename ParameterType>
IPlugView *PLUGIN_API
PlugController<EditorType, ParameterType>::createView(const char *name)
{
  if (name && strcmp(name, "editor") == 0) {
    auto plugEditor = new EditorType(this);
    plugEditor->remember();
    editor.push_back(plugEditor);
    return plugEditor;
  }
  return 0;
}

template<typename EditorType, typename ParameterType>
void PlugController<EditorType, ParameterType>::editorDestroyed(
  Vst::EditorView *editorView)
{
  auto iter = std::find(editor.begin(), editor.end(), editorView);
  if (iter == editor.end()) return;
  (*iter)->forget();
  editor.erase(iter);
}

template<typename EditorType, typename ParameterType>
tresult PLUGIN_API PlugController<EditorType, ParameterType>::setParamNormalized(
  Vst::ParamID id, Vst::ParamValue normalized)
{
  Vst::Parameter *parameter = getParameterObject(id);
  if (parameter) {
    parameter->setNormalized(normalized);
    for (auto &edi : editor) edi->updateUI(id, normalized);
    return kResultTrue;
  }
  return kResultFalse;
}

} // namespace Synth
} // namespace Steinberg
