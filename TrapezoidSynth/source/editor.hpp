// (c) 2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

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

  void addTpzLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name);

  void addSplashScreenTpz(
    CCoord buttonLeft,
    CCoord buttonTop,
    CCoord buttonWidth,
    CCoord buttonHeight,
    CCoord splashLeft,
    CCoord splashTop,
    CCoord splashWidth,
    CCoord splashHeight,
    CCoord textSize,
    const char *pluginName);
};

} // namespace Vst
} // namespace Steinberg
