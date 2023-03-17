// (c) 2022 Takamitsu Endo
//
// This file is part of RingModSpacer.
//
// RingModSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RingModSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RingModSpacer.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelWidth = 80.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float halfLabelWidth = int(labelWidth / 2);

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 6 * labelWidth + 14 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 6 * labelY - 2 * margin);

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
  using ID = Synth::ParameterID::ID;

  ParamID id = pControl->getTag();

  switch (id) {
    case ID::inputLimiterAttackSeconds:
    case ID::sideLimiterAttackSeconds:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;

  const auto gainRange = Scales::gain.getRangeDB();

  // Misc.
  constexpr auto miscTop0 = top0;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscTop2 = miscTop1 + labelY;
  constexpr auto miscTop3 = miscTop2 + labelY;
  constexpr auto miscTop4 = miscTop3 + labelY;
  constexpr auto miscTop5 = miscTop4 + labelY;
  constexpr auto miscLeft0 = left0;
  constexpr auto miscLeft1 = miscLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    miscLeft0, miscTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Gain");

  addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  auto outputGainTextKnob = addTextKnob(
    miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain,
    Scales::gain, true, 5);
  if (outputGainTextKnob) {
    outputGainTextKnob->sensitivity = 0.1 / gainRange;
    outputGainTextKnob->lowSensitivity = 0.01 / gainRange;
    outputGainTextKnob->wheelSensitivity = 0.1 / gainRange;
  }
  addLabel(miscLeft0, miscTop2, labelWidth, labelHeight, uiTextSize, "Side Mix");
  addTextKnob(
    miscLeft1, miscTop2, labelWidth, labelHeight, uiTextSize, ID::sideMix,
    Scales::defaultScale, false, 5);
  addLabel(miscLeft0, miscTop3, labelWidth, labelHeight, uiTextSize, "Ring-Sub. Mix");
  addTextKnob(
    miscLeft1, miscTop3, labelWidth, labelHeight, uiTextSize, ID::ringSubtractMix,
    Scales::defaultScale, false, 5);

  addGroupLabel(
    miscLeft0, miscTop4, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Misc.");

  addLabel(miscLeft0, miscTop5, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop5, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);

  // Misc.
  constexpr auto inputTop0 = top0;
  constexpr auto inputTop1 = inputTop0 + labelY;
  constexpr auto inputTop2 = inputTop1 + labelY;
  constexpr auto inputTop3 = inputTop2 + labelY;
  constexpr auto inputLeft0 = miscLeft0 + 2 * labelWidth + 6 * margin;
  constexpr auto inputLeft1 = inputLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    inputLeft0, inputTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Input");

  addLabel(inputLeft0, inputTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  auto inputGainTextKnob = addTextKnob(
    inputLeft1, inputTop1, labelWidth, labelHeight, uiTextSize, ID::inputGain,
    Scales::gain, true, 5);
  if (inputGainTextKnob) {
    inputGainTextKnob->liveUpdate = false;
    inputGainTextKnob->sensitivity = 0.1 / gainRange;
    inputGainTextKnob->lowSensitivity = 0.01 / gainRange;
    inputGainTextKnob->wheelSensitivity = 0.1 / gainRange;
  }
  addLabel(inputLeft0, inputTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob<Style::warning>(
    inputLeft1, inputTop2, labelWidth, labelHeight, uiTextSize,
    ID::inputLimiterAttackSeconds, Scales::limiterAttackSeconds, false, 5);
  addLabel(inputLeft0, inputTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    inputLeft1, inputTop3, labelWidth, labelHeight, uiTextSize,
    ID::inputLimiterReleaseSeconds, Scales::limiterReleaseSeconds, false, 5);

  // Misc.
  constexpr auto sideTop0 = top0;
  constexpr auto sideTop1 = sideTop0 + labelY;
  constexpr auto sideTop2 = sideTop1 + labelY;
  constexpr auto sideTop3 = sideTop2 + labelY;
  constexpr auto sideLeft0 = inputLeft0 + 2 * labelWidth + 6 * margin;
  constexpr auto sideLeft1 = sideLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    sideLeft0, sideTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Side");

  addLabel(sideLeft0, sideTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  auto sideGainTextKnob = addTextKnob(
    sideLeft1, sideTop1, labelWidth, labelHeight, uiTextSize, ID::sideGain, Scales::gain,
    true, 5);
  if (sideGainTextKnob) {
    sideGainTextKnob->liveUpdate = false;
    sideGainTextKnob->sensitivity = 0.1 / gainRange;
    sideGainTextKnob->lowSensitivity = 0.01 / gainRange;
    sideGainTextKnob->wheelSensitivity = 0.1 / gainRange;
  }
  addLabel(sideLeft0, sideTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob<Style::warning>(
    sideLeft1, sideTop2, labelWidth, labelHeight, uiTextSize,
    ID::sideLimiterAttackSeconds, Scales::limiterAttackSeconds, false, 5);
  addLabel(sideLeft0, sideTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    sideLeft1, sideTop3, labelWidth, labelHeight, uiTextSize,
    ID::sideLimiterReleaseSeconds, Scales::limiterReleaseSeconds, false, 5);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = int(2 * labelWidth + 4 * margin);
  constexpr auto splashHeight = labelHeight + margin;
  constexpr auto splashTop = defaultHeight - uiMargin - splashHeight;
  constexpr auto splashLeft = inputLeft1;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "RingModSpacer");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
