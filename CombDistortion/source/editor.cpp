// (c) 2022 Takamitsu Endo
//
// This file is part of CombDistortion.
//
// CombDistortion is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CombDistortion is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CombDistortion.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 80.0f;
constexpr float halfKnobWidth = knobWidth / 2;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 4 * knobX - 2 * margin);
constexpr int_least32_t defaultHeight = int_least32_t(2 * uiMargin + 8 * labelY + knobY);

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

  constexpr auto top1 = uiMargin;
  constexpr auto top2 = top1 + knobY + 2 * margin;
  constexpr auto top3 = top2 + labelY;
  constexpr auto top4 = top3 + labelY;
  constexpr auto top5 = top4 + labelY;
  constexpr auto top6 = top5 + labelY;
  constexpr auto top7 = top6 + labelY;
  constexpr auto top8 = top7 + labelY;
  constexpr auto top9 = top8 + labelY;

  constexpr auto left0 = uiMargin;
  constexpr auto left1 = left0 + knobX;
  constexpr auto left2 = left1 + knobX;
  constexpr auto left3 = left2 + knobX;

  // Distortion.
  addKnob<Style::accent>(
    left0, top1, knobWidth, margin, uiTextSize, "Output", ID::outputGain);
  addKnob<Style::warning>(left1, top1, knobWidth, margin, uiTextSize, "Mix", ID::mix);
  addKnob<Style::warning>(
    left2, top1, knobWidth, margin, uiTextSize, "Feedback", ID::feedback);
  addKnob(left3, top1, knobWidth, margin, uiTextSize, "Delay", ID::delayTimeSeconds);

  addLabel(left0, top2, knobWidth, labelHeight, uiTextSize, "AM Mix");
  addTextKnob(
    left1, top2, knobWidth, labelHeight, uiTextSize, ID::amMix, Scales::defaultScale,
    false, 5);
  addLabel(left0, top3, knobWidth, labelHeight, uiTextSize, "AM Gain");
  addTextKnob(
    left1, top3, knobWidth, labelHeight, uiTextSize, ID::amClipGain, Scales::gain, false,
    5);
  addLabel(left0, top4, knobWidth, labelHeight, uiTextSize, "AM Invert");
  addCheckbox(
    left1 + halfKnobWidth - int(labelHeight / 2), top4, halfKnobWidth, labelHeight,
    uiTextSize, "", ID::amClipGainNegative);
  addLabel(left0, top5, knobWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob<Style::warning>(
    left1, top5, knobWidth, labelHeight, uiTextSize, ID::feedbackHighpassHz,
    Scales::highpassHz, false, 3);

  addLabel(left2, top2, knobWidth, labelHeight, uiTextSize, "FM Mix");
  addTextKnob(
    left3, top2, knobWidth, labelHeight, uiTextSize, ID::fmMix, Scales::defaultScale,
    false, 5);
  addLabel(left2, top3, knobWidth, labelHeight, uiTextSize, "FM Amount");
  addTextKnob(
    left3, top3, knobWidth, labelHeight, uiTextSize, ID::fmAmount, Scales::gain, false,
    5);
  addLabel(left2, top4, knobWidth, labelHeight, uiTextSize, "FM Clip");
  addTextKnob(
    left3, top4, knobWidth, labelHeight, uiTextSize, ID::fmClip, Scales::gain, false, 5);
  addLabel(left2, top5, knobWidth, labelHeight, uiTextSize, "Lowpass [Hz]");
  addTextKnob<Style::warning>(
    left3, top5, knobWidth, labelHeight, uiTextSize, ID::feedbackLowpassHz,
    Scales::lowpassHz, false, 3);

  // Note.
  addGroupLabel(left0, top6, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Note");

  addLabel(left0, top7, knobWidth, labelHeight, uiTextSize, "Origin [st.]");
  addTextKnob(
    left1, top7, knobWidth, labelHeight, uiTextSize, ID::notePitchOrigin,
    Scales::notePitchOrigin, false, 5);
  addLabel(left0, top8, knobWidth, labelHeight, uiTextSize, "Scaling");
  addTextKnob(
    left1, top8, knobWidth, labelHeight, uiTextSize, ID::notePitchToDelayTime,
    Scales::bipolarScale, false, 5);
  addLabel(left0, top9, knobWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    left1, top9, knobWidth, labelHeight, uiTextSize, ID::noteReleaseSeconds,
    Scales::parameterSmoothingSecond, false, 5);

  // Misc.
  addGroupLabel(left2, top6, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Misc.");

  addLabel(left2, top7, knobWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    left3, top7, knobWidth, labelHeight, uiTextSize, ID::parameterSmoothingSecond,
    Scales::parameterSmoothingSecond, false, 5);
  addLabel(left2, top8, knobWidth, labelHeight, uiTextSize, "Oversampling", kCenterText);
  std::vector<std::string> oversamplingItems{"1x", "16x Halfway", "16x"};
  addOptionMenu<Style::warning>(
    left3, top8, knobWidth, labelHeight, uiTextSize, ID::oversampling, oversamplingItems);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = int(1.75 * knobWidth) + 2 * margin;
  constexpr auto splashHeight = labelHeight;
  constexpr auto splashTop = top9;
  constexpr auto splashLeft = left2 + int(0.25 * knobWidth);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "CombDistortion");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
