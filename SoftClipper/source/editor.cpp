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
constexpr uint32_t defaultWidth = uint32_t(8 * knobX + 30);
constexpr uint32_t defaultHeight = uint32_t(30 + knobX + labelY + 3 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  const auto &scale = param.scale;
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = 15.0f;
  const auto left0 = 15.0f;

  addKnob(left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Input", ID::inputGain);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Clip", ID::clip);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Ratio", ID::ratio);
  addKnob(left0 + 3 * knobX, top0, knobX, margin, uiTextSize, "Slope", ID::slope);
  addKnob(left0 + 4 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto left1 = left0 + 5 * knobX + 4 * margin;
  const auto left2 = left1 + knobX + 6 * margin;

  addLabel(
    left1, top0, 1.5f * knobX, labelHeight, uiTextSize, "Order Integer", kLeftText);
  addTextKnob(
    left2, top0, knobX, labelHeight, uiTextSize, ID::orderInteger, scale.orderInteger);

  addLabel(
    left1, top0 + labelY, 1.5f * knobX, labelHeight, uiTextSize, "Order Fraction",
    kLeftText);
  addTextKnob(
    left2, top0 + labelY, knobX, labelHeight, uiTextSize, ID::orderFraction,
    scale.defaultScale, false, 4);

  addCheckbox(
    left1, top0 + 2 * labelY, 1.5f * knobX, labelHeight, uiTextSize, "OverSample",
    ID::oversample);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX - 2 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, pluginNameTextSize, "SoftClipper");

  return true;
}

} // namespace Vst
} // namespace Steinberg
