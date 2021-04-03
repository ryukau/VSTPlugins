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

#include "style.hpp"

namespace VSTGUI {

// Incremental encoder.
template<Uhhyou::Style style = Uhhyou::Style::common> class RotaryKnob : public CControl {
public:
  RotaryKnob(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : CControl(size, listener, tag), pal(palette)
  {
  }

  CLASS_METHODS(RotaryKnob, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();
    const auto center = CPoint(width / 2.0, height / 2.0);

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft() + center));

    // Background.
    const double borderWidth = 2.0;
    pContext->setLineWidth(borderWidth);
    pContext->setFillColor(pal.background());
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

    // Slit.
    auto radius = center.x > center.y ? center.y : center.x;
    if constexpr (style == Uhhyou::Style::accent) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightAccent() : pal.unfocused());
    } else if (style == Uhhyou::Style::warning) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightWarning() : pal.unfocused());
    } else {
      pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.unfocused());
    }
    pContext->setLineStyle(lineStyle);
    pContext->setLineWidth(halfSlitWidth * 2.0);
    pContext->drawEllipse(
      CRect(
        halfSlitWidth - center.x, halfSlitWidth - center.y, center.x - halfSlitWidth,
        center.y - halfSlitWidth),
      kDrawStroked);

    // Tick for default value. Sharing color and style with slit.
    auto tipLength = halfSlitWidth - radius;
    pContext->setLineWidth(halfSlitWidth / 2.0);
    pContext->drawLine(
      mapValueToSlit(getDefaultValue() / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(getDefaultValue() / getRange(), tipLength));

    // Line from center to tip.
    auto tip = mapValueToSlit(getValueNormalized(), tipLength);
    pContext->setFrameColor(pal.foreground());
    pContext->drawLine(CPoint(0.0, 0.0), tip);

    // Tip.
    pContext->setFillColor(pal.foreground());
    pContext->drawEllipse(
      CRect(
        tip.x - halfSlitWidth, tip.y - halfSlitWidth, tip.x + halfSlitWidth,
        tip.y + halfSlitWidth),
      kDrawFilled);

    setDirty(false);
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (!buttons.isLeftButton()) return kMouseEventNotHandled;

    beginEdit();
    if (checkDefaultValue(buttons)) {
      endEdit();
      return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    isMouseDown = true;
    anchorPoint = where;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (isMouseDown) endEdit();
    isMouseDown = false;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    if (!isMouseDown) return kMouseEventNotHandled;

    auto sensi = (buttons & kShift) ? lowSensitivity : sensitivity;
    value += (float)((anchorPoint.y - where.y) * sensi);
    value = value > 1.0 || value < 0.0 ? value - floor(value) : value;
    bounceValue();

    if (value != getOldValue()) valueChanged();
    if (isDirty()) invalid();

    anchorPoint = where;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    if (isMouseDown) {
      if (isDirty()) {
        valueChanged();
        invalid();
      }
      endEdit();
    }
    isMouseDown = false;
    isMouseEntered = false;
    return kMouseEventHandled;
  }

  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override
  {
    if (isEditing() || axis != kMouseWheelAxisY || distance == 0.0f) return false;

    beginEdit();
    value += distance * float(sensitivity) * 0.5f;
    value -= floor(value);
    bounceValue();
    valueChanged();
    endEdit();
    invalid();
    return true;
  }

  void setSlitWidth(double width) { halfSlitWidth = width / 2.0; }
  void setDefaultTickLength(double length) { defaultTickLength = length; }

protected:
  CPoint mapValueToSlit(double normalized, double length)
  {
    auto radian = 2.0 * normalized * Constants::pi;
    return CPoint(-sin(radian) * length, cos(radian) * length);
  }

  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  double halfSlitWidth = 4.0;
  const double slitNotchHalf = 30.0; // Degree.
  double defaultTickLength = 0.5;

  const double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  const double lowSensitivity = sensitivity / 5.0;

  CPoint anchorPoint{0.0, 0.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  Uhhyou::Palette &pal;
};

} // namespace VSTGUI
