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
constexpr float knobX = knobWidth; // With margin.
constexpr float knobY = knobHeight + labelY + 2.0f * margin;
constexpr float sliderWidth = 70.0f;
constexpr float sliderHeight = 2.0f * (knobHeight + labelY) + 75.0f;
constexpr float sliderX = 80.0f;
constexpr float sliderY = sliderHeight + labelY;
constexpr float checkboxWidth = 80.0f;
constexpr uint32_t defaultWidth = uint32_t(2 * 20 + sliderX + 7 * knobX + 10 * margin);
constexpr uint32_t defaultHeight = uint32_t(2 * 15 + 2 * knobY + 2 * labelHeight + 55);

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
  const auto left0 = 10.0f;
  const auto top0 = 20.0f;

  const auto leftGain = left0 + 2.0f * margin;
  addVSlider(
    leftGain, top0, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize, "Gain",
    ID::gain);

  // Excitation.
  const auto leftExcitation = leftGain + sliderX + 2.0f * margin;
  addToggleButton(
    leftExcitation, top0, 2.0f * knobX, labelHeight, midTextSize, "Excitation",
    ID::excitation);

  const auto topExcitation = top0 + labelHeight + margin;
  addKnob<Style::warning>(
    leftExcitation, topExcitation, knobWidth, margin, uiTextSize, "Feedback",
    ID::pickCombFeedback);
  addKnob(
    leftExcitation + knobX, topExcitation, knobWidth, margin, uiTextSize, "Time",
    ID::pickCombTime);

  // Objects.
  const auto leftObjects = leftExcitation + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftObjects, top0, 2.0f * knobX, labelHeight, midTextSize, "Objects");

  const auto topObjects = top0 + labelHeight + margin;
  addNumberKnob(
    leftObjects, topObjects, knobWidth, margin, uiTextSize, "nCymbal", ID::nCymbal,
    Scales::nCymbal, 1);
  addNumberKnob(
    leftObjects + knobX, topObjects, knobWidth, margin, uiTextSize, "nString", ID::stack,
    Scales::stack, 1);

  // Wave.
  const auto leftWave = leftObjects + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftWave, top0, 3.0f * knobX, labelHeight, midTextSize, "Wave");

  const auto topWave = top0 + labelHeight + margin;
  addKnob<Style::warning>(
    leftWave, topWave, knobWidth, margin, uiTextSize, "Damping", ID::damping);
  addKnob(
    leftWave + knobX, topWave, knobWidth, margin, uiTextSize, "PulsePosition",
    ID::pulsePosition);
  addKnob(
    leftWave + 2.0f * knobX, topWave, knobWidth, margin, uiTextSize, "PulseWidth",
    ID::pulseWidth);

  // Collision.
  const auto top1 = top0 + knobY + 3.0f * margin;

  const auto leftCollision = leftExcitation;
  addToggleButton(
    leftCollision, top1, checkboxWidth, labelHeight, midTextSize, "Collision",
    ID::collision);

  const auto topCollision = top1 + labelHeight + margin;
  addKnob<Style::warning>(
    leftCollision, topCollision, knobWidth, margin, uiTextSize, "Distance", ID::distance);

  // Random.
  const auto leftRandom = leftCollision + knobX + 4.0f * margin;
  addGroupLabel(leftRandom, top1, 2.0f * knobX, labelHeight, midTextSize, "Random");

  const auto topRandom = top1 + labelHeight + margin;
  addNumberKnob(
    leftRandom, topRandom, knobWidth, margin, uiTextSize, "Seed", ID::seed, Scales::seed,
    0);
  addKnob<Style::warning>(
    leftRandom + knobX, topRandom, knobWidth, margin, uiTextSize, "Amount",
    ID::randomAmount);

  // String.
  const auto leftString = leftRandom + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftString, top1, 4.0f * knobX, labelHeight, midTextSize, "String");

  const auto topString = top1 + labelHeight + margin;
  addKnob(
    leftString, topString, knobWidth, margin, uiTextSize, "MinHz", ID::minFrequency);
  addKnob(
    leftString + knobX, topString, knobWidth, margin, uiTextSize, "MaxHz",
    ID::maxFrequency);
  addKnob(
    leftString + 2.0f * knobX, topString, knobWidth, margin, uiTextSize, "Decay",
    ID::decay);
  addKnob(
    leftString + 3.0f * knobX, topString, knobWidth, margin, uiTextSize, "Q",
    ID::bandpassQ);

  // Oscillator.
  const auto top2 = top1 + labelHeight + knobY;

  addGroupLabel(
    leftExcitation, top2, 3.0f * knobX + 2.0f * margin, labelHeight, midTextSize,
    "Oscillator");

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
    leftExcitation + 2.0f * knobX + 2.0f * margin, topOscillator, checkboxWidth,
    labelHeight, uiTextSize, ID::cutoffMap, itemCutoffMap);

  // Smoothness.
  const auto leftSmoothness = leftExcitation + 3.0f * knobX + 4.0f * margin;
  addKnob(
    leftSmoothness + 22.5f, top2 - margin, 50.0f, margin, uiTextSize, "Smoothness",
    ID::smoothness);

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  addSplashScreen(
    defaultWidth - 20.0f - splashWidth, top2 + 2.0 * margin, splashWidth, 40.0f, 100.0f,
    20.0f, defaultWidth - 200.0f, defaultHeight - 40.0f, pluginNameTextSize,
    "WaveCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
