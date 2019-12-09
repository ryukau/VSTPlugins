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

class Knob : public CControl {
public:
  Knob(const CRect &size, IControlListener *listener, int32_t tag)
    : CControl(size, listener, tag)
  {
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(Knob, CControl);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setSlitWidth(double width);
  void setDefaultTickLength(double length);
  void setBackgroundColor(CColor color);
  void setSlitColor(CColor color);
  void setHighlightColor(CColor color);
  void setTipColor(CColor color);

protected:
  CColor backgroundColor = CColor(255, 255, 255, 255);
  CColor highlightColor = CColor(0x33, 0xee, 0xee, 0xff);
  CColor slitColor = CColor(0xdd, 0xdd, 0xdd, 0xff);
  CColor tipColor = CColor(0, 0, 0, 255);

  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  double halfSlitWidth = 4.0;
  const double slitNotchHalf = 30.0; // Degree.
  double defaultTickLength = 0.5;

  const double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  const double lowSensitivity = sensitivity / 5.0;

  CPoint anchorPoint{0.0, 0.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  CPoint mapValueToSlit(double normalized, double length);
};

} // namespace VSTGUI
