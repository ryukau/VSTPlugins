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
    double valueX = value[0] * width;
    double valueY = value[1] * height;
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
      isMouseEntered || isMouseLeftDown ? pal.highlightMain() : pal.border());
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

    if (event.buttonState.isLeft()) {
      mousePosition = translatePoint(event.mousePosition);
      isMouseLeftDown = true;
      updateValueFromPos(mousePosition);
      invalid();
      event.consumed = true;
    }

    return;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (event.buttonState.isLeft()) {
      updateValueFromPos(translatePoint(event.mousePosition));
      isMouseLeftDown = false;
      invalid();
      event.consumed = true;
    }
    return;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    if (event.buttonState.isLeft()) {
      updateValueFromPos(translatePoint(event.mousePosition));
    }
    if (isMouseEntered) {
      mousePosition = translatePoint(event.mousePosition);
      invalid();
    }
    if (isMouseEntered || isMouseLeftDown) event.consumed = true;
    return;
  }

  virtual void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    isMouseLeftDown = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

private:
  CPoint translatePoint(CPoint &pos)
  {
    auto view = getViewSize();
    return pos - CPoint(view.left, view.top);
  }

  void updateValueFromPos(const CPoint &pos)
  {
    value[0] = std::clamp<int>(pos.x, 0, getWidth()) / double(getWidth());
    value[1] = std::clamp<int>(pos.y, 0, getHeight()) / double(getHeight());
    updateValue();
  }
};

} // namespace VSTGUI
