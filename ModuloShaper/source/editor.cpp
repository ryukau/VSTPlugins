// (c) 2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

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
constexpr float splashHeight = 30.0f;

constexpr float limiterLabelWidth = knobX + 3 * margin;

constexpr uint32_t defaultWidth
  = uint32_t(6 * knobX + 2 * margin + 2 * limiterLabelWidth + 30);
constexpr uint32_t defaultHeight = uint32_t(40 + 2 * knobY + 2 * margin + labelY);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();

  switch (tag) {
    case Synth::ParameterID::ID::type:
    case Synth::ParameterID::ID::limiter:
    case Synth::ParameterID::ID::limiterAttack:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = 16.0f;
  const auto left0 = 15.0f;

  addKnob(
    left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Cutoff", ID::lowpassCutoff);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Input", ID::inputGain);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Add", ID::add);
  addKnob(left0 + 3 * knobX, top0, knobX, margin, uiTextSize, "Mul", ID::mul);
  addKnob(left0 + 4 * knobX, top0, knobX, margin, uiTextSize, "Pre-Clip", ID::clipGain);
  addKnob(left0 + 5 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto top1 = top0 + knobY + 2 * margin;

  addKnob<Style::warning>(
    left0 + 2 * knobX, top1, knobX, margin, uiTextSize, "More Add", ID::moreAdd);
  addKnob<Style::warning>(
    left0 + 3 * knobX, top1, knobX, margin, uiTextSize, "More Mul", ID::moreMul);

  const auto top1Checkbox = top1 + floor(2.5 * margin);
  const auto leftCheckbox = left0 + 4 * knobX + 2 * margin;
  addCheckbox(
    leftCheckbox, top1Checkbox, 1.5f * knobX, labelHeight, uiTextSize, "Hardclip",
    ID::hardclip);

  addCheckbox(
    leftCheckbox, top1Checkbox + labelY, 1.5f * knobX, labelHeight, uiTextSize, "Lowpass",
    ID::lowpass);

  const auto top2 = top1 + knobY + 3 * margin;

  addLabel(left0, top2, 1.5f * knobX, labelHeight, uiTextSize, "Anti-aliasing");
  std::vector<std::string> typeItems{"None", "16x OverSampling", "PolyBLEP 4",
                                     "PolyBLEP 8"};
  addOptionMenu(
    left0 + 1.5f * knobX, top2, 2 * knobX, labelHeight, uiTextSize, ID::type, typeItems);

  // Limiter.
  const auto leftLimiter0 = left0 + 6 * knobX + 2 * margin;
  const auto leftLimiter1 = leftLimiter0 + limiterLabelWidth;
  const auto topLimiter1 = top0 + 1 * labelY;
  const auto topLimiter2 = top0 + 2 * labelY;
  const auto topLimiter3 = top0 + 3 * labelY;
  addToggleButton(
    leftLimiter0, top0, 2 * limiterLabelWidth, labelHeight, midTextSize, "Limiter",
    ID::limiter);
  addLabel(
    leftLimiter0, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize, "Threshold",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterThreshold, Scales::limiterThreshold, false, 5);
  addLabel(
    leftLimiter0, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize, "Attack [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterAttack, Scales::limiterAttack, false, 5);
  addLabel(
    leftLimiter0, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, Scales::limiterRelease, false, 5);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashLeft = defaultWidth + 2 * margin - 2 * knobX - 15.0f;
  addSplashScreen(
    splashLeft, splashTop, 2 * knobX - 2 * margin, splashHeight, 15.0f, 15.0f,
    defaultWidth - 30.0f, defaultHeight - 30.0f, pluginNameTextSize, "ModuloShaper");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
