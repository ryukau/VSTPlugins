// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "vstgui/vstgui.h"

namespace VSTGUI {

class OptionMenu : public COptionMenu {
public:
  OptionMenu(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    CBitmap *background = nullptr,
    CBitmap *bgWhenClick = nullptr,
    const int32_t style = 0)
    : COptionMenu(size, listener, tag, background, bgWhenClick, style)
  {
  }

  CLASS_METHODS(OptionMenu, COptionMenu);

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    setFrameColor(highlightColor);
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    setFrameColor(frameColor);
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    setFrameColor(frameColor);
    COptionMenu::onMouseCancelEvent(event);
  }

  void setDefaultFrameColor(CColor color)
  {
    frameColor = color;
    setFrameColor(frameColor);
  }

  void setHighlightColor(CColor color) { highlightColor = color; }

protected:
  CColor frameColor = CColor(0xee, 0xee, 0xee, 255);
  CColor highlightColor = CColor(0, 0, 0, 255);
};

} // namespace VSTGUI
