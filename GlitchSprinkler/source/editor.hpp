// (c) 2023 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/polynomialxypad.hpp"
#include "parameter.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  virtual void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  bool prepareUI() override;

  SharedPointer<PolynomialXYPad> polyXYPad;
  SharedPointer<Label> polyXLabel;
  SharedPointer<Label> polyYLabel;
  SharedPointer<CControl> polyXControl;
  SharedPointer<CControl> polyYControl;
};

} // namespace Vst
} // namespace Steinberg
