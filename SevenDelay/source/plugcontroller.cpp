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

#include "plugcontroller.hpp"
#include "gui/plugeditor.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace SevenDelay {

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
#ifndef LINUX
  if (name && strcmp(name, "editor") == 0) {
    if (editor) editor->forget();
    editor = new Vst::PlugEditor(this);
    editor->remember();
    return editor;
  }
#endif
  return 0;
}

tresult PLUGIN_API
PlugController::setParamNormalized(Vst::ParamID id, Vst::ParamValue normalized)
{
  Vst::Parameter *parameter = getParameterObject(id);
  if (parameter) {
    parameter->setNormalized(normalized);
    if (editor != nullptr) editor->updateUI(id, normalized);
    return kResultTrue;
  }
  return kResultFalse;
}

} // namespace SevenDelay
} // namespace Steinberg
