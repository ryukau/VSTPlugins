// (c) 2022 Takamitsu Endo
//
// This file is part of OrdinaryPhaser.
//
// OrdinaryPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OrdinaryPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OrdinaryPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

// 480 + 20
constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 60.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 100.0f;
constexpr float labelX = labelWidth + margin;

constexpr float barboxWidth = 512.0f;
constexpr float barboxHeight = 200.0f;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 3 * knobX + barboxWidth + 2 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 3 * labelY - 3 * margin + knobY + barboxHeight);

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

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;

  // Allpass.
  constexpr auto apTop0 = top0;
  constexpr auto apTop1 = apTop0 + labelY;
  constexpr auto apTop2 = apTop1 + knobY;
  constexpr auto apTop3 = apTop2 + knobY + 2 * margin;
  constexpr auto apTop4 = apTop3 + labelY;
  constexpr auto apTop5 = apTop4 + labelY;
  constexpr auto apTop6 = apTop5 + labelY;
  constexpr auto apTop7 = apTop6 + labelY;

  constexpr auto apLeft0 = left0;
  constexpr auto apLeft1 = apLeft0 + 1 * knobX;
  constexpr auto apLeft2 = apLeft0 + 2 * knobX;

  constexpr auto apSectionHalfWidth = int(1.5 * knobX) - margin;
  constexpr auto apLeftHalf = apLeft0 + apSectionHalfWidth;

  addGroupLabel(
    apLeft0, apTop0, 3 * knobX - 2 * margin, labelHeight, uiTextSize, "Allpass");

  addKnob<Style::accent>(
    apLeft0, apTop1, knobWidth, margin, uiTextSize, "Output", ID::outputGain);
  addKnob<Style::warning>(apLeft1, apTop1, knobWidth, margin, uiTextSize, "Mix", ID::mix);
  addKnob<Style::warning>(
    apLeft2, apTop1, knobWidth, margin, uiTextSize, "Feedback", ID::feedback);

  addKnob(apLeft0, apTop2, knobWidth, margin, uiTextSize, "Delay", ID::delayTimeSeconds);
  addKnob(
    apLeft1, apTop2, knobWidth, margin, uiTextSize, "Time Mod.", ID::lfoToDelayAmount);
  addKnob(apLeft2, apTop2, knobWidth, margin, uiTextSize, "Cut Spread", ID::cutoffSpread);

  addLabel(apLeft0, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, "Min [Hz]");
  addTextKnob(
    apLeftHalf, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, ID::cutoffMinHz,
    Scales::cutoffHz, false, 5);
  addLabel(apLeft0, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, "Max [Hz]");
  addTextKnob(
    apLeftHalf, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, ID::cutoffMaxHz,
    Scales::cutoffHz, false, 5);
  addLabel(apLeft0, apTop5, apSectionHalfWidth, labelHeight, uiTextSize, "Delay Tuning");
  std::vector<std::string> lfoToDelayTuningTypeItems{
    "Exp. Mul.", "Linear Mul.", "Add", "Fill Lower", "Fill Higher"};
  addOptionMenu(
    apLeftHalf, apTop5, apSectionHalfWidth, labelHeight, uiTextSize,
    ID::lfoToDelayTuningType, lfoToDelayTuningTypeItems);
  addLabel(apLeft0, apTop6, apSectionHalfWidth, labelHeight, uiTextSize, "FM");
  addTextKnob(
    apLeftHalf, apTop6, apSectionHalfWidth, labelHeight, uiTextSize, ID::inputToDelayTime,
    Scales::defaultScale, false, 5);
  addLabel(apLeft0, apTop7, apSectionHalfWidth, labelHeight, uiTextSize, "Stage");
  addTextKnob(
    apLeftHalf, apTop7, apSectionHalfWidth, labelHeight, uiTextSize, ID::stage,
    Scales::stage, false, 0, 1);

  // LFO.
  constexpr auto lfoTop0 = top0;
  constexpr auto lfoTop1 = lfoTop0 + 1 * labelY;
  constexpr auto lfoTop2 = lfoTop1 + 5 * margin;
  constexpr auto lfoTop3 = lfoTop1 + knobY;
  constexpr auto lfoLeft0 = apLeft2 + knobX + 2 * margin;
  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX + 2 * margin;
  constexpr auto lfoLeft3 = lfoLeft2 + knobX;
  constexpr auto lfoLeft4 = lfoLeft3 + knobX;
  constexpr auto lfoLeft5 = lfoLeft4 + knobX + 3 * margin;
  addGroupLabel(lfoLeft0, lfoTop0, barboxWidth, labelHeight, uiTextSize, "LFO");

  addRotaryKnob(
    lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "L-R Offset", ID::lfoPhaseOffset);
  addRotaryKnob(
    lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Phase", ID::lfoPhaseConstant);

  addCheckbox(
    lfoLeft2, lfoTop2, knobWidth, labelHeight, uiTextSize, "Sync.", ID::lfoTempoSync);
  addTextKnob(
    lfoLeft3, lfoTop2 - 2 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoUpper,
    Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft3, lfoTop2 + 2 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoLower,
    Scales::lfoTempoLower, false, 0, 1);
  addKnob(lfoLeft4, lfoTop1, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate);

  addLabel(lfoLeft5, lfoTop1, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  std::vector<std::string> lfoInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu(
    lfoLeft5, lfoTop1 + labelY, labelWidth, labelHeight, uiTextSize, ID::lfoInterpolation,
    lfoInterpolationItems);
  auto barboxLfoWavetable = addBarBox(
    lfoLeft0, lfoTop3, barboxWidth, barboxHeight, ID::lfoWavetable0, Synth::nLfoWavetable,
    Scales::bipolarScale, "LFO Wave");
  if (barboxLfoWavetable) {
    barboxLfoWavetable->sliderZero = 0.5f;
  }

  // Misc.
  constexpr auto miscTop0 = lfoTop0 + labelY + knobY + barboxHeight + 4 * margin;
  constexpr auto miscLeft0 = left0 + int(3.75 * knobX);
  constexpr auto miscLeft1 = miscLeft0 + labelWidth;
  addLabel(miscLeft0, miscTop0, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop0, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = int(1.5 * labelWidth) + margin;
  constexpr auto splashHeight = labelY;
  constexpr auto splashTop = miscTop0 - 1 * margin;
  constexpr auto splashLeft = defaultWidth - uiMargin - splashWidth;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "OrdinaryPhaser");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
