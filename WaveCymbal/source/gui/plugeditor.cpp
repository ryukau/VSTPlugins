// (c) 2019 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

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
  const auto left0 = 10.0f;
  const auto top0 = 20.0f;

  const auto leftGain = left0 + 2.0f * margin;
  addVSlider(
    leftGain, top0, colorBlue, "Gain", ID::gain,
    param.value[ID::gain]->getDefaultNormalized());

  // Excitation.
  const auto leftExcitation = leftGain + sliderX + 2.0f * margin;
  addButton(
    leftExcitation, top0, 2.0f * knobX, "Excitation", ID::excitation,
    CTextButton::kOnOffStyle);

  const auto topExcitation = top0 + labelHeight + margin;
  addKnob(
    leftExcitation, topExcitation, knobWidth, colorRed, "Feedback", ID::pickCombFeedback,
    param.value[ID::pickCombFeedback]->getDefaultNormalized());
  addKnob(
    leftExcitation + knobX, topExcitation, knobWidth, colorBlue, "Time", ID::pickCombTime,
    param.value[ID::pickCombTime]->getDefaultNormalized());

  // Objects.
  const auto leftObjects = leftExcitation + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftObjects, top0, 2.0f * knobX, "Objects");

  const auto topObjects = top0 + labelHeight + margin;
  addNumberKnob(
    leftObjects, topObjects, knobWidth, colorBlue, "nCymbal", ID::nCymbal,
    Scales::nCymbal, 1, param.value[ID::nCymbal]->getDefaultNormalized());
  addNumberKnob(
    leftObjects + knobX, topObjects, knobWidth, colorBlue, "nString", ID::stack,
    Scales::stack, 1, param.value[ID::stack]->getDefaultNormalized());

  // Wave.
  const auto leftWave = leftObjects + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftWave, top0, 3.0f * knobX, "Wave");

  const auto topWave = top0 + labelHeight + margin;
  addKnob(
    leftWave, topWave, knobWidth, colorRed, "Damping", ID::damping,
    param.value[ID::damping]->getDefaultNormalized());
  addKnob(
    leftWave + knobX, topWave, knobWidth, colorBlue, "PulsePosition", ID::pulsePosition,
    param.value[ID::pulsePosition]->getDefaultNormalized());
  addKnob(
    leftWave + 2.0f * knobX, topWave, knobWidth, colorBlue, "PulseWidth", ID::pulseWidth,
    param.value[ID::pulseWidth]->getDefaultNormalized());

  // Bandpass.
  const auto top1 = top0 + knobY + 3.0f * margin;

  // Collision.
  const auto leftCollision = leftExcitation;
  addButton(
    leftCollision, top1, checkboxWidth, "Collision", ID::collision,
    CTextButton::kOnOffStyle);

  const auto topCollision = top1 + labelHeight + margin;
  addKnob(
    leftCollision, topCollision, knobWidth, colorRed, "Distance", ID::distance,
    param.value[ID::distance]->getDefaultNormalized());

  // Random.
  const auto leftRandom = leftCollision + knobX + 4.0f * margin;
  addGroupLabel(leftRandom, top1, 2.0f * knobX, "Random");

  const auto topRandom = top1 + labelHeight + margin;
  addNumberKnob(
    leftRandom, topRandom, knobWidth, colorBlue, "Seed", ID::seed, Scales::seed, 0,
    param.value[ID::seed]->getDefaultNormalized());
  addKnob(
    leftRandom + knobX, topRandom, knobWidth, colorRed, "Amount", ID::randomAmount,
    param.value[ID::randomAmount]->getDefaultNormalized());

  // String.
  const auto leftString = leftRandom + 2.0f * knobX + 4.0f * margin;
  addGroupLabel(leftString, top1, 4.0f * knobX, "String");

  const auto topString = top1 + labelHeight + margin;
  addKnob(
    leftString, topString, knobWidth, colorBlue, "MinHz", ID::minFrequency,
    param.value[ID::minFrequency]->getDefaultNormalized());
  addKnob(
    leftString + knobX, topString, knobWidth, colorBlue, "MaxHz", ID::maxFrequency,
    param.value[ID::maxFrequency]->getDefaultNormalized());
  addKnob(
    leftString + 2.0f * knobX, topString, knobWidth, colorBlue, "Decay", ID::decay,
    param.value[ID::bandpassQ]->getDefaultNormalized());
  addKnob(
    leftString + 3.0f * knobX, topString, knobWidth, colorBlue, "Q", ID::bandpassQ,
    param.value[ID::decay]->getDefaultNormalized());

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
    leftSmoothness + 22.5f, top2 - margin, 50.0f, colorBlue, "Smoothness", ID::smoothness,
    param.value[ID::smoothness]->getDefaultNormalized());

  // Plugin name.
  const auto splashWidth = 3.0f * knobX;
  addSplashScreen(
    viewRect.right - 20.0f - splashWidth, top2 + 2.0 * margin, splashWidth, 40.0f, 100.0f,
    20.0f, viewRect.right - 200.0f, viewRect.bottom - 40.0f, "WaveCymbal");

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
