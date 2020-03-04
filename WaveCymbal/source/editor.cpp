// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 28.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 80.0f;
  knobHeight = knobWidth - 2.0f * margin;
  knobX = knobWidth; // With margin.
  knobY = knobHeight + labelY + 2.0f * margin;
  sliderWidth = 70.0f;
  sliderHeight = 2.0f * (knobHeight + labelY) + 90.0f;
  sliderX = 80.0f;
  sliderY = sliderHeight + labelY;
  checkboxWidth = 80.0f;

  viewRect = ViewRect{0, 0, int32(2 * 20 + sliderX + 7 * knobX + 10 * margin),
                      int32(2 * 15 + 2 * knobY + 2 * labelHeight + 55)};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  // Gain.
  const auto left0 = 10.0f;
  const auto top0 = 20.0f;

  const auto leftGain = left0 + 2.0f * margin;
  addVSlider(leftGain, top0, colorBlue, "Gain", ID::gain);

  // Excitation.
  const auto leftExcitation = leftGain + sliderX + 2.0f * margin;
  addButton(
    leftExcitation, top0, 2.0f * knobX, "Excitation", ID::excitation,
    CTextButton::kOnOffStyle);

  const auto topExcitation = top0 + labelHeight + margin;
  addKnob(
    leftExcitation, topExcitation, knobWidth, colorRed, "Feedback", ID::pickCombFeedback);
  addKnob(
    leftExcitation + knobX, topExcitation, knobWidth, colorBlue, "Time",
    ID::pickCombTime);

  // Objects.
  const auto leftObjects = leftExcitation + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftObjects, top0, 2.0f * knobX, "Objects");

  const auto topObjects = top0 + labelHeight + margin;
  addNumberKnob(
    leftObjects, topObjects, knobWidth, colorBlue, "nCymbal", ID::nCymbal,
    Scales::nCymbal, 1);
  addNumberKnob(
    leftObjects + knobX, topObjects, knobWidth, colorBlue, "nString", ID::stack,
    Scales::stack, 1);

  // Wave.
  const auto leftWave = leftObjects + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftWave, top0, 3.0f * knobX, "Wave");

  const auto topWave = top0 + labelHeight + margin;
  addKnob(leftWave, topWave, knobWidth, colorRed, "Damping", ID::damping);
  addKnob(
    leftWave + knobX, topWave, knobWidth, colorBlue, "PulsePosition", ID::pulsePosition);
  addKnob(
    leftWave + 2.0f * knobX, topWave, knobWidth, colorBlue, "PulseWidth", ID::pulseWidth);

  // Bandpass.
  const auto top1 = top0 + knobY + 3.0f * margin;

  // Collision.
  const auto leftCollision = leftExcitation;
  addButton(
    leftCollision, top1, checkboxWidth, "Collision", ID::collision,
    CTextButton::kOnOffStyle);

  const auto topCollision = top1 + labelHeight + margin;
  addKnob(leftCollision, topCollision, knobWidth, colorRed, "Distance", ID::distance);

  // Random.
  const auto leftRandom = leftCollision + knobX + 4.0f * margin;
  addGroupLabel(leftRandom, top1, 2.0f * knobX, "Random");

  const auto topRandom = top1 + labelHeight + margin;
  addNumberKnob(
    leftRandom, topRandom, knobWidth, colorBlue, "Seed", ID::seed, Scales::seed, 0);
  addKnob(leftRandom + knobX, topRandom, knobWidth, colorRed, "Amount", ID::randomAmount);

  // String.
  const auto leftString = leftRandom + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftString, top1, 4.0f * knobX, "String");

  const auto topString = top1 + labelHeight + margin;
  addKnob(leftString, topString, knobWidth, colorBlue, "MinHz", ID::minFrequency);
  addKnob(leftString + knobX, topString, knobWidth, colorBlue, "MaxHz", ID::maxFrequency);
  addKnob(leftString + 2.0f * knobX, topString, knobWidth, colorBlue, "Decay", ID::decay);
  addKnob(leftString + 3.0f * knobX, topString, knobWidth, colorBlue, "Q", ID::bandpassQ);

  // Oscillator.
  const auto top2 = top1 + labelHeight + knobY;

  addGroupLabel(leftExcitation, top2, 3.0f * knobX + 2.0f * margin, "Oscillator");

  const auto topOscillator = top2 + labelHeight + margin;
  addCheckbox(leftExcitation, topOscillator, checkboxWidth, "Retrigger", ID::retrigger);

  std::vector<UTF8String> itemOscType
    = {"Off", "Impulse", "Sustain", "Velvet Noise", "Brown Noise"};
  addOptionMenu(
    leftExcitation + knobX, topOscillator, checkboxWidth, ID::oscType, itemOscType);

  std::vector<UTF8String> itemCutoffMap = {"Log", "Linear"};
  addOptionMenu(
    leftExcitation + 2.0f * knobX + 2.0f * margin, topOscillator, checkboxWidth,
    ID::cutoffMap, itemCutoffMap);

  // Smoothness.
  const auto leftSmoothness = leftExcitation + 3.0f * knobX + 4.0f * margin;
  addKnob(
    leftSmoothness + 22.5f, top2 - margin, 50.0f, colorBlue, "Smoothness",
    ID::smoothness);

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  addSplashScreen(
    viewRect.right - 20.0f - splashWidth, top2 + 2.0 * margin, splashWidth, 40.0f, 100.0f,
    20.0f, viewRect.right - 200.0f, viewRect.bottom - 40.0f, "WaveCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
