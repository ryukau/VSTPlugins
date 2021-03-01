// (c) 2021 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float splashHeight = 30.0f;

constexpr float limiterLabelWidth = 100.0f;
constexpr float checkboxWidth = 1.5f * limiterLabelWidth;

constexpr uint32_t defaultWidth = uint32_t(2 * uiMargin + 2 * limiterLabelWidth);
constexpr uint32_t defaultHeight = uint32_t(2 * uiMargin + 6 * labelY + splashHeight);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  // param = std::make_unique<Synth::PlugParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<> void Editor<Synth::PlugParameter>::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();

  switch (tag) {
    case Synth::ParameterID::ID::limiterAttack:
    case Synth::ParameterID::ID::truePeak:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Style = Uhhyou::Style;
  const auto &scale = param.scale;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Limiter.
  const auto leftLimiter0 = left0;
  const auto leftLimiter1 = leftLimiter0 + limiterLabelWidth;
  const auto topLimiter1 = top0;
  const auto topLimiter2 = top0 + 1 * labelY;
  const auto topLimiter3 = top0 + 2 * labelY;
  const auto topLimiter4 = top0 + 3 * labelY;
  const auto topLimiter5 = top0 + 4 * labelY;
  const auto topLimiter6 = top0 + 5 * labelY;

  addLabel(
    leftLimiter0, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    "Threshold [dB]", kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterThreshold, scale.limiterThreshold, true, 5);
  addLabel(
    leftLimiter0, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize, "Gate [dB]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterGate, scale.limiterGate, true, 5);
  addLabel(
    leftLimiter0, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize, "Attack [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterAttack, scale.limiterAttack, false, 5);
  addLabel(
    leftLimiter0, topLimiter4, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter4, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, scale.limiterRelease, false, 5);
  addLabel(
    leftLimiter0, topLimiter5, limiterLabelWidth, labelHeight, uiTextSize, "Sustain [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter5, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterSustain, scale.limiterSustain, false, 5);

  addCheckbox(
    leftLimiter0, topLimiter6, checkboxWidth, labelHeight, uiTextSize, "True Peak",
    ID::truePeak);

  // Plugin name.
  const auto splashMargin = uiMargin - margin;
  const auto splashTop = defaultHeight - splashHeight - splashMargin;
  const auto splashLeft = leftLimiter0;
  addSplashScreen(
    splashLeft, splashTop, checkboxWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "BasicLimiter");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
