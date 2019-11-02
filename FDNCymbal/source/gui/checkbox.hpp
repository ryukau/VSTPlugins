// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

namespace VSTGUI {

class CheckBox : public CCheckBox {
public:
  CheckBox(
    const CRect &size,
    IControlListener *listener = nullptr,
    int32_t tag = -1,
    UTF8StringPtr title = nullptr,
    CBitmap *bitmap = nullptr,
    int32_t style = 0)
    : CCheckBox(size, listener, tag, title, bitmap, style)
  {
  }

  CLASS_METHODS(CheckBox, CCheckBox);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameWidth(float width);
  void setHighlightWidth(float width);

protected:
  float frameWidth = 1.0f;
  float highlightFrameWidth = 2.0f;
};

CMouseEventResult CheckBox::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(highlightFrameWidth);
  return kMouseEventHandled;
}

CMouseEventResult CheckBox::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  setFrameWidth(frameWidth);
  return kMouseEventHandled;
}

CMouseEventResult CheckBox::onMouseCancel()
{
  setFrameWidth(frameWidth);
  return CCheckBox::onMouseCancel();
}

void CheckBox::setDefaultFrameWidth(float width)
{
  frameWidth = width;
  setFrameWidth(frameWidth);
}

void CheckBox::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace VSTGUI
