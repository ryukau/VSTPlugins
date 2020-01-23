// (c) 2019 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "plugeditor.hpp"
#include "x11runloop.hpp"

#include "barbox.hpp"
#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "textbutton.hpp"

#include <algorithm>

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

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // Phaser.
  const auto phaserTop = top0 - margin;
  const auto phaserLeft = left0;

  addKnob(phaserLeft, phaserTop, knobWidth, colorBlue, "Mix", ID::mix);
  addKnob(phaserLeft + knobX, phaserTop, knobWidth, colorBlue, "Freq", ID::frequency);
  addKnob(
    phaserLeft + 2.0f * knobX, phaserTop, knobWidth, colorBlue, "Spread", ID::freqSpread);
  addKnob(
    phaserLeft + 3.0f * knobX, phaserTop, knobWidth, colorRed, "Feedback", ID::feedback);
  addKnob(phaserLeft + 4.0f * knobX, phaserTop, knobWidth, colorBlue, "Range", ID::range);
  addKnob(phaserLeft + 5.0f * knobX, phaserTop, knobWidth, colorBlue, "Min", ID::min);
  addKnob(
    phaserLeft + 6.0f * knobX, phaserTop, knobWidth, colorRed, "Cas. Offset",
    ID::cascadeOffset);
  addKnob(
    phaserLeft + 7.0f * knobX, phaserTop, knobWidth, colorBlue, "L/R Offset",
    ID::stereoOffset);
  addRotaryKnob(
    phaserLeft + 8.0f * knobX, phaserTop, knobWidth + labelY, colorBlue, "Phase",
    ID::phase);

  const auto phaserTop1 = phaserTop + knobY + margin;
  const auto phaserLeft1 = left0 + 2.25f * knobX - margin;
  addLabel(phaserLeft1, phaserTop1, knobX * 1.2, "Stage");
  addTextKnob(
    phaserLeft1 + knobX, phaserTop1, knobX, colorBlue, ID::stage, Scales::stage, false, 0,
    1);

  const auto phaserLeft2 = phaserLeft1 + 2.25f * knobX;
  addLabel(phaserLeft2, phaserTop1, knobX, "Smooth");
  addTextKnob(
    phaserLeft2 + knobX, phaserTop1, knobX, colorBlue, ID::smoothness, Scales::smoothness,
    false, 3, 0);

  // Plugin name.
  const auto splashTop = phaserTop1;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.0f * knobX, splashHeight, 20.0f, 20.0f, defaultWidth - 40.0f,
    defaultHeight - 40.0f, "EsPhaser");

  return true;
}

void PlugEditor::close()
{
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

void PlugEditor::valueChanged(ParamID id, ParamValue normalized)
{
  controller->setParamNormalized(id, normalized);
  controller->performEdit(id, normalized);
}

void PlugEditor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter != controlMap.end()) {
    iter->second->setValueNormalized(normalized);
    iter->second->invalid();
    return;
  }

  using ID = Synth::ParameterID::ID;

  for (auto &ctrl : arrayControls) {
    if (id < ctrl->id.front() && id > ctrl->id.back()) continue;
    ctrl->setValueAt(id - ctrl->id.front(), normalized);
    ctrl->invalid();
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

CMouseEventResult
PlugEditor::onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  return kMouseEventNotHandled;
}

void PlugEditor::addBarBox(
  CCoord left, CCoord top, CCoord width, CCoord height, ParamID id0, UTF8String name)
{
  std::vector<ParamID> id(64);
  for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);

  std::vector<double> value(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    value[i] = controller->getParamNormalized(id[i]);

  std::vector<double> defaultValue(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    defaultValue[i] = param.value[id[i]]->getDefaultNormalized();

  auto barBox = new BarBox(
    CRect(left, top, left + width, top + height), this, id, value, defaultValue);
  barBox->setIndexFont(new CFontDesc(Style::fontName(), 10.0, CTxtFace::kBoldFace));
  barBox->setNameFont(new CFontDesc(Style::fontName(), 24.0, CTxtFace::kNormalFace));
  barBox->setBorderColor(colorBlack);
  barBox->setValueColor(colorBlue);
  barBox->setName(name);
  frame->addView(barBox);

  auto iter = std::find_if(
    arrayControls.begin(), arrayControls.end(),
    [&](const ArrayControl *elem) { return elem->id[0] == id0; });
  if (iter != arrayControls.end()) {
    (*iter)->forget();
    arrayControls.erase(iter);
  }
  barBox->remember();
  arrayControls.push_back(barBox);
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

void PlugEditor::addGroupVerticalLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto label = new VGroupLabel(
    CRect(left, top, left + labelHeight, top + width), this, UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
};

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
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  auto right = left + 70.0f;
  auto bottom = top + 230.0f;

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
  slider->setDefaultValue(param.value[tag]->getDefaultNormalized());
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

void PlugEditor::addRotaryKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new RotaryKnob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
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
  CCoord left,
  CCoord top,
  CCoord right,
  CCoord bottom,
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

template<typename Scale>
void PlugEditor::addTextKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  ParamID tag,
  Scale &scale,
  bool isDecibel,
  uint32_t precision,
  int32_t offset)
{
  auto bottom = top + labelHeight;
  auto right = left + width;

  auto knob
    = new TextKnob<Scale>(CRect(left, top, right, bottom), this, tag, scale, isDecibel);
  knob->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  knob->setPrecision(precision);
  knob->offset = offset;
  frame->addView(knob);
  addToControlMap(tag, knob);
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
