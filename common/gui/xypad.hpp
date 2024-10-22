// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "vstgui/vstgui.h"

#include "arraycontrol.hpp"
#include "style.hpp"

#include <cmath>

namespace VSTGUI {

class XYPad : public ArrayControl {
private:
  constexpr static int8_t nGrid = 8;

  CPoint mousePosition{-1, -1};
  bool isMouseEntered = false;
  bool isMouseDown = false;

  Uhhyou::Palette &pal;

  enum class AxisLock { none, x, y };

  CPoint translatePoint(CPoint &pos)
  {
    auto view = getViewSize();
    return pos - CPoint(view.left, view.top);
  }

  AxisLock getLockState(MouseEvent &event)
  {
    if (event.buttonState.isMiddle()) {
      return event.modifiers.has(ModifierKey::Shift) ? AxisLock::x : AxisLock::y;
    }
    return AxisLock::none;
  }

  void updateValueFromPos(const CPoint &pos, AxisLock lock = AxisLock::none)
  {
    if (lock != AxisLock::x) {
      value[0] = std::clamp<int>(pos.x, 0, getWidth()) / double(getWidth());
    }
    if (lock != AxisLock::y) {
      value[1]
        = std::clamp<int>(getHeight() - pos.y, 0, getHeight()) / double(getHeight());
    }
    updateValue();
  }

  void resetValue(AxisLock lock = AxisLock::none)
  {
    if (lock != AxisLock::x) value[0] = defaultValue[0];
    if (lock != AxisLock::y) value[1] = defaultValue[1];
    beginEdit();
    updateValue();
    endEdit();
  }

public:
  float borderWidth = 2.0f;
  double wheelSensitivity = 0.0001;

  explicit XYPad(
    Steinberg::Vst::VSTGUIEditor *editor,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue,
    Uhhyou::Palette &palette)
    : ArrayControl(editor, size, id, value, defaultValue), pal(palette)
  {
  }

  CLASS_METHODS(XYPad, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Background.
    pContext->setFillColor(pal.boxBackground());
    pContext->drawRect(CRect(0, 0, width, height), kDrawFilled);

    // Grid.
    pContext->setFillColor(pal.foregroundInactive());
    for (size_t ix = 1; ix < nGrid; ++ix) {
      for (size_t iy = 1; iy < nGrid; ++iy) {
        auto cx = std::floor(ix * width / nGrid);
        auto cy = std::floor(iy * height / nGrid);
        pContext->drawEllipse(CRect(cx - 2.0, cy - 2.0, cx + 2.0, cy + 2.0), kDrawFilled);
      }
    }

    // Cursor.
    if (isMouseEntered) {
      pContext->setFrameColor(pal.highlightMain());
      pContext->setLineWidth(1.0);
      pContext->drawLine(CPoint(0, mousePosition.y), CPoint(width, mousePosition.y));
      pContext->drawLine(CPoint(mousePosition.x, 0), CPoint(mousePosition.x, height));
    }

    // Value.
    double valueX = std::floor(value[0] * width);
    double valueY = std::floor((1 - value[1]) * height);
    constexpr double valR = 8.0; // Radius of value pointer circle.
    pContext->setFrameColor(pal.foreground());

    pContext->setLineWidth(2.0);
    pContext->drawEllipse(
      CRect(valueX - valR, valueY - valR, valueX + valR, valueY + valR), kDrawStroked);

    pContext->setLineWidth(1.0);
    pContext->drawLine(CPoint(0, valueY), CPoint(width, valueY));
    pContext->drawLine(CPoint(valueX, 0), CPoint(valueX, height));

    // Border.
    pContext->setLineWidth(borderWidth);
    pContext->setFrameColor(
      isMouseEntered || isMouseDown ? pal.highlightMain() : pal.border());
    pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (event.buttonState.isRight()) {
      auto componentHandler = editor->getController()->getComponentHandler();
      if (componentHandler == nullptr) return;

      using namespace Steinberg;

      FUnknownPtr<Vst::IComponentHandler3> handler(componentHandler);
      if (handler == nullptr) return;

      // Open X menu on left half, and open Y menu on right half.
      mousePosition = translatePoint(event.mousePosition);
      size_t index = mousePosition.x < getWidth() / 2.0 ? 0 : 1;
      if (index >= id.size()) return;

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return;

      menu->popup(event.mousePosition.x, event.mousePosition.y);
      menu->release();
      event.consumed = true;
    }

    if (event.modifiers.has(ModifierKey::Control)) {
      resetValue(getLockState(event));
    } else {
      mousePosition = translatePoint(event.mousePosition);
      isMouseDown = true;
      beginEdit();
      updateValueFromPos(mousePosition, getLockState(event));
    }

    invalid();
    event.consumed = true;

    return;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isMouseDown) {
      updateValueFromPos(translatePoint(event.mousePosition), getLockState(event));
      endEdit();
      isMouseDown = false;
      invalid();
      event.consumed = true;
    }
    return;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    if (!event.buttonState.isRight() && isMouseDown) {
      updateValueFromPos(translatePoint(event.mousePosition), getLockState(event));
    }
    if (isMouseEntered) {
      mousePosition = translatePoint(event.mousePosition);
      invalid();
    }
    if (isMouseEntered || isMouseDown) event.consumed = true;
    return;
  }

  virtual void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    isMouseDown = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  virtual void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    if (event.deltaY == 0) return;

    size_t index = event.modifiers.has(ModifierKey::Shift) ? 1 : 0;
    value[index] += event.deltaY * float(wheelSensitivity);

    beginEdit(index);
    updateValueAt(index);
    endEdit(index);

    event.consumed = true;
  }
};

} // namespace VSTGUI
