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
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Allpass.
  const auto apTop0 = top0;
  const auto apTop1 = apTop0 + labelY;
  const auto apTop2 = apTop1 + knobY;
  const auto apTop3 = apTop2 + knobY + 2 * margin;
  const auto apTop4 = apTop3 + labelY;
  const auto apTop5 = apTop4 + labelY;
  const auto apTop6 = apTop5 + labelY;
  const auto apTop7 = apTop6 + labelY;
  const auto apTop8 = apTop7 + labelY;

  const auto apLeft0 = left0;
  const auto apLeft1 = apLeft0 + knobX;
  const auto apLeft2 = apLeft1 + knobX;

  const auto apSectionHalfWidth = int(1.5 * knobX) - margin;
  const auto apLeftHalf = apLeft0 + apSectionHalfWidth;

  addGroupLabel(
    apLeft0, apTop0, 3 * knobX - 2 * margin, labelHeight, uiTextSize, "Allpass");

  addKnob<Style::accent>(
    apLeft0, apTop1, knobWidth, margin, uiTextSize, "Output", ID::outputGain);
  addKnob<Style::warning>(apLeft1, apTop1, knobWidth, margin, uiTextSize, "Mix", ID::mix);
  addKnob<Style::warning>(
    apLeft2, apTop1, knobWidth, margin, uiTextSize, "Feedback", ID::feedback);

  addKnob(apLeft0, apTop2, knobWidth, margin, uiTextSize, "Delay", ID::delayTimeSeconds);
  addKnob(
    apLeft1, apTop2, knobWidth, margin, uiTextSize, "LFO>Time", ID::lfoToDelayAmount);
  addKnob(apLeft2, apTop2, knobWidth, margin, uiTextSize, "Cut Spread", ID::cutoffSpread);

  addLabel(apLeft0, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, "Min [Hz]");
  addTextKnob(
    apLeftHalf, apTop3, apSectionHalfWidth, labelHeight, uiTextSize, ID::cutoffMinHz,
    Scales::cutoffHz, false, 5);
  addLabel(apLeft0, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, "Max [Hz]");
  addTextKnob(
    apLeftHalf, apTop4, apSectionHalfWidth, labelHeight, uiTextSize, ID::cutoffMaxHz,
    Scales::cutoffHz, false, 5);
  addLabel(apLeft0, apTop5, apSectionHalfWidth, labelHeight, uiTextSize, "AM");
  addTextKnob(
    apLeftHalf, apTop5, apSectionHalfWidth, labelHeight, uiTextSize,
    ID::inputToFeedbackGain, Scales::defaultScale, false, 5);
  addLabel(apLeft0, apTop6, apSectionHalfWidth, labelHeight, uiTextSize, "FM");
  addTextKnob(
    apLeftHalf, apTop6, apSectionHalfWidth, labelHeight, uiTextSize, ID::inputToDelayTime,
    Scales::defaultScale, false, 5);
  addLabel(apLeft0, apTop7, apSectionHalfWidth, labelHeight, uiTextSize, "Delay Tuning");
  std::vector<std::string> lfoToDelayTuningTypeItems{
    "Exp. Mul.", "Linear Mul.", "Add", "Fill Lower", "Fill Higher"};
  addOptionMenu(
    apLeftHalf, apTop7, apSectionHalfWidth, labelHeight, uiTextSize,
    ID::lfoToDelayTuningType, lfoToDelayTuningTypeItems);
  addLabel(apLeft0, apTop8, apSectionHalfWidth, labelHeight, uiTextSize, "Stage");
  addTextKnob(
    apLeftHalf, apTop8, apSectionHalfWidth, labelHeight, uiTextSize, ID::stage,
    Scales::stage, false, 0, 1);

  // LFO.
  const auto lfoTop0 = top0;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + 5 * margin;
  const auto lfoTop3 = lfoTop1 + knobY;
  const auto lfoLeft0 = apLeft2 + knobX + 2 * margin;
  const auto lfoLeft1 = lfoLeft0 + knobX;
  const auto lfoLeft2 = lfoLeft1 + knobX + 2 * margin;
  const auto lfoLeft3 = lfoLeft2 + knobX;
  const auto lfoLeft4 = lfoLeft3 + knobX;
  const auto lfoLeft5 = lfoLeft4 + knobX + 3 * margin;
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
  const auto miscTop0 = lfoTop0 + labelY + knobY + barboxHeight + 4 * margin;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscLeft0 = lfoLeft0 + int(0.25 * knobX);
  const auto miscLeft1 = miscLeft0 + labelWidth;
  const auto miscLeft2 = miscLeft1 + labelWidth + 4 * margin;
  const auto miscLeft3 = miscLeft2 + labelWidth;
  const auto smallLabelWidth = int(0.75 * labelWidth);
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
  const auto splashMargin = uiMargin;
  const auto splashWidth = int(1.75 * labelWidth);
  const auto splashHeight = labelY;
  const auto splashTop = miscTop1;
  const auto splashLeft = defaultWidth - uiMargin - splashWidth;
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
