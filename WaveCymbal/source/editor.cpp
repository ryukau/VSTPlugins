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
  const auto left0 = 2 * margin;
  const auto top0 = uiMargin;

  const auto leftGain = left0 + 2 * margin;
  addVSlider(
    leftGain, top0, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize, "Gain",
    ID::gain);

  // Excitation.
  const auto leftExcitation = leftGain + sliderX + 2 * margin;
  addToggleButton(
    leftExcitation, top0, 2 * knobX, labelHeight, midTextSize, "Excitation",
    ID::excitation);

  const auto topExcitation = top0 + labelHeight + margin;
  addKnob<Style::warning>(
    leftExcitation, topExcitation, knobWidth, margin, uiTextSize, "Feedback",
    ID::pickCombFeedback);
  addKnob(
    leftExcitation + knobX, topExcitation, knobWidth, margin, uiTextSize, "Time",
    ID::pickCombTime);

  // Objects.
  const auto leftObjects = leftExcitation + 2 * knobX + 4 * margin;
  addGroupLabel(leftObjects, top0, 2 * knobX, labelHeight, midTextSize, "Objects");

  const auto topObjects = top0 + labelHeight + margin;
  addNumberKnob(
    leftObjects, topObjects, knobWidth, margin, uiTextSize, "nCymbal", ID::nCymbal,
    Scales::nCymbal, 1);
  addNumberKnob(
    leftObjects + knobX, topObjects, knobWidth, margin, uiTextSize, "nString", ID::stack,
    Scales::stack, 1);

  // Wave.
  const auto leftWave = leftObjects + 2 * knobX + 4 * margin;
  addGroupLabel(leftWave, top0, 3 * knobX, labelHeight, midTextSize, "Wave");

  const auto topWave = top0 + labelHeight + margin;
  addKnob<Style::warning>(
    leftWave, topWave, knobWidth, margin, uiTextSize, "Damping", ID::damping);
  addKnob(
    leftWave + knobX, topWave, knobWidth, margin, uiTextSize, "PulsePosition",
    ID::pulsePosition);
  addKnob(
    leftWave + 2 * knobX, topWave, knobWidth, margin, uiTextSize, "PulseWidth",
    ID::pulseWidth);

  // Collision.
  const auto top1 = top0 + knobY + 3 * margin;

  const auto leftCollision = leftExcitation;
  addToggleButton(
    leftCollision, top1, checkboxWidth, labelHeight, midTextSize, "Collision",
    ID::collision);

  const auto topCollision = top1 + labelHeight + margin;
  addKnob<Style::warning>(
    leftCollision, topCollision, knobWidth, margin, uiTextSize, "Distance", ID::distance);

  // Random.
  const auto leftRandom = leftCollision + knobX + 4 * margin;
  addGroupLabel(leftRandom, top1, 2 * knobX, labelHeight, midTextSize, "Random");

  const auto topRandom = top1 + labelHeight + margin;
  addNumberKnob(
    leftRandom, topRandom, knobWidth, margin, uiTextSize, "Seed", ID::seed, Scales::seed,
    0);
  addKnob<Style::warning>(
    leftRandom + knobX, topRandom, knobWidth, margin, uiTextSize, "Amount",
    ID::randomAmount);

  // String.
  const auto leftString = leftRandom + 2 * knobX + 4 * margin;
  addGroupLabel(leftString, top1, 4 * knobX, labelHeight, midTextSize, "String");

  const auto topString = top1 + labelHeight + margin;
  addKnob(
    leftString, topString, knobWidth, margin, uiTextSize, "MinHz", ID::minFrequency);
  addKnob(
    leftString + knobX, topString, knobWidth, margin, uiTextSize, "MaxHz",
    ID::maxFrequency);
  addKnob(
    leftString + 2 * knobX, topString, knobWidth, margin, uiTextSize, "Decay", ID::decay);
  addKnob(
    leftString + 3 * knobX, topString, knobWidth, margin, uiTextSize, "Q", ID::bandpassQ);

  // Oscillator.
  const auto top2 = top1 + labelHeight + knobY;

  addGroupLabel(
    leftExcitation, top2, 3 * knobX + 2 * margin, labelHeight, midTextSize, "Oscillator");

  const auto topOscillator = top2 + labelHeight + margin;
  addCheckbox(
    leftExcitation, topOscillator, checkboxWidth, labelHeight, uiTextSize, "Retrigger",
    ID::retrigger);

  std::vector<std::string> itemOscType
    = {"Off", "Impulse", "Sustain", "Velvet Noise", "Brown Noise"};
  addOptionMenu(
    leftExcitation + knobX, topOscillator, checkboxWidth, labelHeight, uiTextSize,
    ID::oscType, itemOscType);

  std::vector<std::string> itemCutoffMap = {"Log", "Linear"};
  addOptionMenu(
    leftExcitation + 2 * knobX + 2 * margin, topOscillator, checkboxWidth, labelHeight,
    uiTextSize, ID::cutoffMap, itemCutoffMap);

  // Smoothness.
  const auto leftSmoothness = leftExcitation + 3 * knobX + 4 * margin;
  addKnob(
    int(leftSmoothness + 22.5), top2 - margin, 10 * margin, margin, uiTextSize,
    "Smoothness", ID::smoothness);

  // Plugin name.
  const auto splashWidth = 3 * knobX;
  addSplashScreen(
    defaultWidth - uiMargin - splashWidth, top2 + 2 * margin, splashWidth, 2 * uiMargin,
    knobWidth + uiMargin, uiMargin, defaultWidth - 2 * (knobWidth + uiMargin),
    defaultHeight - 2 * uiMargin, pluginNameTextSize, "WaveCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
