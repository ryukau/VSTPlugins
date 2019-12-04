// (c) 2019 Takamitsu Endo
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

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "plugeditor.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "textbutton.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller) { setRect(viewRect); }

bool PlugEditor::open(void *parent, const PlatformType &platformType)
{
  if (frame) return false;

  setIdleRate(1000 / 60);

  frame = new CFrame(
    CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
  if (frame == nullptr) return false;
  frame->setBackgroundColor(colorWhite);
  frame->registerMouseObserver(this);
  frame->open(parent);

  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  Synth::GlobalParameter param;

  const auto left0 = 20.0f;

  const auto top0 = 20.0f;
  addGroupLabel(left0, top0, 6.0f * knobWidth, groupLabelMargin, "Oscillator 1");
  const auto top0knob = top0 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top0knob, knobWidth, colorBlue, "Semi", ID::osc1Semi,
    Scales::semi, 0, param);
  addNumberKnob(
    left0 + 1.0f * knobX, top0knob, knobWidth, colorBlue, "Cent", ID::osc1Cent,
    Scales::cent, 0, param);
  addKnob(
    left0 + 2.0f * knobX, top0knob, knobWidth, colorBlue, "Drift", ID::osc1PitchDrift,
    param);
  addKnob(
    left0 + 3.0f * knobX, top0knob, knobWidth, colorBlue, "Slope", ID::osc1Slope, param);
  addKnob(
    left0 + 4.0f * knobX, top0knob, knobWidth, colorBlue, "PW", ID::osc1PulseWidth,
    param);
  addKnob(
    left0 + 5.0f * knobX, top0knob, knobWidth, colorBlue, "Feedback", ID::osc1Feedback,
    param);

  const auto top1 = top0knob + knobY;
  addGroupLabel(left0, top1, 6.0f * knobWidth, groupLabelMargin, "Oscillator 2");
  const auto top1knob = top1 + labelHeight;
  addNumberKnob(
    left0 + 0.0f * knobX, top1knob, knobWidth, colorBlue, "Semi", ID::osc2Semi,
    Scales::semi, 0, param);
  addNumberKnob(
    left0 + 1.0f * knobX, top1knob, knobWidth, colorBlue, "Cent", ID::osc2Cent,
    Scales::cent, 0, param);
  addNumberKnob(
    left0 + 2.0f * knobX, top1knob, knobWidth, colorBlue, "Overtone", ID::osc2Overtone,
    Scales::overtone, 0, param);
  addKnob(
    left0 + 3.0f * knobX, top1knob, knobWidth, colorBlue, "Slope", ID::osc2Slope, param);
  addKnob(
    left0 + 4.0f * knobX, top1knob, knobWidth, colorBlue, "PW", ID::osc2PulseWidth,
    param);
  addKnob(
    left0 + 5.0f * knobX, top1knob, knobWidth, colorBlue, "PM", ID::pmOsc2ToOsc1, param);

  const auto top2 = top1knob + knobY;
  addGroupLabel(
    left0, top2, 6.0f * knobX, checkboxWidth + groupLabelMargin, "Gain Envelope");
  addCheckbox(
    left0 + 2.1f * knobX, top2, checkboxWidth, "Retrigger", ID::gainEnvRetrigger);
  const auto top2knob = top2 + labelHeight;
  addKnob(left0 + 0.0f * knobX, top2knob, knobWidth, colorBlue, "A", ID::gainA, param);
  addKnob(left0 + 1.0f * knobX, top2knob, knobWidth, colorBlue, "D", ID::gainD, param);
  addKnob(left0 + 2.0f * knobX, top2knob, knobWidth, colorBlue, "S", ID::gainS, param);
  addKnob(left0 + 3.0f * knobX, top2knob, knobWidth, colorBlue, "R", ID::gainR, param);
  addKnob(
    left0 + 4.0f * knobX, top2knob, knobWidth, colorBlue, "Curve", ID::gainCurve, param);
  addKnob(left0 + 5.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::gain, param);

  const auto top3 = top2knob + knobY;
  addGroupLabel(left0, top3, 6.0f * knobWidth, knobWidth + groupLabelMargin, "Filter");
  const auto top3knob = top3 + labelHeight;
  std::vector<UTF8String> filterOrderItems{
    "Order 1", "Order 2", "Order 3", "Order 4",
    "Order 5", "Order 6", "Order 7", "Order 8",
  };
  addOptionMenu(left0 + 0.9f * knobX, top3, knobWidth, ID::filterOrder, filterOrderItems);
  addKnob(
    left0 + 0.0f * knobX, top3knob, knobWidth, colorBlue, "Cut", ID::filterCutoff, param);
  addKnob(
    left0 + 1.0f * knobX, top3knob, knobWidth, colorBlue, "Res.", ID::filterFeedback,
    param);
  addKnob(
    left0 + 2.0f * knobX, top3knob, knobWidth, colorBlue, "Sat.", ID::filterSaturation,
    param);
  addKnob(
    left0 + 3.0f * knobX, top3knob, knobWidth, colorBlue, "Env>Cut",
    ID::filterEnvToCutoff, param);
  addKnob(
    left0 + 4.0f * knobX, top3knob, knobWidth, colorBlue, "Key>Cut",
    ID::filterKeyToCutoff, param);
  addKnob(
    left0 + 5.0f * knobX, top3knob, knobWidth, colorBlue, "+OscMix",
    ID::oscMixToFilterCutoff, param);

  const auto top4 = top3knob + knobY;
  addGroupLabel(
    left0, top4, 6.0f * knobWidth, checkboxWidth + groupLabelMargin, "Filter Envelope");
  addCheckbox(
    left0 + 2.15f * knobX, top4, checkboxWidth, "Retrigger", ID::filterEnvRetrigger);
  const auto top4knob = top4 + labelHeight;
  addKnob(left0 + 0.0f * knobX, top4knob, knobWidth, colorBlue, "A", ID::filterA, param);
  addKnob(left0 + 1.0f * knobX, top4knob, knobWidth, colorBlue, "D", ID::filterD, param);
  addKnob(left0 + 2.0f * knobX, top4knob, knobWidth, colorBlue, "S", ID::filterS, param);
  addKnob(left0 + 3.0f * knobX, top4knob, knobWidth, colorBlue, "R", ID::filterR, param);
  addKnob(
    left0 + 4.0f * knobX, top4knob, knobWidth, colorBlue, "Curve", ID::filterCurve,
    param);
  addNumberKnob(
    left0 + 5.0f * knobX, top4knob, knobWidth, colorBlue, ">Octave",
    ID::filterEnvToOctave, Scales::filterEnvToOctave, 0, param);

  const auto left1 = left0 + 7.0f * knobX;

  addGroupLabel(left1, top0, 3.0f * knobWidth, groupLabelMargin, "Misc");
  addKnob(
    left1 + 0.0f * knobX, top0knob, knobWidth, colorBlue, "OscMix", ID::oscMix, param);
  addNumberKnob(
    left1 + 1.0f * knobX, top0knob, knobWidth, colorBlue, "Octave", ID::octave,
    Scales::octave, 0, param);
  addKnob(
    left1 + 2.0f * knobX, top0knob, knobWidth, colorBlue, "Smooth", ID::smoothness,
    param);

  addGroupLabel(
    left1 + 3.0f * knobX, top0, 3.0f * knobWidth, checkboxWidth + groupLabelMargin,
    "Mod 1");
  addCheckbox(
    left1 + 4.0f * knobX, top0, checkboxWidth, "Retrigger", ID::modEnv1Retrigger);
  addKnob(
    left1 + 3.0f * knobX, top0knob, knobWidth, colorBlue, "Attack", ID::modEnv1Attack,
    param);
  addKnob(
    left1 + 4.0f * knobX, top0knob, knobWidth, colorBlue, "Curve", ID::modEnv1Curve,
    param);
  addKnob(
    left1 + 5.0f * knobX, top0knob, knobWidth, colorBlue, ">PM", ID::modEnv1ToPhaseMod,
    param);

  addGroupLabel(left1, top1, 6.0f * knobWidth, checkboxWidth + groupLabelMargin, "Mod 2");
  addCheckbox(
    left1 + 1.0f * knobX, top1, checkboxWidth, "Retrigger", ID::modEnv2Retrigger);
  addKnob(
    left1 + 0.0f * knobX, top1knob, knobWidth, colorBlue, "Attack", ID::modEnv2Attack,
    param);
  addKnob(
    left1 + 1.0f * knobX, top1knob, knobWidth, colorBlue, "Curve", ID::modEnv2Curve,
    param);
  addKnob(
    left1 + 2.0f * knobX, top1knob, knobWidth, colorBlue, ">Feedback",
    ID::modEnv2ToFeedback, param);
  addKnob(
    left1 + 3.0f * knobX, top1knob, knobWidth, colorBlue, ">LFO",
    ID::modEnv2ToLFOFrequency, param);
  addKnob(
    left1 + 4.0f * knobX, top1knob, knobWidth, colorBlue, ">Slope2",
    ID::modEnv2ToOsc2Slope, param);
  addKnob(
    left1 + 5.0f * knobX, top1knob, knobWidth, colorBlue, ">Shifter1",
    ID::modEnv2ToShifter1, param);

  addGroupLabel(left1, top2, 3.0f * knobWidth, groupLabelMargin, "Shifter 1");
  addNumberKnob(
    left1 + 0.0f * knobX, top2knob, knobWidth, colorBlue, "Semi", ID::shifter1Semi,
    Scales::shifterSemi, 0, param);
  addNumberKnob(
    left1 + 1.0f * knobX, top2knob, knobWidth, colorBlue, "Cent", ID::shifter1Cent,
    Scales::shifterCent, 0, param);
  addKnob(
    left1 + 2.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::shifter1Gain,
    param);

  addGroupLabel(
    left1 + 3.0f * knobX, top2, 3.0f * knobWidth, groupLabelMargin, "Shifter 2");
  addNumberKnob(
    left1 + 3.0f * knobX, top2knob, knobWidth, colorBlue, "Semi", ID::shifter2Semi,
    Scales::shifterSemi, 0, param);
  addNumberKnob(
    left1 + 4.0f * knobX, top2knob, knobWidth, colorBlue, "Cent", ID::shifter2Cent,
    Scales::shifterCent, 0, param);
  addKnob(
    left1 + 5.0f * knobX, top2knob, knobWidth, colorBlue, "Gain", ID::shifter2Gain,
    param);

  addGroupLabel(
    left1, top3, 6.0f * knobWidth, 2.4f * knobX + 2.0f * groupLabelMargin, "LFO");
  std::vector<UTF8String> lfoTypeItems{"Sin", "Saw", "Pulse", "Noise"};
  addOptionMenu(left1 + 0.8f * knobX, top3, knobWidth, ID::lfoType, lfoTypeItems);
  addCheckbox(left1 + 2.0f * knobX, top3, 55.0f, "Tempo", ID::lfoTempoSync);
  addKnob(
    left1 + 0.0f * knobX, top3knob, knobWidth, colorBlue, "Freq", ID::lfoFrequency,
    param);
  addKnob(
    left1 + 1.0f * knobX, top3knob, knobWidth, colorBlue, "Shape", ID::lfoShape, param);
  addKnob(
    left1 + 2.0f * knobX, top3knob, knobWidth, colorBlue, ">Pitch1", ID::lfoToPitch,
    param);
  addKnob(
    left1 + 3.0f * knobX, top3knob, knobWidth, colorBlue, ">Slope1", ID::lfoToSlope,
    param);
  addKnob(
    left1 + 4.0f * knobX, top3knob, knobWidth, colorBlue, ">PW1", ID::lfoToPulseWidth,
    param);
  addKnob(
    left1 + 5.0f * knobX, top3knob, knobWidth, colorBlue, ">Cut", ID::lfoToCutoff, param);

  addGroupLabel(left1, top4, 6.0f * knobWidth, groupLabelMargin, "Slide");
  std::vector<UTF8String> pitchSlideType{"Always", "Sustain", "Reset to 0"};
  addOptionMenu(left1 + 0.8f * knobX, top4, 70.0f, ID::pitchSlideType, pitchSlideType);
  addKnob(
    left1 + 0.0f * knobX, top4knob, knobWidth, colorBlue, "Time", ID::pitchSlide, param);
  addKnob(
    left1 + 1.0f * knobX, top4knob, knobWidth, colorBlue, "Offset", ID::pitchSlideOffset,
    param);

  // Plugin name.
  const auto splashWidth = 3.75f * knobX;
  const auto splashHeight = knobY - 20.0;
  addSplashScreen(
    left1 + 2.25f * knobX, top4 + labelHeight, splashWidth, splashHeight, 20.0f, 20.0f,
    viewRect.right - 40.0f, viewRect.bottom - 40.0f, "TrapezoidSynth");

  return true;
}

