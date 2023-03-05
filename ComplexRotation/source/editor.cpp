// (c) 2022 Takamitsu Endo
//
// This file is part of ComplexRotation.
//
// ComplexRotation is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ComplexRotation is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ComplexRotation.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

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
  = int_least32_t(2 * uiMargin + 4 * labelY - 3 * margin + knobY + barboxHeight);

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
  constexpr auto apTop2 = apTop1 + labelY;
  constexpr auto apTop3 = apTop2 + labelY;
  constexpr auto apTop4 = apTop3 + labelY;
  constexpr auto apTop5 = apTop4 + labelY;
  constexpr auto apTop6 = apTop5 + labelY;
  constexpr auto apTop7 = apTop6 + labelY;
  constexpr auto apTop8 = apTop7 + labelY;

  constexpr auto apLeft0 = left0;
  constexpr auto apLeft1 = apLeft0 + knobX;
  constexpr auto apLeft2 = apLeft1 + knobX;

  constexpr auto apSectionHalfWidth = int(1.5 * knobX) - margin;
  constexpr auto apLeftHalf = apLeft0 + apSectionHalfWidth;

  addGroupLabel(apLeft0, apTop0, 3 * knobX - 2 * margin, labelHeight, uiTextSize, "Mod.");

  addLabel(apLeft0, apTop1, apSectionHalfWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    apLeftHalf, apTop1, apSectionHalfWidth, labelHeight, uiTextSize, ID::outputGain,
    Scales::outputGain, false, 5);
  addLabel(apLeft0, apTop2, apSectionHalfWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob(
    apLeftHalf, apTop2, apSectionHalfWidth, labelHeight, uiTextSize, ID::mix,
    Scales::defaultScale, false, 5);
  addLabel(apLeft0, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, "Input Mod.");
  addTextKnob(
    apLeftHalf, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, ID::inputPhaseMod,
    Scales::modulation, false, 5);
  addLabel(apLeft0, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, "Input LP [Hz]");
  addTextKnob(
    apLeftHalf, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, ID::inputLowpassHz,
    Scales::cutoffHz, false, 5);

  // Modulation.
  constexpr auto modTop0 = top0;
  constexpr auto modTop1 = modTop0 + labelY;
  constexpr auto modTop2 = modTop1 + labelY;
  constexpr auto modTop3 = modTop2 + labelY;
  constexpr auto modTop4 = modTop3 + labelY;
  constexpr auto modTop5 = modTop4 + labelY;
  constexpr auto modTop6 = modTop5 + labelY;
  constexpr auto modTop7 = modTop6 + labelY;
  constexpr auto modTop8 = modTop7 + labelY;
  constexpr auto modLeft0 = apLeft2 + knobX + 2 * margin;
  constexpr auto modLeft1 = modLeft0 + knobX;
  constexpr auto modLeft2 = modLeft1 + knobX;
  constexpr auto modLeftHalf = modLeft0 + apSectionHalfWidth;
  addGroupLabel(
    modLeft0, modTop0, 3 * knobX - 2 * margin, labelHeight, uiTextSize, "Modulation");

  /// TODO: Remove LFO section.
  // LFO.
  constexpr auto lfoTop0 = top0;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + 5 * margin;
  constexpr auto lfoTop3 = lfoTop1 + knobY;
  constexpr auto lfoLeft0 = apLeft2 + knobX + 2 * margin;
  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX + 2 * margin;
  constexpr auto lfoLeft3 = lfoLeft2 + knobX;
  constexpr auto lfoLeft4 = lfoLeft3 + knobX;
  constexpr auto lfoLeft5 = lfoLeft4 + knobX + 3 * margin;

  // Misc.
  constexpr auto miscTop0 = lfoTop0 + labelY + knobY + barboxHeight + 4 * margin;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscLeft0 = lfoLeft0 + int(0.25 * knobX);
  constexpr auto miscLeft1 = miscLeft0 + labelWidth;
  constexpr auto miscLeft2 = miscLeft1 + labelWidth + 4 * margin;
  constexpr auto miscLeft3 = miscLeft2 + labelWidth;
  constexpr auto smallLabelWidth = int(0.75 * labelWidth);
  addLabel(miscLeft0, miscTop0, labelWidth, labelHeight, uiTextSize, "Note Origin");
  addTextKnob(
    miscLeft1, miscTop0, labelWidth, labelHeight, uiTextSize, ID::notePitchOrigin,
    Scales::notePitchOrigin, false, 5);

  addLabel(miscLeft0, miscTop1, smallLabelWidth, labelHeight, uiTextSize, "Note>Cut");
  addSmallKnob(
    miscLeft0 + smallLabelWidth, miscTop1, labelHeight, labelHeight,
    ID::notePitchToAllpassCutoff);

  addLabel(miscLeft1, miscTop1, smallLabelWidth, labelHeight, uiTextSize, "Note>Time");
  addSmallKnob(
    miscLeft1 + smallLabelWidth, miscTop1, labelHeight, labelHeight,
    ID::notePitchToDelayTime);

  addLabel(miscLeft2, miscTop0, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft3, miscTop0, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);

  addCheckbox<Style::warning>(
    miscLeft2, miscTop1, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::oversampling);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = int(1.75 * labelWidth);
  constexpr auto splashHeight = labelY;
  constexpr auto splashTop = miscTop1;
  constexpr auto splashLeft = defaultWidth - uiMargin - splashWidth;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "ComplexRotation");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
