// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "vstgui/vstgui.h"

namespace VSTGUI {

class Slider : public CSlider {
public:
  Slider(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    int32_t iMinPos,
    int32_t iMaxPos,
    CBitmap *handle,
    CBitmap *background,
    const CPoint &offset = CPoint(0, 0),
    const int32_t style = kLeft | kHorizontal)
    : CSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
  {
    setFrameColor(frameColor);
  }

  CLASS_METHODS(Slider, CSlider);

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    setFrameWidth(highlightFrameWidth);
    setFrameColor(highlightColor);
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    setFrameWidth(frameWidth);
    setFrameColor(frameColor);
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    setFrameWidth(frameWidth);
    setFrameColor(frameColor);
    CSlider::onMouseCancelEvent(event);
  }

  void setDefaultFrameColor(CColor color)
  {
    frameColor = color;
    setFrameColor(frameColor);
  }

  void setHighlightColor(CColor color) { highlightColor = color; }

  void setDefaultFrameWidth(float width)
  {
    frameWidth = width;
    setFrameWidth(frameWidth);
  }

  void setHighlightWidth(float width) { highlightFrameWidth = width; }

protected:
  CColor frameColor = CColor(0xee, 0xee, 0xee, 255);
  CColor highlightColor = CColor(0, 0, 0, 255);
  float frameWidth = 1.0f;
  float highlightFrameWidth = 2.0f;
};

} // namespace VSTGUI
