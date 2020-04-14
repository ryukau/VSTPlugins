// (c) 2020 Takamitsu Endo
//
// This file is part of FoldShaper.
//
// FoldShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FoldShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FoldShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 16.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = 40.0f;
  knobX = 60.0f; // With margin.
  knobY = knobHeight + labelY;
  barboxWidth = 12.0f * knobX;
  barboxHeight = 2.0f * knobY;
  barboxY = barboxHeight + 2.0f * margin;
  checkboxWidth = 60.0f;
  splashHeight = 20.0f;
  defaultWidth = int32(6 * knobX + 30);
  defaultHeight = int32(30 + 2 * labelY + splashHeight + margin);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};

  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto top0 = 15.0f;
  const auto left0 = 15.0f;

  addKnob(left0 + 0 * knobX, top0, knobX, colorBlue, "Input", ID::inputGain);
  addKnob(left0 + 1 * knobX, top0, knobX, colorBlue, "Mul", ID::mul);
  addKnob(left0 + 2 * knobX, top0, knobX, colorRed, "More Mul", ID::moreMul);
  addKnob(left0 + 3 * knobX, top0, knobX, colorBlue, "Output", ID::outputGain);

  const auto checkboxTop = top0;
  const auto checkboxLeft = left0 + 4 * knobX + 2 * margin;
  addCheckbox(checkboxLeft, checkboxTop, knobX, "OverSample", ID::oversample);
  addCheckbox(checkboxLeft, checkboxTop + labelY, knobX, "Hardclip", ID::hardclip);

  // Plugin name.
  const auto splashTop = checkboxTop + 2 * labelY + margin;
  const auto splashLeft = checkboxLeft;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX - 2 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, "FoldShaper", 14.0f);

  return true;
}

} // namespace Vst
} // namespace Steinberg
