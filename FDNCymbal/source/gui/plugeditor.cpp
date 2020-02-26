// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "../parameter.hpp"
#include "plugeditor.hpp"
#include "x11runloop.hpp"

#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
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

  IPlatformFrameConfig *config = nullptr;
#if LINUX
  X11::FrameConfig x11config;
  x11config.runLoop = VSTGUI::owned(new RunLoop(plugFrame));
  config = &x11config;
#endif
  frame->open(parent, platformType, config);

  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  Synth::GlobalParameter param;

  // Gain.
  const auto left0 = 20.0f;
  const auto top0 = 20.0f;

  addVSlider(
    left0, top0, colorBlue, "Gain", ID::gain,
    param.value[ID::gain]->getDefaultNormalized());

  // Stick.
  const auto leftStick = left0 + sliderX + 2.0f * margin;
  addButton(leftStick, top0, 2.0f * knobX, "Stick", ID::stick, CTextButton::kOnOffStyle);

  const auto topStick = top0 + labelHeight + margin;
  addKnob(
    leftStick, topStick, knobWidth, colorBlue, "Decay", ID::stickDecay,
    param.value[ID::stickDecay]->getDefaultNormalized());
  addKnob(
    leftStick + knobX, topStick, knobWidth, colorBlue, "ToneMix", ID::stickToneMix,
    param.value[ID::stickToneMix]->getDefaultNormalized());

  // Random.
  const auto leftRandom = leftStick + 2.0f * knobX + 2.0f * margin;
  addGroupLabel(leftRandom, top0, 2.0f * knobX, "Random");

  const auto topRandom = top0 + labelHeight;
  addNumberKnob(
    leftRandom, topRandom + margin, knobWidth, colorBlue, "Seed", ID::seed, Scales::seed,
    0, param.value[ID::seed]->getDefaultNormalized());
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
  addKnob(
    leftFDN, topFDN, knobWidth, colorBlue, "Time", ID::fdnTime,
    param.value[ID::fdnTime]->getDefaultNormalized());
  addKnob(
    leftFDN + knobX, topFDN, knobWidth, colorRed, "Feedback", ID::fdnFeedback,
    param.value[ID::fdnFeedback]->getDefaultNormalized());
  addKnob(
    leftFDN + 2.0f * knobX, topFDN, knobWidth, colorBlue, "CascadeMix", ID::fdnCascadeMix,
    param.value[ID::fdnCascadeMix]->getDefaultNormalized());

  // Allpass.
  const auto top1 = top0 + knobY + labelHeight + margin;
  const auto leftAP = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftAP, top1, knobX, "Allpass");

  const auto topAP = top1 + labelHeight + margin;
  addKnob(
    leftAP, topAP, knobWidth, colorBlue, "Mix", ID::allpassMix,
    param.value[ID::allpassMix]->getDefaultNormalized());

  const auto leftAP1 = leftAP + knobX + 2.0f * margin;
  addGroupLabel(leftAP1, top1, 3.0f * knobX, "Stage 1");
  addCheckbox(
    leftAP1 + knobX + 3.5 * margin, topAP + knobHeight + labelHeight + 0.5f * margin,
    checkboxWidth, "Tanh", ID::allpass1Saturation);

  addKnob(
    leftAP1, topAP, knobWidth, colorBlue, "Time", ID::allpass1Time,
    param.value[ID::allpass1Time]->getDefaultNormalized());
  addKnob(
    leftAP1 + knobX, topAP, knobWidth, colorBlue, "Feedback", ID::allpass1Feedback,
    param.value[ID::allpass1Feedback]->getDefaultNormalized());
  addKnob(
    leftAP1 + 2.0f * knobX, topAP, knobWidth, colorRed, "HP Cutoff",
    ID::allpass1HighpassCutoff,
    param.value[ID::allpass1HighpassCutoff]->getDefaultNormalized());

  const auto leftAP2 = leftAP1 + 3.0f * knobX + 2.0f * margin;
  addGroupLabel(leftAP2, top1, 3.0f * knobX, "Stage 2");

  addKnob(
    leftAP2, topAP, knobWidth, colorBlue, "Time", ID::allpass2Time,
    param.value[ID::allpass2Time]->getDefaultNormalized());
  addKnob(
    leftAP2 + knobX, topAP, knobWidth, colorBlue, "Feedback", ID::allpass2Feedback,
    param.value[ID::allpass2Feedback]->getDefaultNormalized());
  addKnob(
    leftAP2 + 2.0f * knobX, topAP, knobWidth, colorRed, "HP Cutoff",
    ID::allpass2HighpassCutoff,
    param.value[ID::allpass2HighpassCutoff]->getDefaultNormalized());

  // Smooth.
  const auto top2 = top1 + knobY + labelHeight + margin;
  addKnob(
    left0 - margin, top2 + labelHeight + margin, sliderX, colorBlue, "Smooth",
    ID::smoothness, param.value[ID::smoothness]->getDefaultNormalized());

  // Tremolo.
  const auto leftTremolo = left0 + sliderX + 2.0f * margin;
  addGroupLabel(leftTremolo, top2, 4.0f * knobX, "Tremolo");

  const auto topTremolo = top2 + labelHeight + margin;
  addKnob(
    leftTremolo, topTremolo, knobWidth, colorBlue, "Mix", ID::tremoloMix,
    param.value[ID::tremoloMix]->getDefaultNormalized());
  addKnob(
    leftTremolo + knobX, topTremolo, knobWidth, colorBlue, "Depth", ID::tremoloDepth,
    param.value[ID::tremoloDepth]->getDefaultNormalized());
  addKnob(
    leftTremolo + 2.0f * knobX, topTremolo, knobWidth, colorBlue, "Frequency",
    ID::tremoloFrequency, param.value[ID::tremoloFrequency]->getDefaultNormalized());
  addKnob(
    leftTremolo + 3.0f * knobX, topTremolo, knobWidth, colorBlue, "DelayTime",
    ID::tremoloDelayTime, param.value[ID::tremoloDelayTime]->getDefaultNormalized());

  const auto leftTremoloRandom = leftTremolo + 4.0f * knobX + 2.0f * margin;
  addGroupLabel(leftTremoloRandom, top2, 3.0f * knobX + 2.0f * margin, "Random");
  addKnob(
    leftTremoloRandom, topTremolo - 1.5f * margin, 50.0f, colorBlue, "Depth",
    ID::randomTremoloDepth, param.value[ID::randomTremoloDepth]->getDefaultNormalized(),
    LabelPosition::right);
  addKnob(
    leftTremoloRandom + 1.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    colorBlue, "Freq", ID::randomTremoloFrequency,
    param.value[ID::randomTremoloFrequency]->getDefaultNormalized(),
    LabelPosition::right);
  addKnob(
    leftTremoloRandom + 2.0f * (knobX + margin), topTremolo - 1.5f * margin, 50.0f,
    colorBlue, "Time", ID::randomTremoloDelayTime,
    param.value[ID::randomTremoloDelayTime]->getDefaultNormalized(),
    LabelPosition::right);

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  const auto splashHeight = 40.0f;
  addSplashScreen(
    viewRect.right - 20.0f - splashWidth, viewRect.bottom - 20.0f - splashHeight,
    splashWidth, splashHeight, 20.0f, 20.0f, viewRect.right - 40.0f,
    viewRect.bottom - 40.0f, "FDNCymbal");

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
  if (iter != controlMap.end()) {
    iter->second->setValueNormalized(normalized);
    iter->second->invalid();
    return;
  }
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

void PlugEditor::addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + labelHeight;

  auto label
    = new GroupLabel(CRect(left, top, left + width, bottom), this, UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), 14.0, CTxtFace::kBoldFace));
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
    this);
  auto splash = new SplashLabel(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, 0, credit, pluginName);
  splash->setHighlightColor(colorOrange);
  frame->addView(splash);
  frame->addView(credit);
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
  float defaultValue,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(defaultValue);
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
  float defaultValue,
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
  knob->setDefaultValue(defaultValue);
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
