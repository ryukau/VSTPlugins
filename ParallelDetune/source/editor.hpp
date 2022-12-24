// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelDetune.
//
// ParallelDetune is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelDetune is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelDetune.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  bool prepareUI() override;

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(CCoord left, CCoord top, CCoord width, CCoord height, ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setSlitWidth(2.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }
};

} // namespace Vst
} // namespace Steinberg
