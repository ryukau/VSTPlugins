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

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float checkboxWidth = 60.0f;
constexpr float splashHeight = 20.0f;
constexpr uint32_t defaultWidth = uint32_t(5 * knobX + 30);
constexpr uint32_t defaultHeight = uint32_t(30 + knobX + labelY + 3 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = 15.0f;
  const auto left0 = 15.0f;

  addKnob(left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Drive", ID::drive);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Boost", ID::boost);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto top1 = top0 + knobY + 3 * margin;
  addLabel(left0, top1, knobX, labelHeight, uiTextSize, "Order");
  addTextKnob(
    left0 + knobX, top1, knobX, labelHeight, uiTextSize, ID::order, Scales::order, false,
    0, 1);

  const auto checkboxLeft1 = left0 + 3 * knobX + 2 * margin;
  const auto checkboxHeight = labelY - margin;
  addCheckbox(checkboxLeft1, top0, knobX, labelHeight, uiTextSize, "Flip", ID::flip);
  addCheckbox(
    checkboxLeft1, top0 + checkboxHeight, 1.25f * knobX, labelHeight, uiTextSize,
    "Inverse", ID::inverse);
  addCheckbox(
    checkboxLeft1, top0 + 2 * checkboxHeight, 1.5f * knobX, labelHeight, uiTextSize,
    "OverSample", ID::oversample);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashLeft = checkboxLeft1;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX - 2 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, pluginNameTextSize, "OddPowShaper");

  return true;
}

} // namespace Vst
} // namespace Steinberg
