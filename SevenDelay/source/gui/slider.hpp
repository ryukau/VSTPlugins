// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

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

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameColor(CColor color);
  void setHighlightColor(CColor color);
  void setDefaultFrameWidth(float width);
  void setHighlightWidth(float width);

protected:
  CColor frameColor = CColor(0xee, 0xee, 0xee, 255);
  CColor highlightColor = CColor(0, 0, 0, 255);
  float frameWidth = 1.0f;
  float highlightFrameWidth = 2.0f;
};

CMouseEventResult Slider::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(highlightFrameWidth);
  setFrameColor(highlightColor);
  return kMouseEventHandled;
}

CMouseEventResult Slider::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(frameWidth);
  setFrameColor(frameColor);
  return kMouseEventHandled;
}

CMouseEventResult Slider::onMouseCancel()
{
  setFrameWidth(frameWidth);
  setFrameColor(frameColor);
  return CSlider::onMouseCancel();
}

void Slider::setDefaultFrameColor(CColor color)
{
  frameColor = color;
  setFrameColor(frameColor);
}

void Slider::setHighlightColor(CColor color) { highlightColor = color; }

void Slider::setDefaultFrameWidth(float width)
{
  frameWidth = width;
  setFrameWidth(frameWidth);
}

void Slider::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace VSTGUI
