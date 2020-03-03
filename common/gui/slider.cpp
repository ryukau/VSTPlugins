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

#include "slider.hpp"

namespace VSTGUI {

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
