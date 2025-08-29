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
  const auto top1 = top0 + knobY;
  const auto top2 = top1 + knobY;
  const auto top3 = top2 + labelY;
  const auto top4 = top3 + labelY;
  const auto top5 = top4 + labelY;
  const auto left0 = uiMargin;
  const auto left1 = left0 + 1 * knobX;
  const auto left2 = left0 + 2 * knobX;
  const auto left3 = left0 + 3 * knobX;

  addKnob(left0, top0, knobWidth, margin, uiTextSize, "Mix", ID::mix);
  addKnob(left1, top0, knobWidth, margin, uiTextSize, "Frequency", ID::frequencyHz);
  addKnob(left2, top0, knobWidth, margin, uiTextSize, "DC Offset", ID::dcOffset);
  addKnob(left3, top0, knobWidth, margin, uiTextSize, "Feedback", ID::feedbackGain);

  addKnob(
    left0, top1 + int(knobWidth / 4), int(knobWidth / 2), margin, uiTextSize, "Pre Clip",
    ID::preClipGain);
  addKnob(
    left0 + int(knobWidth / 2) + margin, top1 + int(knobWidth / 4), int(knobWidth / 2),
    margin, uiTextSize, "Out", ID::outputGain);
  addKnob<Style::warning>(
    left1, top1, knobWidth, margin, uiTextSize, "Mod. Scale", ID::modFrequencyScaling);
  addKnob(left2, top1, knobWidth, margin, uiTextSize, "Mod. Wrap", ID::modWrapMix);
  addKnob(left3, top1, knobWidth, margin, uiTextSize, "Hardclip", ID::hardclipMix);

  // Note.
  const auto noteLeft1 = left0 + int(knobWidth / 2);
  const auto negativeScaleOffset = 2 * margin;

  addGroupLabel(left0, top2, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Note");
  addToggleButton(
    left0, top3, int(knobWidth / 2) + negativeScaleOffset, labelHeight, uiTextSize,
    "Negative", ID::noteScalingNegative);
  addLabel(
    noteLeft1 + negativeScaleOffset, top3, int(knobWidth / 2) - negativeScaleOffset,
    labelHeight, uiTextSize, "Scale");
  addTextKnob(
    left1, top3, knobWidth, labelHeight, uiTextSize, ID::noteScaling, Scales::noteScaling,
    false, 5);
  addLabel(left0, top4, knobWidth, labelHeight, uiTextSize, "Offset [st.]");
  addTextKnob(
    left1, top4, knobWidth, labelHeight, uiTextSize, ID::noteOffset, Scales::noteOffset,
    false, 5);
  addLabel(left0, top5, knobWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    left1, top5, knobWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

  addLabel(left2, top3, knobWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    left3, top3, knobWidth, labelHeight, uiTextSize, ID::parameterSmoothingSecond,
    Scales::noteSlideTimeSecond, false, 5);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = defaultWidth - uiMargin - splashWidth - int(knobWidth / 4);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "UltrasonicRingMod");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
