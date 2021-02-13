// (c) 2021 Takamitsu Endo
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

#pragma once

#include "vstgui/vstgui.h"

#include "arraycontrol.hpp"
#include "style.hpp"

#include <cmath>

namespace VSTGUI {

class XYPad : public ArrayControl {
private:
  constexpr static int8_t nGrid = 8;

  CPoint cursor{-1, -1};
  bool isMouseEntered = false;
  bool isMouseLeftDown = false;

  Uhhyou::Palette &pal;

public:
  float borderWidth = 2.0f;

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
        auto cx = ix * width / nGrid;
        auto cy = iy * height / nGrid;

        pContext->drawEllipse(CRect(cx - 2.0, cy - 2.0, 4.0, 4.0), kDrawFilled);

        // beginPath();
        // circle(ix * getWidth() / nGrid, iy * getHeight() / nGrid, 2.0f);
        // fill();
      }
    }

    // Cursor.
    if (isMouseEntered) {
      pContext->setFrameColor(pal.highlightMain());
      pContext->setLineWidth(1.0);
      pContext->drawLine(CPoint(0, cursor.y), CPoint(width, cursor.y));
      pContext->drawLine(CPoint(cursor.x, 0), CPoint(cursor.x, height));

      // strokeColor(pal.highlightMain());

      // beginPath();
      // moveTo(0, cursor.getY());
      // lineTo(width, cursor.getY());
      // strokeWidth(1.0f);
      // stroke();

      // beginPath();
      // moveTo(cursor.getX(), 0);
      // lineTo(cursor.getX(), height);
      // strokeWidth(1.0f);
      // stroke();
    }

    // Value.
    double valueX = value[0] * width;
    double valueY = value[1] * height;
    constexpr double valR = 8.0; // Radius of value pointer circle.
    pContext->setFrameColor(pal.foreground());

    pContext->setLineWidth(2.0);
    pContext->drawEllipse(
      CRect(valueX - valR, valueY - valR, 2 * valR, 2 * valR), kDrawStroked);

    pContext->setLineWidth(1.0);
    pContext->drawLine(CPoint(0, valueY), CPoint(width, valueY));
    pContext->drawLine(CPoint(valueX, 0), CPoint(valueX, height));

    // beginPath();
    // circle(valueX, valueY, 8.0f);
    // strokeWidth(2.0f);
    // strokeColor(pal.foreground());
    // stroke();

    // beginPath();
    // moveTo(0, valueY);
    // lineTo(width, valueY);
    // strokeWidth(1.0f);
    // stroke();

    // beginPath();
    // moveTo(valueX, 0);
    // lineTo(valueX, height);
    // strokeWidth(1.0f);
    // stroke();

    // Border.
    pContext->setLineWidth(borderWidth);
    pContext->setFrameColor(
      isMouseEntered || isMouseLeftDown ? pal.highlightMain() : pal.border());
    pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);

    // beginPath();
    // rect(0, 0, width, height);
    // strokeWidth(borderWidth);
    // strokeColor(isMouseEntered || isMouseLeftDown ? pal.highlightMain() :
    // pal.border()); stroke();
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isRightButton()) {
      auto componentHandler = editor->getController()->getComponentHandler();
      if (componentHandler == nullptr) return kMouseEventNotHandled;

      using namespace Steinberg;

      FUnknownPtr<Vst::IComponentHandler3> handler(componentHandler);
      if (handler == nullptr) return kMouseEventNotHandled;

      // Open X menu on left half, and open Y menu on right half.
      cursor = where;
      size_t index = cursor.x < getWidth() / 2.0 ? 0 : 1;
      if (index >= id.size()) return kMouseEventNotHandled;

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return kMouseEventNotHandled;

      menu->popup(where.x, where.y);
      menu->release();
      return kMouseEventHandled;
    }

    if (buttons.isLeftButton()) {
      cursor = where;
      isMouseLeftDown = true;
      updateValueFromPos(where);
      invalid();
      return kMouseEventHandled;
    }

    return kMouseEventNotHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) {
      updateValueFromPos(where);
      isMouseLeftDown = false;
      invalid();
      return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
  }

  // bool onMouse(const MouseEvent &ev) override
  // {
  //   if (ev.press && contains(ev.pos)) {
  //     if (ev.button == 1) {
  //       isMouseLeftDown = true;
  //       updateValueFromPos(ev.pos);
  //     }
  //     repaint();
  //     return true;
  //   }
  //   isMouseLeftDown = false;
  //   repaint();
  //   return false;
  // }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) updateValueFromPos(where);
    if (isMouseEntered) {
      cursor = where;
      invalid();
    }
    if (isMouseEntered || isMouseLeftDown) return kMouseEventHandled;
    return kMouseEventNotHandled;
  }

  // bool onMotion(const MotionEvent &ev) override
  // {
  //   if (isMouseLeftDown) {
  //     updateValueFromPos(ev.pos);
  //   }
  //   isMouseEntered = contains(ev.pos);
  //   if (isMouseEntered) cursor = ev.pos;
  //   repaint();
  //   return isMouseEntered || isMouseLeftDown;
  // }

  virtual CMouseEventResult onMouseCancel() override
  {
    isMouseLeftDown = false;
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

private:
  void updateValueFromPos(const CPoint &pos)
  {
    value[0] = std::clamp<int>(pos.x, 0, getWidth()) / double(getWidth());
    value[1] = std::clamp<int>(pos.y, 0, getHeight()) / double(getHeight());
    updateValue();
  }
};

} // namespace VSTGUI
