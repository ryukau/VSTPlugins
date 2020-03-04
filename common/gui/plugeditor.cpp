// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "plugeditor.hpp"
#include "x11runloop.hpp"

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller) { setRect(viewRect); }

PlugEditor::~PlugEditor()
{
  for (auto &ctrl : controlMap)
    if (ctrl.second) ctrl.second->forget();

  for (auto &ctrl : arrayControls)
    if (ctrl) ctrl->forget();
}

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

  return prepareUI();
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

void PlugEditor::addSplashScreen(
  float buttonLeft,
  float buttonTop,
  float buttonWidth,
  float buttonHeight,
  float splashLeft,
  float splashTop,
  float splashWidth,
  float splashHeight,
  const char *pluginName,
  float buttonFontSize)
{
  auto credit = new CreditView(
    CRect(splashLeft, splashTop, splashLeft + splashWidth, splashTop + splashHeight),
    this);
  auto splash = new SplashLabel(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, 0, credit, pluginName, buttonFontSize);
  splash->setHighlightColor(colorOrange);
  frame->addView(splash);
  frame->addView(credit);
}

BarBox *PlugEditor::addBarBox(
  CCoord left,
  CCoord top,
  CCoord width,
  CCoord height,
  ParamID id0,
  size_t nBar,
  std::string name)
{
  std::vector<ParamID> id(nBar);
  for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);
  std::vector<double> value(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    value[i] = controller->getParamNormalized(id[i]);
  std::vector<double> defaultValue(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    defaultValue[i] = param->getDefaultNormalized(id[i]);

  auto barBox = new BarBox(
    getController(), CRect(left, top, left + width, top + height), this, id, value,
    defaultValue);
  barBox->setIndexFont(
    new CFontDesc(PlugEditorStyle::fontName(), 10.0, CTxtFace::kBoldFace));
  barBox->setNameFont(
    new CFontDesc(PlugEditorStyle::fontName(), 24.0, CTxtFace::kNormalFace));
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
  return barBox;
}

CTextLabel *PlugEditor::addLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font)
{
  auto bottom = top + 20.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  if (font == nullptr)
    font = new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace);
  label->setFont(font);
  label->setHoriAlign(CHoriTxtAlign::kCenterText);
  label->setStyle(CTextLabel::kNoFrame);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
  return label;
}

GroupLabel *
PlugEditor::addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + labelHeight;

  auto label
    = new GroupLabel(CRect(left, top, left + width, bottom), this, UTF8String(name));
  label->setFont(new CFontDesc(PlugEditorStyle::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
  return label;
}

VGroupLabel *
PlugEditor::addGroupVerticalLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  return nullptr;

  // VSTGUI 4.9 can't draw roteted text.
  /*
  auto label = new VGroupLabel(
    CRect(left, top, left + labelHeight, top + width), this, UTF8String(name));
  label->setFont(new CFontDesc(PlugEditorStyle::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
  return label;
  */
};

std::tuple<Slider *, CTextLabel *> PlugEditor::addVSlider(
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
  slider->setDefaultValue(param->getDefaultNormalized(tag));
  slider->setTooltipText(tooltip);
  frame->addView(slider);
  addToControlMap(tag, slider);

  top = bottom + margin;
  bottom = top + labelHeight;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  frame->addView(label);

  return std::make_tuple(slider, label);
}

TextButton *PlugEditor::addButton(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + labelHeight;

  auto button = new TextButton(
    CRect(left, top, right, bottom), this, tag, title, (CTextButton::Style)style);
  button->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
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
  return button;
}

MessageButton *PlugEditor::addStateButton(
  CCoord left,
  CCoord top,
  CCoord width,
  std::string label,
  std::string messageID,
  std::string)
{
  auto right = left + width;
  auto bottom = top + 2.0 * labelHeight;

  auto button = new MessageButton(
    controller, CRect(left, top, right, bottom), label, messageID,
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setHighlightColor(colorRed);
  frame->addView(button);
  return button;
}

KickButton *PlugEditor::addKickButton(
  CCoord left, CCoord top, CCoord width, std::string title, ParamID tag)
{
  auto right = left + width;
  auto bottom = top + 2.0 * labelHeight;

  auto button = new KickButton(
    CRect(left, top, right, bottom), this, tag, title,
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setHighlightColor(colorRed);
  frame->addView(button);
  return button;
}

CheckBox *PlugEditor::addCheckbox(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + 20.0;

  auto checkbox
    = new CheckBox(CRect(left, top, right, bottom), this, tag, title, nullptr, style);
  checkbox->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  checkbox->setFontColor(colorBlack);
  checkbox->setBoxFrameColor(colorBlack);
  checkbox->setBoxFillColor(colorWhite);
  checkbox->setCheckMarkColor(colorBlue);
  checkbox->sizeToFit();
  checkbox->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(checkbox);
  addToControlMap(tag, checkbox);
  return checkbox;
}

OptionMenu *PlugEditor::addOptionMenu(
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
  menu->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  menu->setFontColor(colorBlack);
  menu->setBackColor(colorWhite);
  menu->setDefaultFrameColor(colorBlack);
  menu->setHighlightColor(colorBlue);
  menu->sizeToFit();
  menu->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(menu);
  addToControlMap(tag, menu);
  return menu;
}

std::tuple<Knob *, CTextLabel *> PlugEditor::addKnob(
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
  knob->setDefaultValue(param->getDefaultNormalized(tag));
  frame->addView(knob);
  addToControlMap(tag, knob);

  auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
  return std::make_tuple(knob, label);
}

std::tuple<RotaryKnob *, CTextLabel *> PlugEditor::addRotaryKnob(
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
  knob->setDefaultValue(param->getDefaultNormalized(tag));
  frame->addView(knob);
  addToControlMap(tag, knob);

  auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
  return std::make_tuple(knob, label);
}

CTextLabel *PlugEditor::addKnobLabel(
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
  label->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(CColor{0, 0, 0, 0});
  if (labelPosition == LabelPosition::right) label->sizeToFit();
  frame->addView(label);
  return label;
}

TextView *PlugEditor::addTextView(
  CCoord left, CCoord top, CCoord width, CCoord height, std::string text, CCoord textSize)
{
  auto bottom = top + height;
  auto right = left + width;

  auto view = new TextView(
    CRect(left, top, right, bottom), text,
    new CFontDesc(PlugEditorStyle::fontName(), textSize, CTxtFace::kNormalFace));
  frame->addView(view);
  return view;
}

TextTableView *PlugEditor::addTextTableView(
  CCoord left,
  CCoord top,
  CCoord width,
  CCoord height,
  std::string text,
  float cellWidth,
  CCoord textSize)
{
  auto bottom = top + height;
  auto right = left + width;

  auto view = new TextTableView(
    CRect(left, top, right, bottom), text, cellWidth,
    new CFontDesc(PlugEditorStyle::fontName(), textSize, CTxtFace::kNormalFace));
  frame->addView(view);
  return view;
}

void PlugEditor::addToControlMap(Vst::ParamID id, CControl *control)
{
  auto iter = controlMap.find(id);
  if (iter != controlMap.end()) iter->second->forget();
  control->remember();
  controlMap.insert({id, control});
}

} // namespace Vst
} // namespace Steinberg
