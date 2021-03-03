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

constexpr float limiterLabelWidth = knobX + 3 * margin;

constexpr uint32_t defaultWidth
  = uint32_t(5 * knobX + 2 * limiterLabelWidth + 2 * margin + 30);
constexpr uint32_t defaultHeight = uint32_t(30 + 3 * labelY + labelHeight);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<> void Editor<Synth::PlugParameter>::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();

  switch (tag) {
    case Synth::ParameterID::ID::limiter:
    case Synth::ParameterID::ID::limiterAttack:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  const auto &scale = param.scale;
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = 15.0f;
  const auto left0 = 15.0f;

  addKnob(left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Drive", ID::drive);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Boost", ID::boost);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto topOrder0 = top0 + 3 * labelY;
  addLabel(left0, topOrder0, knobX, labelHeight, uiTextSize, "Order");
  addTextKnob(
    left0 + knobX, topOrder0, knobX, labelHeight, uiTextSize, ID::order, scale.order,
    false, 0, 1);

  const auto checkboxLeft1 = left0 + 3 * knobX + 2 * margin;
  const auto checkboxHeight = labelY - margin;
  addCheckbox(checkboxLeft1, top0, knobX, labelHeight, uiTextSize, "Flip", ID::flip);
  addCheckbox(
    checkboxLeft1, top0 + checkboxHeight, std::floor(1.25f * knobX), labelHeight,
    uiTextSize, "Inverse", ID::inverse);
  addCheckbox(
    checkboxLeft1, top0 + 2 * checkboxHeight, std::floor(1.5f * knobX), labelHeight,
    uiTextSize, "OverSample", ID::oversample);

  // Limiter.
  const auto leftLimiter0 = left0 + 5 * knobX + 2 * margin;
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
    ID::limiterThreshold, scale.limiterThreshold, false, 5);
  addLabel(
    leftLimiter0, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize, "Attack [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterAttack, scale.limiterAttack, false, 5);
  addLabel(
    leftLimiter0, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, scale.limiterRelease, false, 5);

  // Plugin name.
  const auto splashTop = top0 + 3 * labelY;
  const auto splashLeft = checkboxLeft1 - std::floor(0.5f * knobX);
  addSplashScreen(
    splashLeft, splashTop, std::floor(2.5f * knobX) - 2 * margin, splashHeight, 15.0f,
    15.0f, defaultWidth - 30.0f, defaultHeight - 30.0f, pluginNameTextSize,
    "OddPowShaper");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
