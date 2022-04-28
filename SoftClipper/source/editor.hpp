// (c) 2020-2022 Takamitsu Endo
//
// This file is part of SoftClipper.
//
// SoftClipper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoftClipper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoftClipper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/curveview.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);
  ~Editor();

  void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  CurveView<decltype(Synth::Scales::guiInputGainScale)> *curveView = nullptr;
  TextTableView *infoTextView = nullptr;

  ParamValue getPlainValue(ParamID id);
  CurveView<decltype(Synth::Scales::guiInputGainScale)> *Editor::addCurveView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID tag,
    TextTableView *textView);
  void refreshCurveView(ParamID id);

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
