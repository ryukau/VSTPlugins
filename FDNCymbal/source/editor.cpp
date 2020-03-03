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
  sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
  sliderX = 80.0f;
  sliderY = sliderHeight + labelY;
  checkboxWidth = 80.0f;

  viewRect = ViewRect{0, 0, int32(40 + sliderX + 7 * knobX + 6 * margin),
                      int32(20 + 3 * knobY + 3 * labelHeight + 3 * margin)};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  // Gain.
  const auto left0 = 20.0f;
  const auto top0 = 20.0f;
  const auto smallKnobWidth = 50.0f;

  addVSlider(left0, top0, colorBlue, "Gain", ID::gain);

  // Stick.
  const auto leftStick = left0 + sliderX + 2.0f * margin;
  addButton(leftStick, top0, 2.0f * knobX, "Stick", ID::stick, CTextButton::kOnOffStyle);

  const auto topStick = top0 + labelHeight + margin;
  addKnob(
    leftStick, topStick, smallKnobWidth, colorBlue, " Pulse", ID::stickPulseMix,
    LabelPosition::right);
  addKnob(
    leftStick, topStick + smallKnobWidth, smallKnobWidth, colorBlue, " Velvet",
    ID::stickVelvetMix, LabelPosition::right);
  addKnob(
    leftStick + knobX + margin, topStick, smallKnobWidth, colorBlue, " Tone",
    ID::stickToneMix, LabelPosition::right);
  addKnob(
    leftStick + knobX + margin, topStick + smallKnobWidth, smallKnobWidth, colorBlue,
    " Decay", ID::stickDecay, LabelPosition::right);

  // Random.
  const auto leftRandom = leftStick + 2.0f * knobX + 2.0f * margin;
  addGroupLabel(leftRandom, top0, 2.0f * knobX, "Random");

  const auto topRandom = top0 + labelHeight;
  addNumberKnob(
    leftRandom, topRandom + margin, knobWidth, colorBlue, "Seed", ID::seed, Scales::seed,
    0);
  addLabel(leftRandom + knobX, topRandom, knobWidth, "Retrigger");
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + labelHeight, checkboxWidth, "Time",
    ID::retriggerTime);
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + 2.0f * labelHeight, checkboxWidth,
    "Stick", ID::retriggerStick);
  addCheckbox(
    leftRandom + knobX + 2.0f * margin, topRandom + 3.0f * labelHeight, checkboxWidth,
    "Tremolo", ID::retriggerTremolo);

  // FDN.
  const auto leftFDN = leftRandom + 2.0f * knobX + 2.0f * margin;
  addButton(leftFDN, top0, 3.0f * knobX, "FDN", ID::fdn, CTextButton::kOnOffStyle);

  const auto topFDN = top0 + labelHeight + margin;
  addKnob(leftFDN, topFDN, knobWidth, colorBlue, "Time", ID::fdnTime);
  addKnob(leftFDN + knobX, topFDN, knobWidth, colorRed, "Feedback", ID::fdnFeedback);
  addKnob(
    leftFDN + 2.0f * knobX, topFDN, knobWidth, colorBlue, "CascadeMix",
    ID::fdnCascadeMix);

  // Allpass.
  const auto top1 = top0 + knobY + labelHeight + margin;
  const auto leftAP = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftAP, top1, knobX, "Allpass");

  const auto topAP = top1 + labelHeight + margin;
  addKnob(leftAP, topAP, knobWidth, colorBlue, "Mix", ID::allpassMix);

  const auto leftAP1 = leftAP + knobX + 2.0f * margin;
  addGroupLabel(leftAP1, top1, 3.0f * knobX, "Stage 1");
  addCheckbox(
    leftAP1 + knobX + 3.5 * margin, topAP + knobHeight + labelHeight + 0.5f * margin,
    checkboxWidth, "Tanh", ID::allpass1Saturation);

  addKnob(leftAP1, topAP, knobWidth, colorBlue, "Time", ID::allpass1Time);
  addKnob(leftAP1 + knobX, topAP, knobWidth, colorBlue, "Feedback", ID::allpass1Feedback);
  addKnob(
    leftAP1 + 2.0f * knobX, topAP, knobWidth, colorRed, "HP Cutoff",
    ID::allpass1HighpassCutoff);

  const auto leftAP2 = leftAP1 + 3.0f * knobX + 2.0f * margin;
  addGroupLabel(leftAP2, top1, 3.0f * knobX, "Stage 2");

  addKnob(leftAP2, topAP, knobWidth, colorBlue, "Time", ID::allpass2Time);
  addKnob(leftAP2 + knobX, topAP, knobWidth, colorBlue, "Feedback", ID::allpass2Feedback);
  addKnob(
    leftAP2 + 2.0f * knobX, topAP, knobWidth, colorRed, "HP Cutoff",
    ID::allpass2HighpassCutoff);

  // Smooth.
  const auto top2 = top1 + knobY + labelHeight + margin;
  addKnob(
    left0 - margin, top2 + labelHeight + margin, sliderX, colorBlue, "Smooth",
    ID::smoothness);

  // Tremolo.
  const auto leftTremolo = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftTremolo, top2, 4.0f * knobX, "Tremolo");

  const auto topTremolo = top2 + labelHeight + margin;
  addKnob(leftTremolo, topTremolo, knobWidth, colorBlue, "Mix", ID::tremoloMix);
  addKnob(
    leftTremolo + knobX, topTremolo, knobWidth, colorBlue, "Depth", ID::tremoloDepth);
  addKnob(
    leftTremolo + 2.0f * knobX, topTremolo, knobWidth, colorBlue, "Frequency",
    ID::tremoloFrequency);
  addKnob(
    leftTremolo + 3.0f * knobX, topTremolo, knobWidth, colorBlue, "DelayTime",
    ID::tremoloDelayTime);

  const auto leftTremoloRandom = leftTremolo + 4.0f * knobX + 2.0f * margin;
  addGroupLabel(leftTremoloRandom, top2, 3.0f * knobX + 2.0f * margin, "Random");
  addKnob(
    leftTremoloRandom, topTremolo - 1.5f * margin, 50.0f, colorBlue, "Depth",
    ID::randomTremoloDepth, LabelPosition::right);
  addKnob(
    leftTremoloRandom + 1.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    colorBlue, "Freq", ID::randomTremoloFrequency, LabelPosition::right);
  addKnob(
    leftTremoloRandom + 2.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    colorBlue, "Time", ID::randomTremoloDelayTime, LabelPosition::right);

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  const auto splashHeight = 40.0f;
  addSplashScreen(
    viewRect.right - 20.0f - splashWidth, viewRect.bottom - 20.0f - splashHeight,
    splashWidth, splashHeight, 20.0f, 20.0f, viewRect.right - 40.0f,
    viewRect.bottom - 40.0f, "FDNCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
