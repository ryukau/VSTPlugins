// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "button.hpp"

namespace VSTGUI {

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

CMouseEventResult KickButton::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isPressed = false;
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isLeftButton()) return kMouseEventNotHandled;
  isPressed = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isPressed) {
    value = 1;
    valueChanged();
    value = 0;

    isPressed = false;
    invalid();
  }
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseCancel()
{
  value = 0;
  isPressed = false;
  isMouseEntered = false;
  return kMouseEventHandled;
}

CMouseEventResult
MessageButton::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isPressed = false;
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isLeftButton()) return kMouseEventNotHandled;

  isPressed = true;
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isPressed) {
    controller->sendTextMessage(messageID.c_str());
    isPressed = false;
  }
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseCancel()
{
  isPressed = false;
  isMouseEntered = false;
  return kMouseEventHandled;
}

} // namespace VSTGUI
