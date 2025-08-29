// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
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
  const auto top1 = top0 + 3 * labelHeight + 2 * margin;
  const auto top2 = top1 + labelY;
  const auto top3 = top2 + labelY;
  const auto left0 = uiMargin;
  const auto left1 = left0 + labelX;

  addKickButton<Style::warning>(
    left0 + std::floor(labelWidth / 4), top0 + margin,
    std::floor(1.5f * labelWidth) + margin, 2 * labelHeight, uiTextSize, "Refresh FIR",
    ID::refreshFir);

  addLabel(left0, top1, labelWidth, labelHeight, uiTextSize, "Cutoff [Hz]", kLeftText);
  addTextKnob(
    left1, top1, labelWidth, labelHeight, uiTextSize, ID::cutoffHz, Scales::cutoffHz,
    false, 5);
  addLabel(left0, top2, labelWidth, labelHeight, uiTextSize, "HP Gain [dB]", kLeftText);
  auto highpassGainKnob = addTextKnob(
    left1, top2, labelWidth, labelHeight, uiTextSize, ID::highpassGain, Scales::gain,
    true, 5);
  if (highpassGainKnob) {
    highpassGainKnob->sensitivity = 0.2f / 289.0f;
    highpassGainKnob->lowSensitivity = 0.001f / 289.0f;
    highpassGainKnob->wheelSensitivity = 0.1f / 289.0f;
  }
  addLabel(left0, top3, labelWidth, labelHeight, uiTextSize, "LP Gain [dB]", kLeftText);
  auto lowpassGainKnob = addTextKnob(
    left1, top3, labelWidth, labelHeight, uiTextSize, ID::lowpassGain, Scales::gain, true,
    5);
  if (lowpassGainKnob) {
    lowpassGainKnob->sensitivity = 0.4f / 289.0f;
    lowpassGainKnob->lowSensitivity = 0.001f / 289.0f;
    lowpassGainKnob->wheelSensitivity = 0.1f / 289.0f;
  }

  // Plugin name.
  const auto splashMargin = 2 * margin;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "MiniCliffEQ");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
