// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "gui/grouplabeltpz.hpp"
#include "gui/splash.hpp"

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

void Editor::addTpzLabel(
  CCoord left, CCoord top, CCoord width, CCoord height, CCoord textSize, std::string name)
{
  auto bottom = top + labelHeight;

  auto label = new GroupLabelTpz(
    CRect(left, top, left + width, bottom), this, name, getFont(textSize), palette);
  label->setMargin(groupLabelMargin);
  frame->addView(label);
}

void Editor::addSplashScreenTpz(
  CCoord buttonLeft,
  CCoord buttonTop,
  CCoord buttonWidth,
  CCoord buttonHeight,
  CCoord splashLeft,
  CCoord splashTop,
  CCoord splashWidth,
  CCoord splashHeight,
  CCoord textSize,
  const char *pluginName)
{
  auto credit = new CreditView(
    CRect(splashLeft, splashTop, splashLeft + splashWidth, splashTop + splashHeight),
    this, getFont(18.0), getFont(12.0), palette);
  auto splash = new SplashLabelTpz(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, getFont(textSize), palette, 0, credit, pluginName);
  frame->addView(splash);
  frame->addView(credit);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto sc = palette.guiScale();
  const auto left0 = uiMargin;
  const auto top0 = uiMargin;

  addTpzLabel(left0, top0, 6 * knobWidth, labelHeight, midTextSize, "Oscillator 1");
  const auto top0knob = top0 + labelHeight;
  addNumberKnob(
    left0 + 0 * knobX, top0knob, knobWidth, margin, uiTextSize, "Semi", ID::osc1Semi,
    Scales::semi);
  addNumberKnob(
    left0 + 1 * knobX, top0knob, knobWidth, margin, uiTextSize, "Cent", ID::osc1Cent,
    Scales::cent);
  addKnob(
    left0 + 2 * knobX, top0knob, knobWidth, margin, uiTextSize, "Drift",
    ID::osc1PitchDrift);
  addKnob(
    left0 + 3 * knobX, top0knob, knobWidth, margin, uiTextSize, "Slope", ID::osc1Slope);
  addKnob(
    left0 + 4 * knobX, top0knob, knobWidth, margin, uiTextSize, "PW", ID::osc1PulseWidth);
  addKnob(
    left0 + 5 * knobX, top0knob, knobWidth, margin, uiTextSize, "Feedback",
    ID::osc1Feedback);

  const auto top1 = top0knob + knobY;
  addTpzLabel(left0, top1, 6 * knobWidth, labelHeight, midTextSize, "Oscillator 2");
  const auto top1knob = top1 + labelHeight;
  addNumberKnob(
    left0 + 0 * knobX, top1knob, knobWidth, margin, uiTextSize, "Semi", ID::osc2Semi,
    Scales::semi);
  addNumberKnob(
    left0 + 1 * knobX, top1knob, knobWidth, margin, uiTextSize, "Cent", ID::osc2Cent,
    Scales::cent);
  addNumberKnob(
    left0 + 2 * knobX, top1knob, knobWidth, margin, uiTextSize, "Overtone",
    ID::osc2Overtone, Scales::overtone);
  addKnob(
    left0 + 3 * knobX, top1knob, knobWidth, margin, uiTextSize, "Slope", ID::osc2Slope);
  addKnob(
    left0 + 4 * knobX, top1knob, knobWidth, margin, uiTextSize, "PW", ID::osc2PulseWidth);
  addKnob(
    left0 + 5 * knobX, top1knob, knobWidth, margin, uiTextSize, "PM", ID::pmOsc2ToOsc1);

  const auto top2 = top1knob + knobY;
  addTpzLabel(left0, top2, 6 * knobX, labelHeight, midTextSize, "Gain Envelope");
  auto checkBoxGainEnvRetrigger = addCheckbox(
    left0 + std::ceil(2.15 * knobX), top2, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::gainEnvRetrigger);
  checkBoxGainEnvRetrigger->drawBackground = true;
  const auto top2knob = top2 + labelHeight;
  addKnob(left0 + 0 * knobX, top2knob, knobWidth, margin, uiTextSize, "A", ID::gainA);
  addKnob(left0 + 1 * knobX, top2knob, knobWidth, margin, uiTextSize, "D", ID::gainD);
  addKnob(left0 + 2 * knobX, top2knob, knobWidth, margin, uiTextSize, "S", ID::gainS);
  addKnob(left0 + 3 * knobX, top2knob, knobWidth, margin, uiTextSize, "R", ID::gainR);
  addKnob(
    left0 + 4 * knobX, top2knob, knobWidth, margin, uiTextSize, "Curve", ID::gainCurve);
  addKnob(left0 + 5 * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain", ID::gain);

  const auto top3 = top2knob + knobY;
  addTpzLabel(left0, top3, 6 * knobWidth, labelHeight, midTextSize, "Filter");
  const auto top3knob = top3 + labelHeight;
  std::vector<std::string> filterOrderItems{
    "Order 1", "Order 2", "Order 3", "Order 4",
    "Order 5", "Order 6", "Order 7", "Order 8",
  };
  addOptionMenu(
    left0 + int(0.9 * knobX), top3, knobWidth, labelHeight, uiTextSize, ID::filterOrder,
    filterOrderItems);
  addKnob(
    left0 + 0 * knobX, top3knob, knobWidth, margin, uiTextSize, "Cut", ID::filterCutoff);
  addKnob(
    left0 + 1 * knobX, top3knob, knobWidth, margin, uiTextSize, "Res.",
    ID::filterFeedback);
  addKnob(
    left0 + 2 * knobX, top3knob, knobWidth, margin, uiTextSize, "Sat.",
    ID::filterSaturation);
  addKnob(
    left0 + 3 * knobX, top3knob, knobWidth, margin, uiTextSize, "Env>Cut",
    ID::filterEnvToCutoff);
  addKnob(
    left0 + 4 * knobX, top3knob, knobWidth, margin, uiTextSize, "Key>Cut",
    ID::filterKeyToCutoff);
  addKnob(
    left0 + 5 * knobX, top3knob, knobWidth, margin, uiTextSize, "+OscMix",
    ID::oscMixToFilterCutoff);

  const auto top4 = top3knob + knobY;
  addTpzLabel(left0, top4, 6 * knobWidth, labelHeight, midTextSize, "Filter Envelope");
  auto checkBoxFiltEnvRetrigger = addCheckbox(
    left0 + std::ceil(2.15 * knobX), top4, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::filterEnvRetrigger);
  checkBoxFiltEnvRetrigger->drawBackground = true;
  const auto top4knob = top4 + labelHeight;
  addKnob(left0 + 0 * knobX, top4knob, knobWidth, margin, uiTextSize, "A", ID::filterA);
  addKnob(left0 + 1 * knobX, top4knob, knobWidth, margin, uiTextSize, "D", ID::filterD);
  addKnob(left0 + 2 * knobX, top4knob, knobWidth, margin, uiTextSize, "S", ID::filterS);
  addKnob(left0 + 3 * knobX, top4knob, knobWidth, margin, uiTextSize, "R", ID::filterR);
  addKnob(
    left0 + 4 * knobX, top4knob, knobWidth, margin, uiTextSize, "Curve", ID::filterCurve);
  addNumberKnob(
    left0 + 5 * knobX, top4knob, knobWidth, margin, uiTextSize, ">Octave",
    ID::filterEnvToOctave, Scales::filterEnvToOctave);

  const auto left1 = left0 + 7 * knobX;

  addTpzLabel(left1, top0, 3 * knobWidth, labelHeight, midTextSize, "Misc");
  addKnob(
    left1 + 0 * knobX, top0knob, knobWidth, margin, uiTextSize, "OscMix", ID::oscMix);
  addNumberKnob(
    left1 + 1 * knobX, top0knob, knobWidth, margin, uiTextSize, "Octave", ID::octave,
    Scales::octave);
  addKnob(
    left1 + 2 * knobX, top0knob, knobWidth, margin, uiTextSize, "Smooth", ID::smoothness);

  addTpzLabel(left1 + 3 * knobX, top0, 3 * knobWidth, labelHeight, midTextSize, "Mod 1");
  auto checkBoxMod1Retrigger = addCheckbox(
    left1 + std::ceil(3.95 * knobX), top0, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::modEnv1Retrigger);
  checkBoxMod1Retrigger->drawBackground = true;
  addKnob(
    left1 + 3 * knobX, top0knob, knobWidth, margin, uiTextSize, "Attack",
    ID::modEnv1Attack);
  addKnob(
    left1 + 4 * knobX, top0knob, knobWidth, margin, uiTextSize, "Curve",
    ID::modEnv1Curve);
  addKnob(
    left1 + 5 * knobX, top0knob, knobWidth, margin, uiTextSize, ">PM",
    ID::modEnv1ToPhaseMod);

  addTpzLabel(left1, top1, 6 * knobWidth, labelHeight, midTextSize, "Mod 2");
  auto checkBoxMod2Retrigger = addCheckbox(
    left1 + std::ceil(0.95 * knobX), top1, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::modEnv2Retrigger);
  checkBoxMod2Retrigger->drawBackground = true;
  addKnob(
    left1 + 0 * knobX, top1knob, knobWidth, margin, uiTextSize, "Attack",
    ID::modEnv2Attack);
  addKnob(
    left1 + 1 * knobX, top1knob, knobWidth, margin, uiTextSize, "Curve",
    ID::modEnv2Curve);
  addKnob(
    left1 + 2 * knobX, top1knob, knobWidth, margin, uiTextSize, ">Feedback",
    ID::modEnv2ToFeedback);
  addKnob(
    left1 + 3 * knobX, top1knob, knobWidth, margin, uiTextSize, ">LFO",
    ID::modEnv2ToLFOFrequency);
  addKnob(
    left1 + 4 * knobX, top1knob, knobWidth, margin, uiTextSize, ">Slope2",
    ID::modEnv2ToOsc2Slope);
  addKnob(
    left1 + 5 * knobX, top1knob, knobWidth, margin, uiTextSize, ">Shifter1",
    ID::modEnv2ToShifter1);

  addTpzLabel(left1, top2, 3 * knobWidth, labelHeight, midTextSize, "Shifter 1");
  addNumberKnob(
    left1 + 0 * knobX, top2knob, knobWidth, margin, uiTextSize, "Semi", ID::shifter1Semi,
    Scales::shifterSemi);
  addNumberKnob(
    left1 + 1 * knobX, top2knob, knobWidth, margin, uiTextSize, "Cent", ID::shifter1Cent,
    Scales::shifterCent);
  addKnob(
    left1 + 2 * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain", ID::shifter1Gain);

  addTpzLabel(
    left1 + 3 * knobX, top2, 3 * knobWidth, labelHeight, midTextSize, "Shifter 2");
  addNumberKnob(
    left1 + 3 * knobX, top2knob, knobWidth, margin, uiTextSize, "Semi", ID::shifter2Semi,
    Scales::shifterSemi);
  addNumberKnob(
    left1 + 4 * knobX, top2knob, knobWidth, margin, uiTextSize, "Cent", ID::shifter2Cent,
    Scales::shifterCent);
  addKnob(
    left1 + 5 * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain", ID::shifter2Gain);

  addTpzLabel(left1, top3, 6 * knobWidth, labelHeight, midTextSize, "LFO");
  std::vector<std::string> lfoTypeItems{"Sin", "Saw", "Pulse", "Noise"};
  addOptionMenu(
    left1 + int(0.8 * knobX), top3, knobWidth, labelHeight, uiTextSize, ID::lfoType,
    lfoTypeItems);
  auto checkBoxTempo = addCheckbox(
    left1 + int(2.2 * knobX), top3, int(sc * 65), labelHeight, uiTextSize, "Tempo",
    ID::lfoTempoSync);
  checkBoxTempo->drawBackground = true;
  addKnob(
    left1 + 0 * knobX, top3knob, knobWidth, margin, uiTextSize, "Freq", ID::lfoFrequency);
  addKnob(
    left1 + 1 * knobX, top3knob, knobWidth, margin, uiTextSize, "Shape", ID::lfoShape);
  addKnob(
    left1 + 2 * knobX, top3knob, knobWidth, margin, uiTextSize, ">Pitch1",
    ID::lfoToPitch);
  addKnob(
    left1 + 3 * knobX, top3knob, knobWidth, margin, uiTextSize, ">Slope1",
    ID::lfoToSlope);
  addKnob(
    left1 + 4 * knobX, top3knob, knobWidth, margin, uiTextSize, ">PW1",
    ID::lfoToPulseWidth);
  addKnob(
    left1 + 5 * knobX, top3knob, knobWidth, margin, uiTextSize, ">Cut", ID::lfoToCutoff);

  addTpzLabel(left1, top4, 6 * knobWidth, labelHeight, midTextSize, "Slide");
  std::vector<std::string> pitchSlideType{"Always", "Sustain", "Reset to 0"};
  addOptionMenu(
    left1 + int(0.75 * knobX), top4, int(sc * 70), labelHeight, uiTextSize,
    ID::pitchSlideType, pitchSlideType);
  addKnob(
    left1 + 0 * knobX, top4knob, knobWidth, margin, uiTextSize, "Time", ID::pitchSlide);
  addKnob(
    left1 + 1 * knobX, top4knob, knobWidth, margin, uiTextSize, "Offset",
    ID::pitchSlideOffset);

  // Plugin name.,
  const auto splashWidth = int(3.75 * knobX);
  const auto splashHeight = 2 * uiMargin;
  addSplashScreenTpz(
    left1 + int(2.25 * knobX), top4 + int(1.5 * labelHeight), splashWidth, splashHeight,
    uiMargin, uiMargin, defaultWidth - 2 * uiMargin, defaultHeight - 2 * uiMargin,
    pluginNameTextSize, "TrapezoidSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
