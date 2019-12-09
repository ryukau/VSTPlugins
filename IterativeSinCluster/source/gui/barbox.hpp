// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "vstgui/vstgui.h"

#include "arraycontrol.hpp"
#include "plugeditor.hpp"

namespace VSTGUI {

class BarBox : public ArrayControl {
public:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  BarBox(
    const CRect &size,
    Steinberg::Vst::PlugEditor *editor,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue)
    : ArrayControl(size, id, value, defaultValue)
    , editor(editor)
    , sliderWidth((size.right - size.left) / value.size())
  {
    editor->remember();
  }

  ~BarBox()
  {
    if (editor) editor->forget();
    if (fontID) fontID->forget();
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(BarBox, CView);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void valueChangedAt(size_t index)
  {
    if (!getFrame() || editor == nullptr) return;
    getFrame()->beginEdit(id[index]);
    editor->valueChanged(id[index], value[index]);
    getFrame()->endEdit(id[index]);
  }

  void valueChanged()
  {
    if (id.size() != value.size()) return;
    for (size_t i = 0; i < id.size(); ++i) valueChangedAt(i);
  }

  void setFont(CFontRef fontID) { this->fontID = fontID; }
  void setHighlightColor(CColor color) { highlightColor = color; }
  void setValueColor(CColor color) { valueColor = color; }
  void setBorderColor(CColor color) { borderColor = color; }
  void setDefaultBorderWidth(float width) { defaultBorderWidth = width; }
  void setHighlightBorderWidth(float width) { highlightBorderWidth = width; }

protected:
  void setValueFromPosition(CPoint &position, const CButtonState &buttons);

  CColor highlightColor{0x00, 0xff, 0x00, 0x33};
  CColor splitterColor{0xff, 0xff, 0xff};
  CColor valueColor{0xdd, 0xdd, 0xdd};
  CColor borderColor{0, 0, 0};

  CFontRef fontID = nullptr;

  CCoord defaultBorderWidth = 2.0;
  CCoord highlightBorderWidth = 4.0;

  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CCoord sliderWidth;
};

void BarBox::draw(CDrawContext *pContext)
{
  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  // Value bar and Text.
  pContext->setFillColor(valueColor);
  pContext->setFont(fontID);
  pContext->setFontColor(borderColor);
  for (size_t i = 0; i < value.size(); ++i) {
    auto sliderHeight = value[i] * height;
    auto left = round(i * sliderWidth);
    auto right = round(left + sliderWidth);
    auto top = round(height - sliderHeight);
    pContext->drawRect(CRect(left, top, right, height), kDrawFilled);
    pContext->drawString(
      UTF8String(std::to_string(i + 1).c_str()),
      CRect(left, height - 16, right, height - 4)); // TODO
  }

  // Splitter.
  pContext->setLineWidth(defaultBorderWidth);
  pContext->setFrameColor(splitterColor);
  for (size_t i = 0; i < value.size(); ++i) {
    auto x = i * sliderWidth;
    pContext->drawLine(CPoint(x, 0), CPoint(x, height));
  }

  // Border.
  pContext->setFrameColor(borderColor);
  pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);

  // Highlight.
  if (isMouseEntered) {
    size_t index = size_t(value.size() * mousePosition.x / width);
    if (index < value.size()) {
      pContext->setFillColor(highlightColor);
      auto left = index * sliderWidth;
      pContext->drawRect(CRect(left, 0, left + sliderWidth, height), kDrawFilled);
    }
  }

  setDirty(false);
}

CMouseEventResult BarBox::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (buttons.isLeftButton() || buttons.isMiddleButton()) {
    auto view = getViewSize();
    mousePosition = where - CPoint(view.left, view.top);
    setValueFromPosition(mousePosition, buttons);
    return kMouseEventHandled;
  } else if (buttons.isRightButton()) {
    auto componentHandler = editor->getController()->getComponentHandler();
    if (componentHandler == nullptr) return kMouseEventNotHandled;

    using namespace Steinberg;

    FUnknownPtr<Vst::IComponentHandler3> handler(componentHandler);
    if (handler == nullptr) return kMouseEventNotHandled;

    auto view = getViewSize();
    mousePosition = where - CPoint(view.left, view.top);
    size_t index = size_t(mousePosition.x / sliderWidth);

    Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
    if (menu == nullptr) return kMouseEventNotHandled;

    menu->popup(where.x, where.y);
    menu->release();
    return kMouseEventHandled;
  }
  return kMouseEventNotHandled;
}

CMouseEventResult BarBox::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  auto view = getViewSize();
  mousePosition = where - CPoint(view.left, view.top);
  invalid();
  if (!buttons.isLeftButton() && !buttons.isMiddleButton()) return kMouseEventNotHandled;
  setValueFromPosition(mousePosition, buttons);
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseCancel()
{
  if (isDirty()) {
    valueChanged();
    invalid();
  }
  return kMouseEventHandled;
}

void BarBox::setValueFromPosition(CPoint &position, const CButtonState &buttons)
{
  size_t index = size_t(position.x / sliderWidth);
  if (index >= value.size()) return;
  if (buttons.isLeftButton()) {
    if (buttons & kControl)
      setValueAt(index, defaultValue[index]);
    else
      setValueAt(index, 1.0 - double(position.y) / getHeight());
  } else if (buttons.isMiddleButton()) {
    if (buttons & kControl)
      setValueAt(index, 1);
    else
      setValueAt(index, 0);
  }
  valueChangedAt(index);
  invalid();
}

} // namespace VSTGUI
