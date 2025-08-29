// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

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

  // Gain.
  const auto left0 = uiMargin;
  const auto top0 = uiMargin;
  const auto smallKnobWidth = 10 * margin;

  addVSlider(
    left0, top0, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize, "Gain",
    ID::gain);

  // Stick.
  const auto leftStick0 = left0 + sliderX + 2 * margin;
  const auto leftStick1 = leftStick0 + knobX + margin;
  addToggleButton(
    leftStick0, top0, 2 * knobX, labelHeight, midTextSize, "Stick", ID::stick);

  const auto topStick = top0 + labelHeight + margin;
  addKnob(
    leftStick0, topStick, smallKnobWidth, margin, uiTextSize, "Pulse", ID::stickPulseMix,
    LabelPosition::right, knobRightMargin);
  addKnob(
    leftStick0, topStick + smallKnobWidth, smallKnobWidth, margin, uiTextSize, "Velvet",
    ID::stickVelvetMix, LabelPosition::right, knobRightMargin);
  addKnob(
    leftStick1, topStick, smallKnobWidth, margin, uiTextSize, "Tone", ID::stickToneMix,
    LabelPosition::right, knobRightMargin);
  addKnob(
    leftStick1, topStick + smallKnobWidth, smallKnobWidth, margin, uiTextSize, "Decay",
    ID::stickDecay, LabelPosition::right, knobRightMargin);

  // Random.
  const auto leftRandom = leftStick0 + 2 * knobX + 2 * margin;
  addGroupLabel(leftRandom, top0, 2 * knobX, labelHeight, midTextSize, "Random");

  const auto topRandom = top0 + labelHeight;
  addNumberKnob(
    leftRandom, topRandom + margin, knobWidth, margin, uiTextSize, "Seed", ID::seed,
    Scales::seed);
  addLabel(
    leftRandom + knobX, topRandom, knobWidth, labelHeight, uiTextSize, "Retrigger");
  addCheckbox(
    leftRandom + knobX + 2 * margin, topRandom + labelHeight, checkboxWidth, labelHeight,
    uiTextSize, "Time", ID::retriggerTime);
  addCheckbox(
    leftRandom + knobX + 2 * margin, topRandom + 2 * labelHeight, checkboxWidth,
    labelHeight, uiTextSize, "Stick", ID::retriggerStick);
  addCheckbox(
    leftRandom + knobX + 2 * margin, topRandom + 3 * labelHeight, checkboxWidth,
    labelHeight, uiTextSize, "Tremolo", ID::retriggerTremolo);

  // FDN.
  const auto leftFDN = leftRandom + 2 * knobX + 2 * margin;
  addToggleButton(leftFDN, top0, 3 * knobX, labelHeight, midTextSize, "FDN", ID::fdn);

  const auto topFDN = top0 + labelHeight + margin;
  addKnob(leftFDN, topFDN, knobWidth, margin, uiTextSize, "Time", ID::fdnTime);
  addKnob<Style::warning>(
    leftFDN + knobX, topFDN, knobWidth, margin, uiTextSize, "Feedback", ID::fdnFeedback);
  addKnob(
    leftFDN + 2 * knobX, topFDN, knobWidth, margin, uiTextSize, "CascadeMix",
    ID::fdnCascadeMix);

  // Allpass.
  const auto top1 = top0 + knobY + labelHeight + margin;
  const auto leftAP = left0 + sliderX + 2 * margin;
  addGroupLabel(leftAP, top1, knobX, labelHeight, midTextSize, "Allpass");

  const auto topAP = top1 + labelHeight + margin;
  addKnob(leftAP, topAP, knobWidth, margin, uiTextSize, "Mix", ID::allpassMix);

  const auto leftAP1 = leftAP + knobX + 2 * margin;
  addGroupLabel(leftAP1, top1, 3 * knobX, labelHeight, midTextSize, "Stage 1");
  addCheckbox(
    floor(leftAP1 + knobX + 3.5 * margin),
    floor(topAP + knobHeight + labelHeight + 0.5f * margin), checkboxWidth, labelHeight,
    uiTextSize, "Tanh", ID::allpass1Saturation);

  addKnob(leftAP1, topAP, knobWidth, margin, uiTextSize, "Time", ID::allpass1Time);
  addKnob(
    leftAP1 + knobX, topAP, knobWidth, margin, uiTextSize, "Feedback",
    ID::allpass1Feedback);
  addKnob<Style::warning>(
    leftAP1 + 2 * knobX, topAP, knobWidth, margin, uiTextSize, "HP Cutoff",
    ID::allpass1HighpassCutoff);

  const auto leftAP2 = leftAP1 + 3 * knobX + 2 * margin;
  addGroupLabel(leftAP2, top1, 3 * knobX, labelHeight, midTextSize, "Stage 2");

  addKnob(leftAP2, topAP, knobWidth, margin, uiTextSize, "Time", ID::allpass2Time);
  addKnob(
    leftAP2 + knobX, topAP, knobWidth, margin, uiTextSize, "Feedback",
    ID::allpass2Feedback);
  addKnob<Style::warning>(
    leftAP2 + 2 * knobX, topAP, knobWidth, margin, uiTextSize, "HP Cutoff",
    ID::allpass2HighpassCutoff);

  // Smooth.
  const auto top2 = top1 + knobY + labelHeight + margin;
  addKnob(
    left0 - margin, top2 + labelHeight + margin, sliderX, margin, uiTextSize, "Smooth",
    ID::smoothness);

  // Tremolo.
  const auto leftTremolo = left0 + sliderX + 2 * margin;
  addGroupLabel(leftTremolo, top2, 4 * knobX, labelHeight, midTextSize, "Tremolo");

  const auto topTremolo = top2 + labelHeight + margin;
  addKnob(leftTremolo, topTremolo, knobWidth, margin, uiTextSize, "Mix", ID::tremoloMix);
  addKnob(
    leftTremolo + knobX, topTremolo, knobWidth, margin, uiTextSize, "Depth",
    ID::tremoloDepth);
  addKnob(
    leftTremolo + 2 * knobX, topTremolo, knobWidth, margin, uiTextSize, "Frequency",
    ID::tremoloFrequency);
  addKnob(
    leftTremolo + 3 * knobX, topTremolo, knobWidth, margin, uiTextSize, "DelayTime",
    ID::tremoloDelayTime);

  const auto leftTremoloRandom = leftTremolo + 4 * knobX + 2 * margin;
  addGroupLabel(
    leftTremoloRandom, top2, 3 * knobX + 2 * margin, labelHeight, midTextSize, "Random");
  addKnob(
    leftTremoloRandom, topTremolo - int(1.5 * margin), smallKnobWidth, margin, uiTextSize,
    "Depth", ID::randomTremoloDepth, LabelPosition::right, knobRightMargin);
  addKnob(
    leftTremoloRandom + 1 * (knobX + margin), topTremolo - int(1.5 * margin),
    smallKnobWidth, margin, uiTextSize, "Freq", ID::randomTremoloFrequency,
    LabelPosition::right, knobRightMargin);
  addKnob(
    leftTremoloRandom + 2 * (knobX + margin), topTremolo - int(1.5 * margin),
    smallKnobWidth, margin, uiTextSize, "Time", ID::randomTremoloDelayTime,
    LabelPosition::right, knobRightMargin);

  // Plugin name.
  const auto splashWidth = 3 * knobX;
  const auto splashHeight = 2 * uiMargin;
  addSplashScreen(
    defaultWidth - uiMargin - splashWidth, defaultHeight - uiMargin - splashHeight,
    splashWidth, splashHeight, uiMargin, uiMargin, defaultWidth - 2 * uiMargin,
    defaultHeight - 2 * uiMargin, pluginNameTextSize, "FDNCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
