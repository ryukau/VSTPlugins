// (c) 2019 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

namespace VSTGUI {

// Incremental encoder.
class RotaryKnob : public CControl {
public:
  RotaryKnob(const CRect &size, IControlListener *listener, int32_t tag)
    : CControl(size, listener, tag)
  {
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(RotaryKnob, CControl);

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

CPoint RotaryKnob::mapValueToSlit(double normalized, double length)
{
  auto radian = 2.0 * normalized * Constants::pi;
  return CPoint(-sin(radian) * length, cos(radian) * length);
}

void RotaryKnob::draw(CDrawContext *pContext)
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
  pContext->setFillColor(backgroundColor);
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  // Slit.
  auto radius = center.x > center.y ? center.y : center.x;
  pContext->setFrameColor(isMouseEntered ? highlightColor : slitColor);
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
    mapValueToSlit(defaultValue / getRange(), tipLength * defaultTickLength),
    mapValueToSlit(defaultValue / getRange(), tipLength));

  // Line from center to tip.
  auto tip = mapValueToSlit(getValueNormalized(), tipLength);
  pContext->setFrameColor(tipColor);
  pContext->drawLine(CPoint(0.0, 0.0), tip);

  // Tip.
  pContext->setFillColor(tipColor);
  pContext->drawEllipse(
    CRect(
      tip.x - halfSlitWidth, tip.y - halfSlitWidth, tip.x + halfSlitWidth,
      tip.y + halfSlitWidth),
    kDrawFilled);

  setDirty(false);
}

CMouseEventResult RotaryKnob::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult RotaryKnob::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult RotaryKnob::onMouseDown(CPoint &where, const CButtonState &buttons)
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

CMouseEventResult RotaryKnob::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isMouseDown) endEdit();
  isMouseDown = false;
  return kMouseEventHandled;
}

CMouseEventResult RotaryKnob::onMouseMoved(CPoint &where, const CButtonState &buttons)
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

CMouseEventResult RotaryKnob::onMouseCancel()
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

void RotaryKnob::setSlitWidth(double width) { halfSlitWidth = width / 2.0; }
void RotaryKnob::setDefaultTickLength(double length) { defaultTickLength = length; }
void RotaryKnob::setBackgroundColor(CColor color) { backgroundColor = color; }
void RotaryKnob::setSlitColor(CColor color) { slitColor = color; }
void RotaryKnob::setHighlightColor(CColor color) { highlightColor = color; }
void RotaryKnob::setTipColor(CColor color) { tipColor = color; }

} // namespace VSTGUI
