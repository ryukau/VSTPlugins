// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
constexpr float knobWidth = 100.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = knobWidth;
constexpr float labelX = labelWidth + margin;
constexpr float splashWidth = int(1.5 * labelWidth) + margin;
constexpr float splashHeight = labelY;

constexpr float barboxWidth = 500.0f;
constexpr float barboxHeight = 160.0f;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 4 * knobX - 2 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 2 * knobY + 4 * labelY - 2 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
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

  constexpr auto top0 = uiMargin;
  constexpr auto top1 = top0 + knobY;
  constexpr auto top2 = top1 + knobY;
  constexpr auto top3 = top2 + labelY;
  constexpr auto top4 = top3 + labelY;
  constexpr auto top5 = top4 + labelY;
  constexpr auto left0 = uiMargin;
  constexpr auto left1 = left0 + 1 * knobX;
  constexpr auto left2 = left0 + 2 * knobX;
  constexpr auto left3 = left0 + 3 * knobX;

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
  constexpr auto noteLeft1 = left0 + int(knobWidth / 2);
  constexpr auto negativeScaleOffset = 2 * margin;

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
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = defaultHeight - uiMargin - splashHeight;
  constexpr auto splashLeft = defaultWidth - uiMargin - splashWidth - int(knobWidth / 4);
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
