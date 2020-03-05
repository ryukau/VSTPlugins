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

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 14.0f;
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
  splashHeight = labelHeight;
  defaultWidth = int32(40 + 9.0f * knobX + labelY);
  defaultHeight = int32(40 + labelHeight + knobY);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // Phaser.
  const auto phaserTop = top0 - margin;
  const auto phaserLeft = left0;

  addKnob(phaserLeft, phaserTop, knobWidth, colorBlue, "Mix", ID::mix);
  addKnob(phaserLeft + knobX, phaserTop, knobWidth, colorBlue, "Freq", ID::frequency);
  addKnob(
    phaserLeft + 2.0f * knobX, phaserTop, knobWidth, colorBlue, "Spread", ID::freqSpread);
  addKnob(
    phaserLeft + 3.0f * knobX, phaserTop, knobWidth, colorRed, "Feedback", ID::feedback);
  addKnob(phaserLeft + 4.0f * knobX, phaserTop, knobWidth, colorBlue, "Range", ID::range);
  addKnob(phaserLeft + 5.0f * knobX, phaserTop, knobWidth, colorBlue, "Min", ID::min);
  addKnob(
    phaserLeft + 6.0f * knobX, phaserTop, knobWidth, colorRed, "Cas. Offset",
    ID::cascadeOffset);
  addKnob(
    phaserLeft + 7.0f * knobX, phaserTop, knobWidth, colorBlue, "L/R Offset",
    ID::stereoOffset);
  addRotaryKnob(
    phaserLeft + 8.0f * knobX, phaserTop, knobWidth + labelY, colorBlue, "Phase",
    ID::phase);

  const auto phaserTop1 = phaserTop + knobY + margin;
  const auto phaserLeft1 = left0 + 2.25f * knobX - margin;
  addLabel(phaserLeft1, phaserTop1, knobX * 1.2, "Stage");
  addTextKnob(
    phaserLeft1 + knobX, phaserTop1, knobX, colorBlue, ID::stage, Scales::stage, false, 0,
    1);

  const auto phaserLeft2 = phaserLeft1 + 2.25f * knobX;
  addLabel(phaserLeft2, phaserTop1, knobX, "Smooth");
  addTextKnob(
    phaserLeft2 + knobX, phaserTop1, knobX, colorBlue, ID::smoothness, Scales::smoothness,
    false, 3, 0);

  // Plugin name.
  const auto splashTop = phaserTop1;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX, splashHeight, 20.0f, 20.0f, defaultWidth - 40.0f,
    defaultHeight - 40.0f, "EsPhaser");

  return true;
}

} // namespace Vst
} // namespace Steinberg
