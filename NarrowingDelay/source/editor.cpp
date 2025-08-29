// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

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

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Shifter.
  const auto psTop0 = top0;
  const auto psTop1 = psTop0 + labelY;
  const auto psTop2 = psTop1 + knobY;
  const auto psTop3 = psTop2 + labelY;
  const auto psTop4 = psTop3 + labelY;
  const auto psLeft0 = left0;
  const auto psLeft1 = psLeft0 + knobX;
  const auto psLeft2 = psLeft1 + knobX;
  const auto psLeft3 = psLeft2 + knobX;
  const auto psLabelWidth = int(1.5 * knobX) - margin;
  const auto psLabelLeft1 = psLeft0 + psLabelWidth;
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

  const auto psFilterMargin = int(halfKnobWidth * 1 / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop1, halfKnobWidth, labelHeight, uiTextSize, "HP");
  addSmallKnob(
    psLeft3 + psFilterMargin, psTop1 + labelHeight, labelHeight, labelHeight,
    ID::highpassHz);
  addLabel(psLeft3 + halfKnobWidth, psTop1, halfKnobWidth, labelHeight, uiTextSize, "LP");
  addSmallKnob(
    psLeft3 + psFilterMargin + halfKnobWidth, psTop1 + labelHeight, labelHeight,
    labelHeight, ID::lowpassHz);

  const auto psSmallLeft = psLeft3 + int(knobWidth / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop2 - labelY, knobWidth, labelHeight, uiTextSize, "LFO");
  addSmallKnob(psSmallLeft, psTop2, labelHeight, labelHeight, ID::lfoToPrimaryDelayTime);
  addSmallKnob(psSmallLeft, psTop3, labelHeight, labelHeight, ID::lfoToPrimaryShiftPitch);
  addSmallKnob(psSmallLeft, psTop4, labelHeight, labelHeight, ID::lfoToPrimaryShiftHz);

  // LFO.
  const auto lfoTop0 = psTop0 + 4 * labelY + knobY;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + knobY;
  const auto lfoLeft0 = psLeft0;
  const auto lfoLeft1 = lfoLeft0 + knobX;
  const auto lfoLeft2 = lfoLeft1 + knobX;
  const auto lfoLeft3 = lfoLeft2 + knobX;
  addGroupLabel(
    lfoLeft0, lfoTop0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "LFO");

  addRotaryKnob(
    lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "L-R Offset", ID::lfoPhaseOffset);
  addRotaryKnob(
    lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Phase", ID::lfoPhaseConstant);
  addKnob(lfoLeft2, lfoTop1, knobWidth, margin, uiTextSize, "Clip", ID::lfoShapeClip);
  addKnob(lfoLeft3, lfoTop1, knobWidth, margin, uiTextSize, "Skew", ID::lfoShapeSkew);

  const auto lfoTopSync = lfoTop2 + labelY;
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
  const auto miscTop0 = lfoTop0 + labelY + 2 * knobY;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscLeft0 = left0;
  const auto miscLeft1 = miscLeft0 + labelWidth;
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
  const auto splashMargin = uiMargin;
  const auto splashWidth = 2 * knobX - 2 * margin;
  const auto splashHeight = labelY;
  const auto splashTop = miscTop0 + 2 * labelY + 2 * margin;
  const auto splashLeft = left0 + knobX;
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
