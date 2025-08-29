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

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  const auto gainRange = Scales::gain.getRangeDB();

  // Misc.
  const auto miscTop0 = top0;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscTop2 = miscTop1 + labelY;
  const auto miscTop3 = miscTop2 + labelY;
  const auto miscTop4 = miscTop3 + labelY;
  const auto miscTop5 = miscTop4 + labelY;
  const auto miscLeft0 = left0;
  const auto miscLeft1 = miscLeft0 + labelWidth + 2 * margin;

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

  const auto inputTop0 = top0;
  const auto inputTop1 = inputTop0 + labelY;
  const auto inputTop2 = inputTop1 + labelY;
  const auto inputTop3 = inputTop2 + labelY;
  const auto inputLeft0 = miscLeft0 + 2 * labelWidth + 6 * margin;
  const auto inputLeft1 = inputLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    inputLeft0, inputTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Input");

  addLabel(inputLeft0, inputTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  auto inputGainTextKnob = addTextKnob(
    inputLeft1, inputTop1, labelWidth, labelHeight, uiTextSize, ID::inputGain,
    Scales::gain, true, 5);
  if (inputGainTextKnob) {
    inputGainTextKnob->sensitivity = 0.1 / gainRange;
    inputGainTextKnob->lowSensitivity = 0.01 / gainRange;
    inputGainTextKnob->wheelSensitivity = 0.1 / gainRange;
  }
  addLabel(inputLeft0, inputTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  auto inputAttackTextKnob = addTextKnob<Style::warning>(
    inputLeft1, inputTop2, labelWidth, labelHeight, uiTextSize,
    ID::inputLimiterAttackSeconds, Scales::limiterAttackSeconds, false, 5);
  if (inputAttackTextKnob) {
    inputAttackTextKnob->liveUpdate = false;
  }
  addLabel(inputLeft0, inputTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    inputLeft1, inputTop3, labelWidth, labelHeight, uiTextSize,
    ID::inputLimiterReleaseSeconds, Scales::limiterReleaseSeconds, false, 5);

  const auto sideTop0 = top0;
  const auto sideTop1 = sideTop0 + labelY;
  const auto sideTop2 = sideTop1 + labelY;
  const auto sideTop3 = sideTop2 + labelY;
  const auto sideLeft0 = inputLeft0 + 2 * labelWidth + 6 * margin;
  const auto sideLeft1 = sideLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    sideLeft0, sideTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Side");

  addLabel(sideLeft0, sideTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  auto sideGainTextKnob = addTextKnob(
    sideLeft1, sideTop1, labelWidth, labelHeight, uiTextSize, ID::sideGain, Scales::gain,
    true, 5);
  if (sideGainTextKnob) {
    sideGainTextKnob->sensitivity = 0.1 / gainRange;
    sideGainTextKnob->lowSensitivity = 0.01 / gainRange;
    sideGainTextKnob->wheelSensitivity = 0.1 / gainRange;
  }
  addLabel(sideLeft0, sideTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  auto sideAttackTextKnob = addTextKnob<Style::warning>(
    sideLeft1, sideTop2, labelWidth, labelHeight, uiTextSize,
    ID::sideLimiterAttackSeconds, Scales::limiterAttackSeconds, false, 5);
  if (sideAttackTextKnob) {
    sideAttackTextKnob->liveUpdate = false;
  }
  addLabel(sideLeft0, sideTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    sideLeft1, sideTop3, labelWidth, labelHeight, uiTextSize,
    ID::sideLimiterReleaseSeconds, Scales::limiterReleaseSeconds, false, 5);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashWidth = int(2 * labelWidth + 4 * margin);
  const auto splashHeight = labelHeight + margin;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = inputLeft1;
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