void PlugEditor::close()
{
  if (!frame) return;
  frame->forget();
  frame = nullptr;
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

void PlugEditor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter == controlMap.end()) return;
  iter->second->setValueNormalized(normalized);
  iter->second->invalid();
}

CMouseEventResult
PlugEditor::onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isRightButton()) return kMouseEventNotHandled;

  auto componentHandler = controller->getComponentHandler();
  if (componentHandler == nullptr) return kMouseEventNotHandled;

  FUnknownPtr<IComponentHandler3> handler(componentHandler);
  if (handler == nullptr) return kMouseEventNotHandled;

  auto control = dynamic_cast<CControl *>(frame->getViewAt(where));
  if (control == nullptr) return kMouseEventNotHandled;

  // Context menu will not popup when the control has negative tag.
  ParamID id = control->getTag();
  if (id < 1 || id >= LONG_MAX) return kMouseEventNotHandled;

  IContextMenu *menu = handler->createContextMenu(this, &id);
  if (menu == nullptr) return kMouseEventNotHandled;

  menu->popup(where.x, where.y);
  menu->release();
  return kMouseEventHandled;
}
void PlugEditor::addLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font)
{
  auto bottom = top + 20.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  if (font == nullptr)
    font = new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace);
  label->setFont(font);
  label->setHoriAlign(CHoriTxtAlign::kCenterText);
  label->setStyle(CTextLabel::kNoFrame);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
}

