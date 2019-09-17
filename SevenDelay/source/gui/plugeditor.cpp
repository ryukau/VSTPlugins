// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "../parameter.hpp"
#include "knob.hpp"
#include "plugeditor.hpp"
#include "splash.hpp"
#include "waveview.hpp"

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

  using ID = SevenDelay::ParameterID;
  SevenDelay::GlobalParameter param;

  const auto normalWidth = 80.0;
  const auto normalHeight = normalWidth + 40.0;
  const auto smallWidth = 40.0;
  const auto smallHeight = 50.0;
  const auto interval = 100.0;

  // clang-format off

  // Delay.
  const auto delayTop1 = 50.0;
  const auto delayLeft = 20.0;
  addGroupLabel(delayLeft, 10.0, 480.0, "Delay");
  addKnob(delayLeft, delayTop1, normalWidth, colorBlue, "Time", ID::time, param.time);
  addKnob(1.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Feedback", ID::feedback, param.feedback);
  addKnob(2.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Stereo", ID::offset, param.offset, LabelPosition::bottom, "L/R Stereo Offset");
  addKnob(3.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Wet", ID::wetMix, param.wetMix);
  addKnob(4.0 * interval + delayLeft, delayTop1, normalWidth, colorGreen, "Dry", ID::dryMix, param.dryMix);

  const auto delayTop2 = delayTop1 + normalHeight;
  const auto delayTop3 = delayTop2 + smallHeight;
  const auto delayTop4 = delayTop3 + smallHeight;
  addCheckbox(delayLeft + 10.0, delayTop2, "Sync", ID::tempoSync);
  addCheckbox(delayLeft + 10.0, delayTop3, "Negative", ID::negativeFeedback);

  addKnob(1.0 * interval + delayLeft, delayTop2, smallWidth, colorBlue, "Input Spread", ID::inSpread, param.inSpread, LabelPosition::right, "Input Stereo Spread");
  addKnob(1.0 * interval + delayLeft, delayTop3, smallWidth, colorBlue, "Output Spread", ID::outSpread, param.outSpread, LabelPosition::right, "Output Stereo Spread");
  addKnob(2.4 * interval + delayLeft, delayTop2, smallWidth, colorBlue, "Input Pan", ID::inPan, param.inPan, LabelPosition::right, "Input Pan");
  addKnob(2.4 * interval + delayLeft, delayTop3, smallWidth, colorBlue, "Output Pan", ID::outPan, param.outPan, LabelPosition::right, "Output Pan");

  addKnob(3.8 * interval + delayLeft, delayTop2, smallWidth, colorBlue, "Allpass", ID::tone, param.tone, LabelPosition::right, "Delay Allpass");
  addKnob(3.8 * interval + delayLeft, delayTop3, smallWidth, colorBlue, "DC Kill", ID::dckill, param.dckill, LabelPosition::right, "DC Kill");
  addKnob(3.8 * interval + delayLeft, delayTop4,  smallWidth, colorBlue, "Smooth", ID::smoothness, param.smoothness, LabelPosition::right, "Automation Smoothness");

  // Plugin name.
  const auto nameLeft = delayLeft + interval;
  const auto nameTop = viewRect.bottom - 50.0;
  const auto nameWidth = 180.0;
  addPluginNameLabel(nameLeft, nameTop, nameWidth, "SevenDelay");
  addSplashScreen(
    CRect(nameLeft, nameTop, nameLeft + nameWidth, nameTop + 40.0),
    CRect(viewRect.left + 200.0, viewRect.top + 20.0, viewRect.right - 200.0, viewRect.bottom - 20.0));

  // LFO.
  // 750 - 520 = 230 / 3 = 66 + 10
  const auto lfoTop1 = 50.0;
  const auto lfoLeft1 = 520.0;
  addGroupLabel(520.0, 10.0, 420.0, "LFO");
  addVSlider(lfoLeft1, 50.0, colorBlue, "To Time", ID::lfoTimeAmount, param.lfoTimeAmount);
  addVSlider(lfoLeft1 + 75.0, 50.0, colorBlue, "To Allpass", ID::lfoToneAmount, param.lfoToneAmount);
  addVSlider(lfoLeft1 + 150.0, 50.0, colorGreen, "Frequency", ID::lfoFrequency, param.lfoFrequency);
  const auto lfoLeft2 = lfoLeft1 + 230.0;
  addKnob(lfoLeft2, 50.0, normalWidth, colorBlue, "Shape", ID::lfoShape, param.lfoShape);
  addKnob(interval + lfoLeft2, 50.0, normalWidth, colorBlue, "Phase", ID::lfoInitialPhase, param.lfoInitialPhase);

  addButton(WaveViewSize.left, WaveViewSize.bottom + 10.0, WaveViewSize.right - WaveViewSize.left, "LFO Hold", ID::lfoHold, CTextButton::kOnOffStyle);
  addWaveView(WaveViewSize);

  // clang-format on

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

  if (tag == SevenDelay::ParameterID::lfoShape) {
    auto some = dynamic_cast<WaveView *>(frame->getViewAt(WaveViewPos));
    if (some == nullptr) return;
    some->shape = getPlainValue(tag);
    some->setDirty(true);
  } else if (tag == SevenDelay::ParameterID::lfoInitialPhase) {
    auto some = dynamic_cast<WaveView *>(frame->getViewAt(WaveViewPos));
    if (some == nullptr) return;
    some->phase = getPlainValue(tag);
    some->setDirty(true);
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

void PlugEditor::addPluginNameLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + 40.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  label->setFont(
    new CFontDesc(fontName, 24.0, CTxtFace::kBoldFace | CTxtFace::kItalicFace));
  label->setStyle(0);
  label->setFrameWidth(1.0);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
}

void PlugEditor::addLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font)
{
  auto bottom = top + 20.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  if (font == nullptr) font = new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace);
  label->setFont(font);
  label->setStyle(CTextLabel::kNoFrame);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->sizeToFit();
  frame->addView(label);
}

void PlugEditor::addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + 30.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  label->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  label->setStyle(0);
  label->setFrameWidth(frameWidth);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
}

