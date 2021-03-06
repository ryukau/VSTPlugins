// (c) 2020 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 12.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float barboxWidth = 12.0f * knobX;
constexpr float barboxHeight = 2.0f * knobY;
constexpr float barboxY = barboxHeight + 2.0f * margin;
constexpr float checkboxWidth = 60.0f;
constexpr float splashHeight = labelHeight;
constexpr uint32_t defaultWidth = uint32_t(40 + 9.0f * knobX + labelY);
constexpr uint32_t defaultHeight = uint32_t(40 + labelHeight + knobY);

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

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // Phaser.
  const auto phaserTop = top0 - margin;
  const auto phaserLeft = left0;

  addKnob(phaserLeft, phaserTop, knobWidth, margin, uiTextSize, "Mix", ID::mix);
  addKnob(
    phaserLeft + knobX, phaserTop, knobWidth, margin, uiTextSize, "Freq", ID::frequency);
  addKnob(
    phaserLeft + 2.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Spread",
    ID::freqSpread);
  addKnob<Style::warning>(
    phaserLeft + 3.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Feedback",
    ID::feedback);
  addKnob(
    phaserLeft + 4.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Range",
    ID::range);
  addKnob(
    phaserLeft + 5.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Min", ID::min);
  addKnob(
    phaserLeft + 6.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Cas. Offset",
    ID::cascadeOffset);
  addKnob(
    phaserLeft + 7.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "L/R Offset",
    ID::stereoOffset);
  addRotaryKnob(
    phaserLeft + 8.0f * knobX, phaserTop, knobWidth + labelY, margin, uiTextSize, "Phase",
    ID::phase);

  const auto phaserTop1 = phaserTop + knobY + margin;
  const auto phaserLeft1 = left0 + 2.25f * knobX - margin;
  addLabel(phaserLeft1, phaserTop1, knobX * 1.2, labelHeight, uiTextSize, "Stage");
  addTextKnob(
    phaserLeft1 + knobX, phaserTop1, knobX, labelHeight, uiTextSize, ID::stage,
    Scales::stage, false, 0, 1);

  const auto phaserLeft2 = phaserLeft1 + 2.25f * knobX;
  addLabel(phaserLeft2, phaserTop1, knobX, labelHeight, uiTextSize, "Smooth");
  addTextKnob(
    phaserLeft2 + knobX, phaserTop1, knobX, labelHeight, uiTextSize, ID::smoothness,
    Scales::smoothness, false, 3, 0);

  // Plugin name.
  const auto splashTop = phaserTop1;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX, splashHeight, 20.0f, 20.0f, defaultWidth - 40.0f,
    defaultHeight - 40.0f, pluginNameTextSize, "EsPhaser");

  return true;
} // namespace Vst

} // namespace Vst
} // namespace Steinberg
