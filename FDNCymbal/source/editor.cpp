// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 24.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 80.0f;
constexpr float knobHeight = knobWidth - 2.0f * margin;
constexpr float knobRightMargin = 0.0f;
constexpr float knobX = knobWidth; // With margin.
constexpr float knobY = knobHeight + labelY + 2.0f * margin;
constexpr float sliderWidth = 70.0f;
constexpr float sliderHeight = int(2.0f * (knobHeight + labelY) + 67.5f);
constexpr float sliderX = 80.0f;
constexpr float sliderY = sliderHeight + labelY;
constexpr float checkboxWidth = 80.0f;
constexpr uint32_t defaultWidth = uint32_t(40 + sliderX + 7 * knobX + 6 * margin);
constexpr uint32_t defaultHeight
  = uint32_t(20 + 3 * knobY + 3 * labelHeight + 3 * margin);

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

  // Gain.
  const auto left0 = 20.0f;
  const auto top0 = 20.0f;
  const auto smallKnobWidth = 50.0f;

  addVSlider(
    left0, top0, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize, "Gain",
    ID::gain);

  // Stick.
  const auto leftStick0 = left0 + sliderX + 2.0f * margin;
  const auto leftStick1 = leftStick0 + knobX + margin;
  addToggleButton(
    leftStick0, top0, 2.0f * knobX, labelHeight, midTextSize, "Stick", ID::stick);

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
  const auto leftRandom = leftStick0 + 2.0f * knobX + 2.0f * margin;
  addGroupLabel(leftRandom, top0, 2.0f * knobX, labelHeight, midTextSize, "Random");

  const auto topRandom = top0 + labelHeight;
  addNumberKnob(
    leftRandom, topRandom + margin, knobWidth, margin, uiTextSize, "Seed", ID::seed,
    Scales::seed);
  addLabel(
    leftRandom + knobX, topRandom, knobWidth, labelHeight, uiTextSize, "Retrigger");
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + labelHeight, checkboxWidth,
    labelHeight, uiTextSize, "Time", ID::retriggerTime);
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + 2.0f * labelHeight, checkboxWidth,
    labelHeight, uiTextSize, "Stick", ID::retriggerStick);
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + 3.0f * labelHeight, checkboxWidth,
    labelHeight, uiTextSize, "Tremolo", ID::retriggerTremolo);

  // FDN.
  const auto leftFDN = leftRandom + 2.0f * knobX + 2.0f * margin;
  addToggleButton(leftFDN, top0, 3.0f * knobX, labelHeight, midTextSize, "FDN", ID::fdn);

  const auto topFDN = top0 + labelHeight + margin;
  addKnob(leftFDN, topFDN, knobWidth, margin, uiTextSize, "Time", ID::fdnTime);
  addKnob<Style::warning>(
    leftFDN + knobX, topFDN, knobWidth, margin, uiTextSize, "Feedback", ID::fdnFeedback);
  addKnob(
    leftFDN + 2.0f * knobX, topFDN, knobWidth, margin, uiTextSize, "CascadeMix",
    ID::fdnCascadeMix);

  // Allpass.
  const auto top1 = top0 + knobY + labelHeight + margin;
  const auto leftAP = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftAP, top1, knobX, labelHeight, midTextSize, "Allpass");

  const auto topAP = top1 + labelHeight + margin;
  addKnob(leftAP, topAP, knobWidth, margin, uiTextSize, "Mix", ID::allpassMix);

  const auto leftAP1 = leftAP + knobX + 2.0f * margin;
  addGroupLabel(leftAP1, top1, 3.0f * knobX, labelHeight, midTextSize, "Stage 1");
  addCheckbox(
    floor(leftAP1 + knobX + 3.5 * margin),
    floor(topAP + knobHeight + labelHeight + 0.5f * margin), checkboxWidth, labelHeight,
    uiTextSize, "Tanh", ID::allpass1Saturation);

  addKnob(leftAP1, topAP, knobWidth, margin, uiTextSize, "Time", ID::allpass1Time);
  addKnob(
    leftAP1 + knobX, topAP, knobWidth, margin, uiTextSize, "Feedback",
    ID::allpass1Feedback);
  addKnob<Style::warning>(
    leftAP1 + 2.0f * knobX, topAP, knobWidth, margin, uiTextSize, "HP Cutoff",
    ID::allpass1HighpassCutoff);

  const auto leftAP2 = leftAP1 + 3.0f * knobX + 2.0f * margin;
  addGroupLabel(leftAP2, top1, 3.0f * knobX, labelHeight, midTextSize, "Stage 2");

  addKnob(leftAP2, topAP, knobWidth, margin, uiTextSize, "Time", ID::allpass2Time);
  addKnob(
    leftAP2 + knobX, topAP, knobWidth, margin, uiTextSize, "Feedback",
    ID::allpass2Feedback);
  addKnob<Style::warning>(
    leftAP2 + 2.0f * knobX, topAP, knobWidth, margin, uiTextSize, "HP Cutoff",
    ID::allpass2HighpassCutoff);

  // Smooth.
  const auto top2 = top1 + knobY + labelHeight + margin;
  addKnob(
    left0 - margin, top2 + labelHeight + margin, sliderX, margin, uiTextSize, "Smooth",
    ID::smoothness);

  // Tremolo.
  const auto leftTremolo = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftTremolo, top2, 4.0f * knobX, labelHeight, midTextSize, "Tremolo");

  const auto topTremolo = top2 + labelHeight + margin;
  addKnob(leftTremolo, topTremolo, knobWidth, margin, uiTextSize, "Mix", ID::tremoloMix);
  addKnob(
    leftTremolo + knobX, topTremolo, knobWidth, margin, uiTextSize, "Depth",
    ID::tremoloDepth);
  addKnob(
    leftTremolo + 2.0f * knobX, topTremolo, knobWidth, margin, uiTextSize, "Frequency",
    ID::tremoloFrequency);
  addKnob(
    leftTremolo + 3.0f * knobX, topTremolo, knobWidth, margin, uiTextSize, "DelayTime",
    ID::tremoloDelayTime);

  const auto leftTremoloRandom = leftTremolo + 4.0f * knobX + 2.0f * margin;
  addGroupLabel(
    leftTremoloRandom, top2, 3.0f * knobX + 2.0f * margin, labelHeight, midTextSize,
    "Random");
  addKnob(
    leftTremoloRandom, topTremolo - 1.5f * margin, 50.0f, margin, uiTextSize, "Depth",
    ID::randomTremoloDepth, LabelPosition::right, knobRightMargin);
  addKnob(
    leftTremoloRandom + 1.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    margin, uiTextSize, "Freq", ID::randomTremoloFrequency, LabelPosition::right,
    knobRightMargin);
  addKnob(
    leftTremoloRandom + 2.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    margin, uiTextSize, "Time", ID::randomTremoloDelayTime, LabelPosition::right,
    knobRightMargin);

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  const auto splashHeight = 40.0f;
  addSplashScreen(
    defaultWidth - 20.0f - splashWidth, defaultHeight - 20.0f - splashHeight, splashWidth,
    splashHeight, 20.0f, 20.0f, defaultWidth - 40.0f, defaultHeight - 40.0f,
    pluginNameTextSize, "FDNCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