void PlugEditor::addGroupLabel(
  CCoord left, CCoord top, CCoord width, CCoord lineMargin, UTF8String name)
{
  top -= margin;
  auto bottom = top + labelHeight;

  auto label
    = new GroupLabel(CRect(left, top, left + width, bottom), this, UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), 14.0, CTxtFace::kBoldFace));
  label->setMargin(lineMargin);
  frame->addView(label);
}

void PlugEditor::addSplashScreen(
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
    nullptr);
  auto splash = new SplashLabel(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, -666, credit, pluginName);
  splash->setHighlightColor(colorOrange);
  credit->setListener(splash);
  frame->addView(splash);
}

void PlugEditor::addVSlider(
  CCoord left,
  CCoord top,
  CColor valueColor,
  UTF8String name,
  ParamID tag,
  float defaultValue,
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  auto right = left + sliderWidth;
  auto bottom = top + sliderHeight;

  auto slider = new Slider(
    CRect(left, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);
  slider->setSliderMode(CSliderMode::FreeClick);
  slider->setStyle(CSlider::kBottom | CSlider::kVertical);
  slider->setDrawStyle(
    CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
    | (drawFromCenter ? CSlider::kDrawValueFromCenter | CSlider::kDrawInverted : 0));
  slider->setBackColor(colorWhite);
  slider->setValueColor(valueColor);
  slider->setHighlightColor(valueColor);
  slider->setDefaultFrameColor(colorBlack);
  slider->setHighlightWidth(3.0);
  slider->setValueNormalized(controller->getParamNormalized(tag));
  slider->setDefaultValue(defaultValue);
  slider->setTooltipText(tooltip);
  frame->addView(slider);
  addToControlMap(tag, slider);

  top = bottom + margin;
  bottom = top + labelHeight;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  frame->addView(label);
}

void PlugEditor::addButton(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + labelHeight;

  auto button = new TextButton(
    CRect(left, top, right, bottom), this, tag, title, (CTextButton::Style)style);
  button->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setTextColor(colorBlack);
  button->setTextColorHighlighted(colorBlack);
  button->setGradient(CGradient::create(0.0, 1.0, colorWhite, colorWhite));
  button->setGradientHighlighted(CGradient::create(0.0, 1.0, colorOrange, colorOrange));
  button->setHighlightColor(colorOrange);
  button->setFrameColorHighlighted(colorBlack);
  button->setFrameWidth(1.0);
  button->setRoundRadius(0.0);
  button->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(button);
  addToControlMap(tag, button);
}

void PlugEditor::addCheckbox(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  top -= margin;
  auto right = left + width;
  auto bottom = top + 20.0;

  auto checkbox
    = new CheckBox(CRect(left, top, right, bottom), this, tag, title, nullptr, style);
  checkbox->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  checkbox->setFontColor(colorBlack);
  checkbox->setBoxFrameColor(colorBlack);
  checkbox->setBoxFillColor(colorWhite);
  checkbox->setCheckMarkColor(colorBlue);
  checkbox->sizeToFit();
  checkbox->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(checkbox);
  addToControlMap(tag, checkbox);
}

void PlugEditor::addOptionMenu(
  CCoord left,
  CCoord top,
  CCoord width,
  ParamID tag,
  const std::vector<UTF8String> &items)
{
  top -= margin;
  auto right = left + width;
  auto bottom = top + 20.0;

  auto menu = new OptionMenu(
    CRect(left, top, right, bottom), this, tag, nullptr, nullptr,
    COptionMenu::kCheckStyle);
  for (const auto &item : items) menu->addEntry(item);
  menu->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  menu->setFontColor(colorBlack);
  menu->setBackColor(colorWhite);
  menu->setDefaultFrameColor(colorBlack);
  menu->setHighlightColor(colorBlue);
  menu->sizeToFit();
  menu->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(menu);
  addToControlMap(tag, menu);
}

void PlugEditor::addKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  Synth::GlobalParameter &param,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

template<typename Scale>
void PlugEditor::addNumberKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  Scale &scale,
  int32_t offset,
  Synth::GlobalParameter &param,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new NumberKnob<Scale>(
    CRect(left + 5.0, top, right - 5.0, bottom), this, tag, scale, offset);
  knob->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

void PlugEditor::addKnobLabel(
  float left,
  float top,
  float right,
  float bottom,
  UTF8String name,
  LabelPosition labelPosition)
{
  switch (labelPosition) {
    default:
    case LabelPosition::bottom:
      top = bottom;
      bottom = top + 30.0;
      left -= 10.0;
      right += 10.0;
      break;

    case LabelPosition::right:
      left = right + margin;
      right = left + 100.0;
      break;
  }

  auto label
    = new CTextLabel(CRect(left - 10.0, top, right + 10.0, bottom), UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(CColor{0, 0, 0, 0});
  if (labelPosition == LabelPosition::right) label->sizeToFit();
  frame->addView(label);
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
