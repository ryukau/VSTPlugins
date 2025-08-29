// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
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
