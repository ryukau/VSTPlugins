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

#include "vstgui/vstgui.h"

namespace VSTGUI {

class TextButton : public CTextButton {
public:
  TextButton(
    const CRect &size,
    IControlListener *listener = nullptr,
    int32_t tag = -1,
    UTF8StringPtr title = nullptr,
    Style style = kKickStyle)
    : CTextButton(size, listener, tag, title, style)
  {
    setFrameColor(frameColor);
  }

  CLASS_METHODS(TextButton, CTextButton);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameColor(CColor color);
  void setHighlightColor(CColor color);
  void setDefaultFrameWidth(float width);
  void setHighlightWidth(float width);

protected:
  CColor frameColor = CColor(0, 0, 0, 255);
  CColor highlightColor = CColor(0, 0, 0, 255);
  float frameWidth = 1.0f;
  float highlightFrameWidth = 2.0f;
};

CMouseEventResult TextButton::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(highlightFrameWidth);
  setFrameColor(highlightColor);
  return kMouseEventHandled;
}

CMouseEventResult TextButton::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(frameWidth);
  setFrameColor(frameColor);
  return kMouseEventHandled;
}

CMouseEventResult TextButton::onMouseCancel()
{
  setFrameWidth(frameWidth);
  setFrameColor(frameColor);
  return CTextButton::onMouseCancel();
}

void TextButton::setDefaultFrameColor(CColor color)
{
  frameColor = color;
  setFrameColor(frameColor);
}

void TextButton::setHighlightColor(CColor color) { highlightColor = color; }

void TextButton::setDefaultFrameWidth(float width)
{
  frameWidth = width;
  setFrameWidth(frameWidth);
}

void TextButton::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace VSTGUI