void PlugEditor::addWaveView(const CRect &size)
{
  auto view = new WaveView(size);
  view->shape = getPlainValue(SevenDelay::ParameterID::lfoShape);
  view->phase = getPlainValue(SevenDelay::ParameterID::lfoInitialPhase);
  frame->addView(view);
}

void PlugEditor::addSplashScreen(CRect &buttonRect, CRect splashRect)
{
  auto credit = new CreditView(splashRect, nullptr);
  auto splash = new CSplashScreen(buttonRect, this, -666, credit);
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
  // width, height = 100, 270.

  auto right = left + 70.0;
  auto bottom = top + 230.0;

  auto slider = new CSlider(
    CRect(left, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);
  slider->setSliderMode(CSliderMode::FreeClick);
  slider->setStyle(CSlider::kBottom | CSlider::kVertical);
  slider->setDrawStyle(
    CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
    | (drawFromCenter ? CSlider::kDrawValueFromCenter | CSlider::kDrawInverted : 0));
  slider->setFrameWidth(frameWidth);
  slider->setFrameColor(colorBlack);
  slider->setBackColor(colorWhite);
  slider->setValueColor(valueColor);
  slider->setValueNormalized(controller->getParamNormalized(tag));
  slider->setDefaultValue(defaultValue);
  slider->setTooltipText(tooltip);
  frame->addView(slider);

  top = bottom + 10.0;
  bottom = top + 30.0;
  right = left + 70.0;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
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
  auto bottom = top + 30.0;

  auto button = new CTextButton(
    CRect(left, top, right, bottom), this, tag, title, (CTextButton::Style)style);
  button->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  button->setTextColor(colorBlack);
  button->setTextColorHighlighted(colorBlack);
  button->setGradient(CGradient::create(0.0, 1.0, colorWhite, colorWhite));
  button->setGradientHighlighted(CGradient::create(0.0, 1.0, colorOrange, colorOrange));
  button->setFrameColor(colorBlack);
  button->setFrameColorHighlighted(colorBlack);
  button->setFrameWidth(1.0);
  button->setRoundRadius(0.0);
  button->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(button);
}

void PlugEditor::addCheckbox(
  CCoord left, CCoord top, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + 100.0;
  auto bottom = top + 30.0;

  auto checkbox
    = new CCheckBox(CRect(left, top, right, bottom), this, tag, title, nullptr, style);
  checkbox->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  checkbox->setFontColor(colorBlack);
  checkbox->setBoxFrameColor(colorBlack);
  checkbox->setBoxFillColor(colorWhite);
  checkbox->setCheckMarkColor(colorBlue);
  checkbox->sizeToFit();
  checkbox->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(checkbox);
}

void PlugEditor::addOptionMenu(
  CCoord left, CCoord top, ParamID tag, const std::vector<UTF8String> &items)
{
  auto right = left + 100.0;
  auto bottom = top + 40.0;

  auto menu = new COptionMenu(
    CRect(left, top, right, bottom),
    this,
    tag,
    nullptr,
    nullptr,
    COptionMenu::kCheckStyle);
  for (const auto &item : items) menu->addEntry(item);
  menu->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  menu->setFontColor(colorBlack);
  menu->setBackColor(colorWhite);
  menu->setFrameColor(colorBlack);
  menu->sizeToFit();
  menu->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(menu);
}

void PlugEditor::addKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  float defaultValue,
  LabelPosition labelPosition,
  UTF8StringPtr tooltip)
{
  auto bottom = top + width;
  auto right = left + width;

  auto knob = new Knob(CRect(left, top, right, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(defaultValue);
  knob->setTooltipText(tooltip);
  frame->addView(knob);

  switch (labelPosition) {
    default:
    case LabelPosition::bottom:
      top = bottom;
      bottom = top + 30.0;
      break;

    case LabelPosition::right:
      left = right + 10.0;
      right = left + 100.0;
      break;
  }

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
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
