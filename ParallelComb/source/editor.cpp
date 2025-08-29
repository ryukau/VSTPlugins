// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "gui/panicbutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto top1 = top0 + barboxHeight + 2 * margin;
  const auto top2 = top1 + barboxHeight + 2 * margin;
  const auto left0 = uiMargin;

  addBarBox(
    left0, top0, barboxWidth, barboxHeight, ID::delayTime0, nCombTaps, Scales::delayTime,
    "Delay Time [s]");

  const auto ctrlLeft1 = left0;
  const auto ctrlLeft2 = ctrlLeft1 + labelX;
  const auto ctrlLeft3 = ctrlLeft2 + labelX + 2 * margin;
  const auto ctrlLeft4 = ctrlLeft3 + labelX;
  const auto ctrlLeft5 = ctrlLeft4 + labelX + 3 * margin;
  const auto ctrlLeft6 = ctrlLeft5 + labelX;
  const auto ctrlLeft7 = ctrlLeft6 + labelX;
  const auto ctrlLeft8 = ctrlLeft7 + labelX;
  const auto ctrlTop1 = top1;
  const auto ctrlTop2 = ctrlTop1 + labelY;
  const auto ctrlTop3 = ctrlTop2 + labelY;
  const auto ctrlTop4 = ctrlTop3 + labelY;
  const auto ctrlTop5 = ctrlTop4 + labelY;
  const auto ctrlTop6 = ctrlTop5 + labelY;
  const auto ctrlTop7 = ctrlTop6 + labelY;

  addGroupLabel(
    ctrlLeft1, ctrlTop1, 4 * labelX + margin, labelHeight, uiTextSize, "Delay");
  addLabel(
    ctrlLeft1, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Time Multi.", kCenterText);
  auto textKnobTimeMultiplier = addTextKnob(
    ctrlLeft2, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::timeMultiplier,
    Scales::defaultScale, false, 5);
  if (textKnobTimeMultiplier) {
    textKnobTimeMultiplier->lowSensitivity = 0.00001;
    textKnobTimeMultiplier->wheelSensitivity = 0.001;
  }
  addLabel(
    ctrlLeft1, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Interp. Rate",
    kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeInterpRate,
    Scales::delayTimeInterpRate, false, 5);
  addLabel(
    ctrlLeft1, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Interp. LP [s]",
    kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop4, labelWidth, labelHeight, uiTextSize,
    ID::delayTimeInterpLowpassSeconds, Scales::delayTimeInterpLowpassSeconds, false, 5);

  addKnob(
    ctrlLeft1, ctrlTop5, labelWidth, margin, uiTextSize, "L-R Lean", ID::stereoLean);
  addKnob(ctrlLeft2, ctrlTop5, labelWidth, margin, uiTextSize, "Cross", ID::stereoCross);

  addLabel(
    ctrlLeft3, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Feedback", kCenterText);
  auto feedbackTextKnob = addTextKnob(
    ctrlLeft4, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::feedback,
    Scales::feedback, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]",
    kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop3, labelWidth, labelHeight, uiTextSize,
    ID::feedbackHighpassCutoffHz, Scales::feedbackHighpassCutoffHz, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Limiter R. [s]",
    kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::feedbackLimiterRelease,
    Scales::feedbackLimiterRelease, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop5, labelWidth, labelHeight, uiTextSize, "Self Mod.", kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop5, labelWidth, labelHeight, uiTextSize, ID::feedbackToDelayTime,
    Scales::feedbackToDelayTime, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop6, labelWidth, labelHeight, uiTextSize, "Gate Th. [dB]",
    kCenterText);
  addTextKnob<Style::warning>(
    ctrlLeft4, ctrlTop6, labelWidth, labelHeight, uiTextSize, ID::gateThreshold,
    Scales::gateThreshold, true, 5);
  addLabel(
    ctrlLeft3, ctrlTop7, labelWidth, labelHeight, uiTextSize, "Gate R. [s]", kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop7, labelWidth, labelHeight, uiTextSize, ID::gateRelease,
    Scales::gateRelease, false, 5);

  addGroupLabel(ctrlLeft5, ctrlTop1, 2 * labelX - margin, labelHeight, uiTextSize, "Mix");
  addLabel(
    ctrlLeft5, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Dry [dB]", kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::dry, Scales::dry, true,
    5);
  addLabel(
    ctrlLeft5, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Wet [dB]", kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::wet, Scales::wet, true,
    5);
  addLabel(
    ctrlLeft5, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Channel Type",
    kCenterText);
  std::vector<std::string> channelTypeItems{"L-R", "M-S"};
  addOptionMenu<Style::warning>(
    ctrlLeft6, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::channelType,
    channelTypeItems);
  addCheckbox(
    ctrlLeft5, ctrlTop5, 2 * labelWidth, labelHeight, uiTextSize, "16x OverSampling",
    ID::overSampling);

  // Panic button.
  const auto panicButtonLeft = ctrlLeft5 + 2 * margin;
  const auto panicButtonTop = ctrlTop6;
  auto panicButton = new PanicButton(
    CRect(
      panicButtonLeft, panicButtonTop, panicButtonLeft + 2 * labelWidth - 4 * margin,
      panicButtonTop + labelHeight),
    this, 0, "Panic!", getFont(uiTextSize), palette, this);
  frame->addView(panicButton);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = ctrlLeft5;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "ParallelComb");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
