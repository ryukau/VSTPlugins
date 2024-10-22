// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
constexpr float halfKnobWidth = int(knobWidth / 2);
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 100.0f;
constexpr float labelX = labelWidth + margin;

constexpr float barboxWidth = 512.0f;
constexpr float barboxHeight = 200.0f;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 4 * knobX - 2 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 9 * labelY + 3 * knobY - 4 * margin);

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

  // Shifter.
  constexpr auto psTop0 = top0;
  constexpr auto psTop1 = psTop0 + labelY;
  constexpr auto psTop2 = psTop1 + knobY;
  constexpr auto psTop3 = psTop2 + labelY;
  constexpr auto psTop4 = psTop3 + labelY;
  constexpr auto psLeft0 = left0;
  constexpr auto psLeft1 = psLeft0 + knobX;
  constexpr auto psLeft2 = psLeft1 + knobX;
  constexpr auto psLeft3 = psLeft2 + knobX;
  constexpr auto psLabelWidth = int(1.5 * knobX) - margin;
  constexpr auto psLabelLeft1 = psLeft0 + psLabelWidth;
  addGroupLabel(
    psLeft0, psTop0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "Shifter");

  addKnob<Style::accent>(
    psLeft0, psTop1, knobWidth, margin, uiTextSize, "Dry", ID::dryGain);
  addKnob<Style::accent>(
    psLeft1, psTop1, knobWidth, margin, uiTextSize, "Wet", ID::wetGain);
  addKnob(psLeft2, psTop1, knobWidth, margin, uiTextSize, "Feedback", ID::feedback);

  addLabel(psLeft0, psTop2, psLabelWidth, labelHeight, uiTextSize, "Delay Time [s]");
  addTextKnob(
    psLabelLeft1, psTop2, psLabelWidth, labelHeight, uiTextSize, ID::delayTimeSeconds,
    Scales::delayTimeSeconds, false, 5);
  addLabel(psLeft0, psTop3, psLabelWidth, labelHeight, uiTextSize, "Shift [oct.]");
  addTextKnob(
    psLabelLeft1, psTop3, psLabelWidth, labelHeight, uiTextSize, ID::shiftPitch,
    Scales::shiftPitch, false, 5);
  addLabel(psLeft0, psTop4, psLabelWidth, labelHeight, uiTextSize, "Shift [Hz]");
  addTextKnob(
    psLabelLeft1, psTop4, psLabelWidth, labelHeight, uiTextSize, ID::shiftHz,
    Scales::shiftHz, false, 5);

  constexpr auto psFilterMargin = int(halfKnobWidth * 1 / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop1, halfKnobWidth, labelHeight, uiTextSize, "HP");
  addSmallKnob(
    psLeft3 + psFilterMargin, psTop1 + labelHeight, labelHeight, labelHeight,
    ID::highpassHz);
  addLabel(psLeft3 + halfKnobWidth, psTop1, halfKnobWidth, labelHeight, uiTextSize, "LP");
  addSmallKnob(
    psLeft3 + psFilterMargin + halfKnobWidth, psTop1 + labelHeight, labelHeight,
    labelHeight, ID::lowpassHz);

  constexpr auto psSmallLeft = psLeft3 + int(knobWidth / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop2 - labelY, knobWidth, labelHeight, uiTextSize, "LFO");
  addSmallKnob(psSmallLeft, psTop2, labelHeight, labelHeight, ID::lfoToPrimaryDelayTime);
  addSmallKnob(psSmallLeft, psTop3, labelHeight, labelHeight, ID::lfoToPrimaryShiftPitch);
  addSmallKnob(psSmallLeft, psTop4, labelHeight, labelHeight, ID::lfoToPrimaryShiftHz);

  // LFO.
  constexpr auto lfoTop0 = psTop0 + 4 * labelY + knobY;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + knobY;
  constexpr auto lfoLeft0 = psLeft0;
  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX;
  constexpr auto lfoLeft3 = lfoLeft2 + knobX;
  constexpr auto lfoLabelLeft1 = lfoLeft0 + psLabelWidth;
  addGroupLabel(
    lfoLeft0, lfoTop0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "LFO");

  addRotaryKnob(
    lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "L-R Offset", ID::lfoPhaseOffset);
  addRotaryKnob(
    lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Phase", ID::lfoPhaseConstant);
  addKnob(lfoLeft2, lfoTop1, knobWidth, margin, uiTextSize, "Clip", ID::lfoShapeClip);
  addKnob(lfoLeft3, lfoTop1, knobWidth, margin, uiTextSize, "Skew", ID::lfoShapeSkew);

  constexpr auto lfoTopSync = lfoTop2 + labelY;
  addCheckbox(
    lfoLeft0 + halfKnobWidth, lfoTopSync, knobWidth, labelHeight, uiTextSize, "Sync.",
    ID::lfoTempoSync);
  addTextKnob(
    lfoLeft1 + halfKnobWidth, lfoTopSync - 2 * margin, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoUpper, Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft1 + halfKnobWidth, lfoTopSync + 2 * margin, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoLower, Scales::lfoTempoLower, false, 0, 1);
  addKnob(
    lfoLeft2 + halfKnobWidth, lfoTop2, knobWidth, margin, uiTextSize, "Rate",
    ID::lfoRate);

  // Misc.
  constexpr auto miscTop0 = lfoTop0 + labelY + 2 * knobY;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscLeft0 = left0;
  constexpr auto miscLeft1 = miscLeft0 + labelWidth;
  addLabel(miscLeft0, miscTop0, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop0, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);

  addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "Oversampling");
  std::vector<std::string> oversamplingItems{"1x", "2x", "8x"};
  addOptionMenu<Style::warning>(
    miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize, ID::oversampling,
    oversamplingItems);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = 2 * knobX - 2 * margin;
  constexpr auto splashHeight = labelY;
  constexpr auto splashTop = miscTop0 + 2 * labelY + 2 * margin;
  constexpr auto splashLeft = left0 + knobX;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "NarrowingDelay");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
