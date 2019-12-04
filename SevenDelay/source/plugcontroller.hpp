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

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

#include "gui/plugeditor.hpp"

namespace Steinberg {
namespace SevenDelay {

class PlugController : public Vst::EditController {
public:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  ~PlugController()
  {
    if (editor) editor->forget();
  }

  static FUnknown *createInstance(void *)
  {
    return (Vst::IEditController *)new PlugController();
  }

  tresult PLUGIN_API initialize(FUnknown *context) SMTG_OVERRIDE;
  tresult PLUGIN_API setComponentState(IBStream *state) SMTG_OVERRIDE;
  IPlugView *PLUGIN_API createView(const char *name) SMTG_OVERRIDE;
  tresult PLUGIN_API setParamNormalized(Vst::ParamID id, Vst::ParamValue normalized)
    SMTG_OVERRIDE;

  OBJ_METHODS(PlugController, EditController)
  REFCOUNT_METHODS(EditController)
};

} // namespace SevenDelay
} // namespace Steinberg
