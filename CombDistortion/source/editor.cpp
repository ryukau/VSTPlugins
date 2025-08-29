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

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top1 = uiMargin;
  const auto top2 = top1 + knobY + 2 * margin;
  const auto top3 = top2 + labelY;
  const auto top4 = top3 + labelY;
  const auto top5 = top4 + labelY;
  const auto top6 = top5 + labelY;
  const auto top7 = top6 + labelY;
  const auto top8 = top7 + labelY;
  const auto top9 = top8 + labelY;

  const auto left0 = uiMargin;
  const auto left1 = left0 + knobX;
  const auto left2 = left1 + knobX;
  const auto left3 = left2 + knobX;

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
  const auto splashMargin = uiMargin;
  const auto splashWidth = int(1.75 * knobWidth) + 2 * margin;
  const auto splashHeight = labelHeight;
  const auto splashTop = top9;
  const auto splashLeft = left2 + int(0.25 * knobWidth);
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
