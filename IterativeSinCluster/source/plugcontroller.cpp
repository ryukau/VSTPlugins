// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"

#include "public.sdk/source/vst/vstparameters.h"

#include "gui/plugeditor.hpp"
#include "parameter.hpp"
#include "plugcontroller.hpp"

#include <algorithm>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

tresult PLUGIN_API PlugController::initialize(FUnknown *context)
{
  if (EditController::initialize(context) != kResultTrue) return kResultTrue;
  GlobalParameter param;
  return param.addParameter(parameters);
}

tresult PLUGIN_API PlugController::setComponentState(IBStream *state)
{
  if (!state) return kResultFalse;

  GlobalParameter param;
  if (param.setState(state)) return kResultFalse;

  for (auto &val : param.value)
    if (setParamNormalized(val->getId(), val->getNormalized())) return kResultFalse;

  return kResultOk;
}

IPlugView *PLUGIN_API PlugController::createView(const char *name)
{
  if (name && strcmp(name, "editor") == 0) {
    auto plugEditor = new Vst::PlugEditor(this);
    plugEditor->remember();
    editor.push_back(plugEditor);
    return plugEditor;
  }
  return 0;
}

void PlugController::editorDestroyed(Vst::EditorView *editorView)
{
  auto iter = std::find(editor.begin(), editor.end(), editorView);
  if (iter == editor.end()) return;
  (*iter)->forget();
  editor.erase(iter);
}

tresult PLUGIN_API
PlugController::setParamNormalized(Vst::ParamID id, Vst::ParamValue normalized)
{
  Vst::Parameter *parameter = getParameterObject(id);
  if (parameter) {
    parameter->setNormalized(normalized);
    for (auto &edi : editor) edi->updateUI(id, normalized);
    return kResultTrue;
  }
  return kResultFalse;
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

} // namespace Synth
} // namespace Steinberg
