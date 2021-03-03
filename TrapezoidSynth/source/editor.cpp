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

#include "gui/grouplabeltpz.hpp"
#include "gui/splash.hpp"

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 14.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float margin = 5.0f;
constexpr float groupLabelMargin = 10.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = knobWidth - 2.0f * margin;
constexpr float knobX = knobWidth; // With margin.
constexpr float knobY = knobHeight + labelY + 2.0f * margin;
constexpr float sliderWidth = 70.0f;
constexpr float sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
constexpr float sliderX = 80.0f;
constexpr float sliderY = sliderHeight + labelY;
constexpr float checkboxWidth = 80.0f;
constexpr uint32_t defaultWidth = uint32_t(40 + 13 * knobX);
constexpr uint32_t defaultHeight = uint32_t(20 + 5 * (knobY + labelHeight));

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<>
void Editor<Synth::PlugParameter>::addTpzLabel(
  CCoord left, CCoord top, CCoord width, CCoord height, CCoord textSize, std::string name)
{
  auto bottom = top + labelHeight;

  auto label = new GroupLabelTpz(
    CRect(left, top, left + width, bottom), this, name,
    new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette);
  label->setMargin(groupLabelMargin);
  frame->addView(label);
}

template<>
void Editor<Synth::PlugParameter>::addSplashScreenTpz(
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
    this, palette);
  auto splash = new SplashLabelTpz(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette, 0,
    credit, pluginName);
  frame->addView(splash);
  frame->addView(credit);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  const auto &scale = param.scale;
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto left0 = 20.0f;

  const auto top0 = 20.0f;
  addTpzLabel(left0, top0, 6.0f * knobWidth, labelHeight, midTextSize, "Oscillator 1");
  const auto top0knob = top0 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Semi", ID::osc1Semi,
    scale.semi);
  addNumberKnob(
    left0 + 1.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Cent", ID::osc1Cent,
    scale.cent);
  addKnob(
    left0 + 2.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Drift",
    ID::osc1PitchDrift);
  addKnob(
    left0 + 3.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Slope",
    ID::osc1Slope);
  addKnob(
    left0 + 4.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "PW",
    ID::osc1PulseWidth);
  addKnob(
    left0 + 5.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Feedback",
    ID::osc1Feedback);

  const auto top1 = top0knob + knobY;
  addTpzLabel(left0, top1, 6.0f * knobWidth, labelHeight, midTextSize, "Oscillator 2");
  const auto top1knob = top1 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Semi", ID::osc2Semi,
    scale.semi);
  addNumberKnob(
    left0 + 1.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Cent", ID::osc2Cent,
    scale.cent);
  addNumberKnob(
    left0 + 2.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Overtone",
    ID::osc2Overtone, scale.overtone);
  addKnob(
    left0 + 3.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Slope",
    ID::osc2Slope);
  addKnob(
    left0 + 4.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "PW",
    ID::osc2PulseWidth);
  addKnob(
    left0 + 5.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "PM",
    ID::pmOsc2ToOsc1);

  const auto top2 = top1knob + knobY;
  addTpzLabel(left0, top2, 6.0f * knobX, labelHeight, midTextSize, "Gain Envelope");
  auto checkBoxGainEnvRetrigger = addCheckbox(
    left0 + ceil(2.15f * knobX), top2, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::gainEnvRetrigger);
  checkBoxGainEnvRetrigger->drawBackground = true;
  const auto top2knob = top2 + labelHeight;
  addKnob(left0 + 0.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "A", ID::gainA);
  addKnob(left0 + 1.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "D", ID::gainD);
  addKnob(left0 + 2.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "S", ID::gainS);
  addKnob(left0 + 3.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "R", ID::gainR);
  addKnob(
    left0 + 4.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Curve",
    ID::gainCurve);
  addKnob(
    left0 + 5.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain", ID::gain);

  const auto top3 = top2knob + knobY;
  addTpzLabel(left0, top3, 6.0f * knobWidth, labelHeight, midTextSize, "Filter");
  const auto top3knob = top3 + labelHeight;
  std::vector<std::string> filterOrderItems{
    "Order 1", "Order 2", "Order 3", "Order 4",
    "Order 5", "Order 6", "Order 7", "Order 8",
  };
  addOptionMenu(
    left0 + 0.9f * knobX, top3, knobWidth, labelHeight, uiTextSize, ID::filterOrder,
    filterOrderItems);
  addKnob(
    left0 + 0.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Cut",
    ID::filterCutoff);
  addKnob(
    left0 + 1.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Res.",
    ID::filterFeedback);
  addKnob(
    left0 + 2.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Sat.",
    ID::filterSaturation);
  addKnob(
    left0 + 3.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Env>Cut",
    ID::filterEnvToCutoff);
  addKnob(
    left0 + 4.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Key>Cut",
    ID::filterKeyToCutoff);
  addKnob(
    left0 + 5.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "+OscMix",
    ID::oscMixToFilterCutoff);

  const auto top4 = top3knob + knobY;
  addTpzLabel(left0, top4, 6.0f * knobWidth, labelHeight, midTextSize, "Filter Envelope");
  auto checkBoxFiltEnvRetrigger = addCheckbox(
    left0 + ceil(2.15f * knobX), top4, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::filterEnvRetrigger);
  checkBoxFiltEnvRetrigger->drawBackground = true;
  const auto top4knob = top4 + labelHeight;
  addKnob(
    left0 + 0.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "A", ID::filterA);
  addKnob(
    left0 + 1.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "D", ID::filterD);
  addKnob(
    left0 + 2.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "S", ID::filterS);
  addKnob(
    left0 + 3.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "R", ID::filterR);
  addKnob(
    left0 + 4.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "Curve",
    ID::filterCurve);
  addNumberKnob(
    left0 + 5.0f * knobX, top4knob, knobWidth, margin, uiTextSize, ">Octave",
    ID::filterEnvToOctave, scale.filterEnvToOctave);

  const auto left1 = left0 + 7.0f * knobX;

  addTpzLabel(left1, top0, 3.0f * knobWidth, labelHeight, midTextSize, "Misc");
  addKnob(
    left1 + 0.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "OscMix", ID::oscMix);
  addNumberKnob(
    left1 + 1.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Octave", ID::octave,
    scale.octave);
  addKnob(
    left1 + 2.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Smooth",
    ID::smoothness);

  addTpzLabel(
    left1 + 3.0f * knobX, top0, 3.0f * knobWidth, labelHeight, midTextSize, "Mod 1");
  auto checkBoxMod1Retrigger = addCheckbox(
    left1 + ceil(3.95f * knobX), top0, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::modEnv1Retrigger);
  checkBoxMod1Retrigger->drawBackground = true;
  addKnob(
    left1 + 3.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Attack",
    ID::modEnv1Attack);
  addKnob(
    left1 + 4.0f * knobX, top0knob, knobWidth, margin, uiTextSize, "Curve",
    ID::modEnv1Curve);
  addKnob(
    left1 + 5.0f * knobX, top0knob, knobWidth, margin, uiTextSize, ">PM",
    ID::modEnv1ToPhaseMod);

  addTpzLabel(left1, top1, 6.0f * knobWidth, labelHeight, midTextSize, "Mod 2");
  auto checkBoxMod2Retrigger = addCheckbox(
    left1 + ceil(0.95f * knobX), top1, checkboxWidth, labelHeight, uiTextSize,
    "Retrigger", ID::modEnv2Retrigger);
  checkBoxMod2Retrigger->drawBackground = true;
  addKnob(
    left1 + 0.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Attack",
    ID::modEnv2Attack);
  addKnob(
    left1 + 1.0f * knobX, top1knob, knobWidth, margin, uiTextSize, "Curve",
    ID::modEnv2Curve);
  addKnob(
    left1 + 2.0f * knobX, top1knob, knobWidth, margin, uiTextSize, ">Feedback",
    ID::modEnv2ToFeedback);
  addKnob(
    left1 + 3.0f * knobX, top1knob, knobWidth, margin, uiTextSize, ">LFO",
    ID::modEnv2ToLFOFrequency);
  addKnob(
    left1 + 4.0f * knobX, top1knob, knobWidth, margin, uiTextSize, ">Slope2",
    ID::modEnv2ToOsc2Slope);
  addKnob(
    left1 + 5.0f * knobX, top1knob, knobWidth, margin, uiTextSize, ">Shifter1",
    ID::modEnv2ToShifter1);

  addTpzLabel(left1, top2, 3.0f * knobWidth, labelHeight, midTextSize, "Shifter 1");
  addNumberKnob(
    left1 + 0.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Semi",
    ID::shifter1Semi, scale.shifterSemi);
  addNumberKnob(
    left1 + 1.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Cent",
    ID::shifter1Cent, scale.shifterCent);
  addKnob(
    left1 + 2.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain",
    ID::shifter1Gain);

  addTpzLabel(
    left1 + 3.0f * knobX, top2, 3.0f * knobWidth, labelHeight, midTextSize, "Shifter 2");
  addNumberKnob(
    left1 + 3.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Semi",
    ID::shifter2Semi, scale.shifterSemi);
  addNumberKnob(
    left1 + 4.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Cent",
    ID::shifter2Cent, scale.shifterCent);
  addKnob(
    left1 + 5.0f * knobX, top2knob, knobWidth, margin, uiTextSize, "Gain",
    ID::shifter2Gain);

  addTpzLabel(left1, top3, 6.0f * knobWidth, labelHeight, midTextSize, "LFO");
  std::vector<std::string> lfoTypeItems{"Sin", "Saw", "Pulse", "Noise"};
  addOptionMenu(
    left1 + 0.8f * knobX, top3, knobWidth, labelHeight, uiTextSize, ID::lfoType,
    lfoTypeItems);
  auto checkBoxTempo = addCheckbox(
    left1 + 2.2f * knobX, top3, 65.0f, labelHeight, uiTextSize, "Tempo",
    ID::lfoTempoSync);
  checkBoxTempo->drawBackground = true;
  addKnob(
    left1 + 0.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Freq",
    ID::lfoFrequency);
  addKnob(
    left1 + 1.0f * knobX, top3knob, knobWidth, margin, uiTextSize, "Shape", ID::lfoShape);
  addKnob(
    left1 + 2.0f * knobX, top3knob, knobWidth, margin, uiTextSize, ">Pitch1",
    ID::lfoToPitch);
  addKnob(
    left1 + 3.0f * knobX, top3knob, knobWidth, margin, uiTextSize, ">Slope1",
    ID::lfoToSlope);
  addKnob(
    left1 + 4.0f * knobX, top3knob, knobWidth, margin, uiTextSize, ">PW1",
    ID::lfoToPulseWidth);
  addKnob(
    left1 + 5.0f * knobX, top3knob, knobWidth, margin, uiTextSize, ">Cut",
    ID::lfoToCutoff);

  addTpzLabel(left1, top4, 6.0f * knobWidth, labelHeight, midTextSize, "Slide");
  std::vector<std::string> pitchSlideType{"Always", "Sustain", "Reset to 0"};
  addOptionMenu(
    left1 + 0.75f * knobX, top4, 70.0f, labelHeight, uiTextSize, ID::pitchSlideType,
    pitchSlideType);
  addKnob(
    left1 + 0.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "Time",
    ID::pitchSlide);
  addKnob(
    left1 + 1.0f * knobX, top4knob, knobWidth, margin, uiTextSize, "Offset",
    ID::pitchSlideOffset);

  // Plugin name.,
  const auto splashWidth = 3.75f * knobX;
  const auto splashHeight = 40.0f;
  addSplashScreenTpz(
    left1 + 2.25f * knobX, top4 + 1.5f * labelHeight, splashWidth, splashHeight, 20.0f,
    20.0f, defaultWidth - 40.0f, defaultHeight - 40.0f, pluginNameTextSize,
    "TrapezoidSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
