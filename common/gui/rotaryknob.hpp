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

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;
    beginEdit();
    isMouseDown = true;
    anchorPoint = event.mousePosition;
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isMouseDown) endEdit();
    isMouseDown = false;
    event.consumed = true;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    if (!isMouseDown) return;

    auto sensi = event.modifiers.is(ModifierKey::Shift) ? lowSensitivity : sensitivity;
    value += (float)((anchorPoint.y - event.mousePosition.y) * sensi);
    value = value > 1.0 || value < 0.0 ? value - floor(value) : value;
    bounceValue();

    if (value != getOldValue()) valueChanged();
    if (isDirty()) invalid();

    anchorPoint = event.mousePosition;
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
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
    event.consumed = true;
  }

  void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    if (isEditing() || event.deltaY == 0) return;
    beginEdit();
    value += event.deltaY * float(sensitivity) * 0.5f;
    value -= floor(value);
    bounceValue();
    valueChanged();
    endEdit();
    invalid();
    event.consumed = true;
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
