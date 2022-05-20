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
