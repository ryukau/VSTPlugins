// (c) 2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "gui/grouplabel.hpp"
#include "gui/splash.hpp"

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

  uiTextSize = 12.0f;
  midTextSize = 14.0f;
  pluginNameTextSize = 24.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = knobWidth - 2.0f * margin;
  knobX = knobWidth; // With margin.
  knobY = knobHeight + labelY + 2.0f * margin;
  sliderWidth = 70.0f;
  sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
  sliderX = 80.0f;
  sliderY = sliderHeight + labelY;
  checkboxWidth = 80.0f;

  viewRect
    = ViewRect{0, 0, int32(40 + 13 * knobX), int32(20 + 5 * (knobY + labelHeight))};
  setRect(viewRect);
}

void Editor::addGroupLabelTpz(
  CCoord left, CCoord top, CCoord width, CCoord lineMargin, std::string name)
{
  top -= margin;
  auto bottom = top + labelHeight;

  auto label = new GroupLabelTpz(CRect(left, top, left + width, bottom), this, name);
  label->setFont(new CFontDesc(PlugEditorStyle::fontName(), 14.0, CTxtFace::kBoldFace));
  label->setMargin(lineMargin);
  frame->addView(label);
}

void Editor::addSplashScreenTpz(
  float buttonLeft,
  float buttonTop,
  float buttonWidth,
  float buttonHeight,
  float splashLeft,
  float splashTop,
  float splashWidth,
  float splashHeight,
  const char *pluginName)
{
  auto credit = new CreditView(
    CRect(splashLeft, splashTop, splashLeft + splashWidth, splashTop + splashHeight),
    this);
  auto splash = new SplashLabelTpz(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, 0, credit, pluginName);
  splash->setHighlightColor(colorOrange);
  frame->addView(splash);
  frame->addView(credit);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto left0 = 20.0f;

  const auto top0 = 20.0f;
  addGroupLabelTpz(left0, top0, 6.0f * knobWidth, groupLabelMargin, "Oscillator 1");
  const auto top0knob = top0 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top0knob, knobWidth, colorBlue, "Semi", ID::osc1Semi,
    Scales::semi, 0);
  addNumberKnob(
    left0 + 1.0f * knobX, top0knob, knobWidth, colorBlue, "Cent", ID::osc1Cent,
    Scales::cent, 0);
  addKnob(
    left0 + 2.0f * knobX, top0knob, knobWidth, colorBlue, "Drift", ID::osc1PitchDrift);
  addKnob(left0 + 3.0f * knobX, top0knob, knobWidth, colorBlue, "Slope", ID::osc1Slope);
  addKnob(left0 + 4.0f * knobX, top0knob, knobWidth, colorBlue, "PW", ID::osc1PulseWidth);
  addKnob(
    left0 + 5.0f * knobX, top0knob, knobWidth, colorBlue, "Feedback", ID::osc1Feedback);

  const auto top1 = top0knob + knobY;
  addGroupLabelTpz(left0, top1, 6.0f * knobWidth, groupLabelMargin, "Oscillator 2");
  const auto top1knob = top1 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top1knob, knobWidth, colorBlue, "Semi", ID::osc2Semi,
    Scales::semi, 0);
  addNumberKnob(
    left0 + 1.0f * knobX, top1knob, knobWidth, colorBlue, "Cent", ID::osc2Cent,
    Scales::cent, 0);
  addNumberKnob(
    left0 + 2.0f * knobX, top1knob, knobWidth, colorBlue, "Overtone", ID::osc2Overtone,
    Scales::overtone, 0);
  addKnob(left0 + 3.0f * knobX, top1knob, knobWidth, colorBlue, "Slope", ID::osc2Slope);
  addKnob(left0 + 4.0f * knobX, top1knob, knobWidth, colorBlue, "PW", ID::osc2PulseWidth);
  addKnob(left0 + 5.0f * knobX, top1knob, knobWidth, colorBlue, "PM", ID::pmOsc2ToOsc1);

  const auto top2 = top1knob + knobY;
  addGroupLabelTpz(
    left0, top2, 6.0f * knobX, checkboxWidth + groupLabelMargin, "Gain Envelope");
  addCheckbox(
    left0 + 2.1f * knobX, top2 - margin, checkboxWidth, "Retrigger",
    ID::gainEnvRetrigger);
  const auto top2knob = top2 + labelHeight;
  addKnob(left0 + 0.0f * knobX, top2knob, knobWidth, colorBlue, "A", ID::gainA);
  addKnob(left0 + 1.0f * knobX, top2knob, knobWidth, colorBlue, "D", ID::gainD);
  addKnob(left0 + 2.0f * knobX, top2knob, knobWidth, colorBlue, "S", ID::gainS);
  addKnob(left0 + 3.0f * knobX, top2knob, knobWidth, colorBlue, "R", ID::gainR);
  addKnob(left0 + 4.0f * knobX, top2knob, knobWidth, colorBlue, "Curve", ID::gainCurve);
  addKnob(left0 + 5.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::gain);

  const auto top3 = top2knob + knobY;
  addGroupLabelTpz(left0, top3, 6.0f * knobWidth, knobWidth + groupLabelMargin, "Filter");
  const auto top3knob = top3 + labelHeight;
  std::vector<UTF8String> filterOrderItems{
    "Order 1", "Order 2", "Order 3", "Order 4",
    "Order 5", "Order 6", "Order 7", "Order 8",
  };
  addOptionMenu(
    left0 + 0.9f * knobX, top3 - margin, knobWidth, ID::filterOrder, filterOrderItems);
  addKnob(left0 + 0.0f * knobX, top3knob, knobWidth, colorBlue, "Cut", ID::filterCutoff);
  addKnob(
    left0 + 1.0f * knobX, top3knob, knobWidth, colorBlue, "Res.", ID::filterFeedback);
  addKnob(
    left0 + 2.0f * knobX, top3knob, knobWidth, colorBlue, "Sat.", ID::filterSaturation);
  addKnob(
    left0 + 3.0f * knobX, top3knob, knobWidth, colorBlue, "Env>Cut",
    ID::filterEnvToCutoff);
  addKnob(
    left0 + 4.0f * knobX, top3knob, knobWidth, colorBlue, "Key>Cut",
    ID::filterKeyToCutoff);
  addKnob(
    left0 + 5.0f * knobX, top3knob, knobWidth, colorBlue, "+OscMix",
    ID::oscMixToFilterCutoff);

  const auto top4 = top3knob + knobY;
  addGroupLabelTpz(
    left0, top4, 6.0f * knobWidth, checkboxWidth + groupLabelMargin, "Filter Envelope");
  addCheckbox(
    left0 + 2.15f * knobX, top4 - margin, checkboxWidth, "Retrigger",
    ID::filterEnvRetrigger);
  const auto top4knob = top4 + labelHeight;
  addKnob(left0 + 0.0f * knobX, top4knob, knobWidth, colorBlue, "A", ID::filterA);
  addKnob(left0 + 1.0f * knobX, top4knob, knobWidth, colorBlue, "D", ID::filterD);
  addKnob(left0 + 2.0f * knobX, top4knob, knobWidth, colorBlue, "S", ID::filterS);
  addKnob(left0 + 3.0f * knobX, top4knob, knobWidth, colorBlue, "R", ID::filterR);
  addKnob(left0 + 4.0f * knobX, top4knob, knobWidth, colorBlue, "Curve", ID::filterCurve);
  addNumberKnob(
    left0 + 5.0f * knobX, top4knob, knobWidth, colorBlue, ">Octave",
    ID::filterEnvToOctave, Scales::filterEnvToOctave, 0);

  const auto left1 = left0 + 7.0f * knobX;

  addGroupLabelTpz(left1, top0, 3.0f * knobWidth, groupLabelMargin, "Misc");
  addKnob(left1 + 0.0f * knobX, top0knob, knobWidth, colorBlue, "OscMix", ID::oscMix);
  addNumberKnob(
    left1 + 1.0f * knobX, top0knob, knobWidth, colorBlue, "Octave", ID::octave,
    Scales::octave, 0);
  addKnob(left1 + 2.0f * knobX, top0knob, knobWidth, colorBlue, "Smooth", ID::smoothness);

  addGroupLabelTpz(
    left1 + 3.0f * knobX, top0, 3.0f * knobWidth, checkboxWidth + groupLabelMargin,
    "Mod 1");
  addCheckbox(
    left1 + 4.0f * knobX, top0 - margin, checkboxWidth, "Retrigger",
    ID::modEnv1Retrigger);
  addKnob(
    left1 + 3.0f * knobX, top0knob, knobWidth, colorBlue, "Attack", ID::modEnv1Attack);
  addKnob(
    left1 + 4.0f * knobX, top0knob, knobWidth, colorBlue, "Curve", ID::modEnv1Curve);
  addKnob(
    left1 + 5.0f * knobX, top0knob, knobWidth, colorBlue, ">PM", ID::modEnv1ToPhaseMod);

  addGroupLabelTpz(
    left1, top1, 6.0f * knobWidth, checkboxWidth + groupLabelMargin, "Mod 2");
  addCheckbox(
    left1 + 1.0f * knobX, top1 - margin, checkboxWidth, "Retrigger",
    ID::modEnv2Retrigger);
  addKnob(
    left1 + 0.0f * knobX, top1knob, knobWidth, colorBlue, "Attack", ID::modEnv2Attack);
  addKnob(
    left1 + 1.0f * knobX, top1knob, knobWidth, colorBlue, "Curve", ID::modEnv2Curve);
  addKnob(
    left1 + 2.0f * knobX, top1knob, knobWidth, colorBlue, ">Feedback",
    ID::modEnv2ToFeedback);
  addKnob(
    left1 + 3.0f * knobX, top1knob, knobWidth, colorBlue, ">LFO",
    ID::modEnv2ToLFOFrequency);
  addKnob(
    left1 + 4.0f * knobX, top1knob, knobWidth, colorBlue, ">Slope2",
    ID::modEnv2ToOsc2Slope);
  addKnob(
    left1 + 5.0f * knobX, top1knob, knobWidth, colorBlue, ">Shifter1",
    ID::modEnv2ToShifter1);

  addGroupLabelTpz(left1, top2, 3.0f * knobWidth, groupLabelMargin, "Shifter 1");
  addNumberKnob(
    left1 + 0.0f * knobX, top2knob, knobWidth, colorBlue, "Semi", ID::shifter1Semi,
    Scales::shifterSemi, 0);
  addNumberKnob(
    left1 + 1.0f * knobX, top2knob, knobWidth, colorBlue, "Cent", ID::shifter1Cent,
    Scales::shifterCent, 0);
  addKnob(left1 + 2.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::shifter1Gain);

  addGroupLabelTpz(
    left1 + 3.0f * knobX, top2, 3.0f * knobWidth, groupLabelMargin, "Shifter 2");
  addNumberKnob(
    left1 + 3.0f * knobX, top2knob, knobWidth, colorBlue, "Semi", ID::shifter2Semi,
    Scales::shifterSemi, 0);
  addNumberKnob(
    left1 + 4.0f * knobX, top2knob, knobWidth, colorBlue, "Cent", ID::shifter2Cent,
    Scales::shifterCent, 0);
  addKnob(left1 + 5.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::shifter2Gain);

  addGroupLabelTpz(
    left1, top3, 6.0f * knobWidth, 2.4f * knobX + 2.0f * groupLabelMargin, "LFO");
  std::vector<UTF8String> lfoTypeItems{"Sin", "Saw", "Pulse", "Noise"};
  addOptionMenu(
    left1 + 0.8f * knobX, top3 - margin, knobWidth, ID::lfoType, lfoTypeItems);
  addCheckbox(left1 + 2.0f * knobX, top3 - margin, 55.0f, "Tempo", ID::lfoTempoSync);
  addKnob(left1 + 0.0f * knobX, top3knob, knobWidth, colorBlue, "Freq", ID::lfoFrequency);
  addKnob(left1 + 1.0f * knobX, top3knob, knobWidth, colorBlue, "Shape", ID::lfoShape);
  addKnob(
    left1 + 2.0f * knobX, top3knob, knobWidth, colorBlue, ">Pitch1", ID::lfoToPitch);
  addKnob(
    left1 + 3.0f * knobX, top3knob, knobWidth, colorBlue, ">Slope1", ID::lfoToSlope);
  addKnob(
    left1 + 4.0f * knobX, top3knob, knobWidth, colorBlue, ">PW1", ID::lfoToPulseWidth);
  addKnob(left1 + 5.0f * knobX, top3knob, knobWidth, colorBlue, ">Cut", ID::lfoToCutoff);

  addGroupLabelTpz(left1, top4, 6.0f * knobWidth, groupLabelMargin, "Slide");
  std::vector<UTF8String> pitchSlideType{"Always", "Sustain", "Reset to 0"};
  addOptionMenu(
    left1 + 0.8f * knobX, top4 - margin, 70.0f, ID::pitchSlideType, pitchSlideType);
  addKnob(left1 + 0.0f * knobX, top4knob, knobWidth, colorBlue, "Time", ID::pitchSlide);
  addKnob(
    left1 + 1.0f * knobX, top4knob, knobWidth, colorBlue, "Offset", ID::pitchSlideOffset);

  // Plugin name.
  const auto splashWidth = 3.75f * knobX;
  const auto splashHeight = knobY - 20.0;
  addSplashScreenTpz(
    left1 + 2.25f * knobX, top4 + labelHeight, splashWidth, splashHeight, 20.0f, 20.0f,
    viewRect.right - 40.0f, viewRect.bottom - 40.0f, "TrapezoidSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
