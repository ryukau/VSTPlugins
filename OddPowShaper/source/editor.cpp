// (c) 2020 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

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
  pluginNameTextSize = 18.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = 40.0f;
  knobX = 60.0f; // With margin.
  knobY = knobHeight + labelY;
  checkboxWidth = 60.0f;
  splashHeight = 20.0f;
  defaultWidth = int32(5 * knobX + 30);
  defaultHeight = int32(30 + knobX + labelY + 3 * margin);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};

  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto top0 = 15.0f;
  const auto left0 = 15.0f;

  addKnob(left0 + 0 * knobX, top0, knobX, colorBlue, "Drive", ID::drive);
  addKnob(left0 + 1 * knobX, top0, knobX, colorBlue, "Boost", ID::boost);
  addKnob(left0 + 2 * knobX, top0, knobX, colorBlue, "Output", ID::outputGain);

  const auto top1 = top0 + knobY + 3 * margin;
  addLabel(left0, top1, knobX, "Order");
  addTextKnob(
    left0 + knobX, top1, knobX, colorBlue, ID::order, Scales::order, false, 0, 1);

  const auto checkboxLeft1 = left0 + 3 * knobX + 2 * margin;
  const auto checkboxHeight = labelY - margin;
  addCheckbox(checkboxLeft1, top0, knobX, "Flip", ID::flip);
  addCheckbox(checkboxLeft1, top0 + checkboxHeight, knobX, "Inverse", ID::inverse);
  addCheckbox(
    checkboxLeft1, top0 + 2 * checkboxHeight, knobX, "OverSample", ID::oversample);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashLeft = checkboxLeft1 - 6 * margin;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX + 4 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, "OddPowShaper", 14.0f);

  return true;
}

} // namespace Vst
} // namespace Steinberg
