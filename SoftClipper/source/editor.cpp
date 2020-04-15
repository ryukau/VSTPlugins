// (c) 2020 Takamitsu Endo
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
  defaultWidth = int32(8 * knobX + 30);
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

  addKnob(left0 + 0 * knobX, top0, knobX, colorBlue, "Input", ID::inputGain);
  addKnob(left0 + 1 * knobX, top0, knobX, colorBlue, "Clip", ID::clip);
  addKnob(left0 + 2 * knobX, top0, knobX, colorBlue, "Ratio", ID::ratio);
  addKnob(left0 + 3 * knobX, top0, knobX, colorBlue, "Slope", ID::slope);
  addKnob(left0 + 4 * knobX, top0, knobX, colorBlue, "Output", ID::outputGain);

  const auto left1 = left0 + 5 * knobX + 4 * margin;
  const auto left2 = left1 + knobX + 6 * margin;

  auto orderIntLabel = addLabel(left1, top0, 1.5f * knobX, "Order Integer");
  orderIntLabel->setHoriAlign(CHoriTxtAlign::kLeftText);
  addTextKnob(left2, top0, knobX, colorBlue, ID::orderInteger, Scales::orderInteger);

  auto orderFracLabel = addLabel(left1, top0 + labelY, 1.5f * knobX, "Order Fraction");
  orderFracLabel->setHoriAlign(CHoriTxtAlign::kLeftText);
  addTextKnob(
    left2, top0 + labelY, knobX, colorBlue, ID::orderFraction, Scales::defaultScale,
    false, 4);

  addCheckbox(left1, top0 + 2 * labelY, knobX, "OverSample", ID::oversample);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX - 2 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, "SoftClipper", 14.0f);

  return true;
}

} // namespace Vst
} // namespace Steinberg
