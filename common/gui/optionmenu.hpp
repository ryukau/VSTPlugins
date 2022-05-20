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
