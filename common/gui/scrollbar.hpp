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

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "vstgui/vstgui.h"

#include "style.hpp"

#include <algorithm>

namespace VSTGUI {

template<typename Scrollable> class ScrollBar : public CControl {
public:
  ScrollBar(
    const CRect &size,
    IControlListener *listener,
    Scrollable parent,
    Uhhyou::Palette &palette)
    : CControl(size, listener), parent(parent), pal(palette)
  {
    parent->remember();
  }

  ~ScrollBar() { parent->forget(); }

  CLASS_METHODS(ScrollBar, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    pContext->setLineWidth(1.0);
    pContext->setFrameColor(pal.border());

    // Bar.
    CCoord rightHandleL = rightPos * width - handleWidth;
    CCoord left = leftPos * width;
    CCoord barL = left + handleWidth;
    pContext->setFillColor(
      pointed == Part::bar ? pal.highlightButton() : pal.highlightMain());
    pContext->drawRect(
      CRect(barL - 1, 0, rightHandleL + 1, height), kDrawFilledAndStroked);

    // Left handle.
    pContext->setFillColor(
      pointed == Part::leftHandle ? pal.highlightButton() : pal.unfocused());
    pContext->drawRect(CRect(left, 0, left + handleWidth, height), kDrawFilledAndStroked);

    // Right handle.
    pContext->setFillColor(
      pointed == Part::rightHandle ? pal.highlightButton() : pal.unfocused());
    pContext->drawRect(
      CRect(rightHandleL, 0, rightHandleL + handleWidth, height), kDrawFilledAndStroked);
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    grabbed = pointed = Part::background;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (event.buttonState.isLeft()) {
      CPoint point = translateMousePosition(event.mousePosition);

      grabbed = pointed = hitTest(point);

      if (grabbed == Part::leftHandle)
        grabOffset = int(leftPos * getWidth() - point.x);
      else if (grabbed == Part::rightHandle)
        grabOffset = int(rightPos * getWidth() - point.x);
      else if (grabbed == Part::bar)
        grabOffset = int(leftPos * getWidth() - point.x);

    } else if (event.buttonState.isRight()) {
      leftPos = 0;
      rightPos = 1;
      parent->setViewRange(leftPos, rightPos);
    }
    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    grabbed = Part::background;
    invalid();
    event.consumed = true;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    CPoint point = translateMousePosition(event.mousePosition);

    auto posX = std::clamp<int>(point.x + grabOffset, 0, getWidth()) / float(getWidth());
    switch (grabbed) {
      case Part::bar: {
        auto barWidth = rightPos - leftPos;
        leftPos = posX;
        rightPos = leftPos + barWidth;
        if (leftPos < 0.0f) {
          rightPos = barWidth;
          leftPos = 0.0f;
        } else if (rightPos > 1.0f) {
          leftPos = 1.0f - barWidth;
          rightPos = 1.0f;
        }

        // Just in case.
        leftPos = std::clamp<CCoord>(leftPos, 0, 1);
        rightPos = std::clamp<CCoord>(rightPos, 0, 1);
      } break;

      case Part::leftHandle: {
        setLeftPos(posX);
      } break;

      case Part::rightHandle: {
        setRightPos(posX);
      } break;

      default:
        pointed = hitTest(point);
        invalid();
        event.consumed = true;
    }

    parent->setViewRange(leftPos, rightPos);
    invalid();

    event.consumed = true;
  }

  void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    CPoint point = translateMousePosition(event.mousePosition);

    const CCoord mouseX = float(point.x) / getWidth();
    const CCoord delta = event.deltaY;

    float amountL = 0;
    float amountR = 0;
    if (delta > 0) {
      amountL = 0.5 * zoomSensi;
      amountR = 0.5 * zoomSensi;
    } else {
      const CCoord bias = (mouseX - leftPos) / (rightPos - leftPos);
      amountL = zoomSensi * std::clamp<CCoord>(bias, 0.0, 1.0);
      amountR = zoomSensi * std::clamp<CCoord>((1.0 - bias), 0.0, 1.0);
      if (bias < 0.0 || 1.0 < bias) std::swap(amountL, amountR);
    }
    setLeftPos(leftPos - amountL * delta);
    setRightPos(rightPos + amountR * delta);

    parent->setViewRange(leftPos, rightPos);
    invalid();

    event.consumed = true;
  }

  void setHandleWidth(float width) { handleWidth = std::max(width, 0.0f); }

protected:
  enum class Part : uint8_t { background = 0, bar, leftHandle, rightHandle };

  inline CPoint translateMousePosition(const CPoint &mousePos)
  {
    auto view = getViewSize();
    return mousePos - CPoint(view.left, view.top);
  }

  inline void setLeftPos(CCoord x)
  {
    CCoord rightMost = std::max(rightPos - 3 * handleWidth / getWidth(), 0.0);
    leftPos = std::clamp<CCoord>(x, 0, rightMost);
  }

  inline void setRightPos(CCoord x)
  {
    CCoord leftMost = std::min(leftPos + 3 * handleWidth / getWidth(), 1.0);
    rightPos = std::clamp<CCoord>(x, leftMost, 1);
  }

  Part hitTest(const CPoint &point)
  {
    if (point.y < 0 || point.y > int(getHeight())) return Part::background;

    auto left = leftPos * getWidth();
    auto right = rightPos * getWidth();
    auto width = handleWidth;
    auto px = point.x;

    auto leftHandleR = left + width;
    if (px >= left && px <= leftHandleR) return Part::leftHandle;

    auto rightHandleL = right - width;
    if (px >= rightHandleL && px <= right) return Part::rightHandle;

    if (px > leftHandleR && px < rightHandleL) return Part::bar;

    return Part::background;
  }

  CCoord zoomSensi = 0.05;
  CCoord handleWidth = 10;
  CCoord leftPos = 0;
  CCoord rightPos = 1;
  int grabOffset = 0;

  Part pointed = Part::background;
  Part grabbed = Part::background;

  Scrollable parent;
  Uhhyou::Palette &pal;
};

} // namespace VSTGUI
