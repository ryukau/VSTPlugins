// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "knob.hpp"

namespace VSTGUI {

CPoint Knob::mapValueToSlit(double normalized, double length)
{
  auto radian
    = (2.0 * normalized - 1.0) * Constants::pi * (180.0 - slitNotchHalf) / 180.0;
  return CPoint(-sin(radian) * length, cos(radian) * length);
}

void Knob::draw(CDrawContext *pContext)
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
  pContext->drawArc(CRect(halfSlitWidth - radius, halfSlitWidth - radius,
                      radius - halfSlitWidth, radius - halfSlitWidth),
    (float)(90.0 + slitNotchHalf), (float)(90.0 - slitNotchHalf));

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
  pContext->drawEllipse(CRect(tip.x - halfSlitWidth, tip.y - halfSlitWidth,
                          tip.x + halfSlitWidth, tip.y + halfSlitWidth),
    kDrawFilled);

  setDirty(false);
}

CMouseEventResult Knob::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult Knob::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult Knob::onMouseDown(CPoint &where, const CButtonState &buttons)
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

CMouseEventResult Knob::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isMouseDown) endEdit();
  isMouseDown = false;
  return kMouseEventHandled;
}

CMouseEventResult Knob::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  if (!isMouseDown) return kMouseEventNotHandled;

  auto sensi = (buttons & kShift) ? lowSensitivity : sensitivity;
  value += (float)((anchorPoint.y - where.y) * sensi);
  bounceValue();

  if (value != getOldValue()) valueChanged();
  if (isDirty()) invalid();

  anchorPoint = where;
  return kMouseEventHandled;
}

CMouseEventResult Knob::onMouseCancel()
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

void Knob::setSlitWidth(double width) { halfSlitWidth = width / 2.0; }
void Knob::setDefaultTickLength(double length) { defaultTickLength = length; }
void Knob::setBackgroundColor(CColor color) { backgroundColor = color; }
void Knob::setSlitColor(CColor color) { slitColor = color; }
void Knob::setHighlightColor(CColor color) { highlightColor = color; }
void Knob::setTipColor(CColor color) { tipColor = color; }

} // namespace VSTGUI
