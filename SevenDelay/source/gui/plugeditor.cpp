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
#include "plugeditor.hpp"
#include "splash.hpp"
#include "waveview.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller)
{
  setRect(viewRect);
}

bool PlugEditor::open(void *parent, const PlatformType &platformType)
{
  if (frame) return false;

  frame = new CFrame(
    CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
  if (frame == nullptr) return false;
  frame->setBackgroundColor(colorWhite);
  frame->registerMouseObserver(this);
  frame->open(parent);

  using ID = SevenDelay::ParameterID;

  // Plugin name.
  addPluginNameLabel(30.0, 340.0, 170.0, "SevenDelay");
  addSplashScreen(
    CRect(30.0, 340.0, 190.0, 380.0),
    CRect(
      viewRect.left + 200.0,
      viewRect.top + 20.0,
      viewRect.right - 200.0,
      viewRect.bottom - 20.0));

  // Delay.
  addGroupLabel(30.0, 10.0, 480.0, "Delay");
  addVSlider(20.0, 50.0, colorBlue, "Time", ID::time);
  addVSlider(120.0, 50.0, colorBlue, "Feedback", ID::feedback);
  addVSlider(220.0, 50.0, colorBlue, "Stereo", ID::offset, "L/R Stereo Offset", true);
  addVSlider(320.0, 50.0, colorBlue, "Wet", ID::wetMix);
  addVSlider(420.0, 50.0, colorGreen, "Dry", ID::dryMix);

  addCheckbox(220.0, 330.0, "Sync", ID::tempoSync);
  addCheckbox(220.0, 360.0, "Negative", ID::negativeFeedback);

  addKnob(340.0, 330.0, colorBlue, "InS", ID::inSpread, "Input Stereo Spread");
  addKnob(340.0, 360.0, colorBlue, "InP", ID::inPan, "Input Pan", true);
  addKnob(430.0, 330.0, colorBlue, "OutS", ID::outSpread, "Output Stereo Spread");
  addKnob(430.0, 360.0, colorBlue, "OutP", ID::outPan, "Output Pan", true);

  // LFO.
  addGroupLabel(560.0, 10.0, 380.0, "LFO");
  addVSlider(550.0, 50.0, colorBlue, "Amount", ID::lfoAmount);
  addVSlider(650.0, 50.0, colorBlue, "Freq", ID::lfoFrequency);
  addVSlider(750.0, 50.0, colorBlue, "Shape", ID::lfoShape);
  addVSlider(850.0, 50.0, colorBlue, "Phase", ID::lfoInitialPhase);

  addButton(660.0, 340.0, 80.0, "Hold", ID::lfoHold, CTextButton::kOnOffStyle);
  addWaveView(WaveViewSize);

  // Misc.
  addKnob(530.0, 330.0, colorBlue, "Smooth", ID::smoothness, "Automation Smoothness");
  addKnob(530.0, 360.0, colorBlue, "Tone", ID::tone, "Delay Tone");

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
  }
  if (tag == SevenDelay::ParameterID::lfoInitialPhase) {
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
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  // width, height = 100, 270.

  auto sLeft = left + 10.0;
  auto right = sLeft + 80.0;
  auto bottom = top + 230.0;

  auto slider = new CSlider(
    CRect(sLeft, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);
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
  slider->setTooltipText(tooltip);
  frame->addView(slider);

  top = bottom + 10.0;
  bottom = top + 30.0;
  right = left + 100.0;

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
  auto bottom = top + 40.0;

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
  CColor valueColor,
  UTF8String name,
  ParamID tag,
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  // width, height = 150, 30.

  auto bottom = top + 30.0;
  auto right = left + 30.0;

  const CCoord slitWidth = 4.0;
  auto knob = new CKnob(
    CRect(left, top, right, bottom),
    this,
    tag,
    nullptr,
    nullptr,
    CPoint(0, 0),
    CKnob::kHandleCircleDrawing | CKnob::kCoronaDrawing | CKnob::kCoronaOutline
      | (drawFromCenter ? CKnob::kCoronaFromCenter : 0));
  knob->setColorHandle(valueColor);
  knob->setCoronaColor(valueColor);
  knob->setColorShadowHandle(colorFaintGray);
  knob->setHandleLineWidth(slitWidth);
  knob->setCoronaInset(slitWidth);
  knob->setInsetValue(slitWidth);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setTooltipText(tooltip);
  frame->addView(knob);

  left = right + 10.0;
  right = left + 100.0;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(new CFontDesc(fontName, fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setHoriAlign(CHoriTxtAlign::kLeftText);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  label->sizeToFit();
  frame->addView(label);
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
