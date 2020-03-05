// (c) 2019-2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "splash.hpp"
#include "../version.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

void SplashLabelTpz::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setFont(fontID);
  pContext->setFontColor(fontColor);
  const auto textOffsetTop = 6.0;
  const auto textOffsetLeft = 3.0;
  pContext->drawString(
    txt, CRect(textOffsetLeft, textOffsetTop, width, height), kCenterText, true);

  const double borderWidth = isMouseEntered ? highlightFrameWidth : frameWidth;
  const double halfBorderWidth = int(borderWidth / 2.0);
  pContext->setFrameColor(isMouseEntered ? highlightColor : frameColor);
  pContext->setLineWidth(borderWidth);
  pContext->drawLine(CPoint(25.0, borderWidth), CPoint(width - 25.0, borderWidth));
  pContext->drawLine(
    CPoint(borderWidth, height - borderWidth),
    CPoint(width - borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(25.0, borderWidth), CPoint(borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(width - 25.0, borderWidth), CPoint(width - borderWidth, height - borderWidth));

  setDirty(false);
}

CMouseEventResult SplashLabelTpz::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  splashView->setVisible(true);
  return kMouseEventHandled;
}

CMouseEventResult
SplashLabelTpz::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult
SplashLabelTpz::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult SplashLabelTpz::onMouseCancel()
{
  isMouseEntered = false;
  return kMouseEventHandled;
}

void SplashLabelTpz::setDefaultFrameColor(CColor color) { frameColor = color; }
void SplashLabelTpz::setHighlightColor(CColor color) { highlightColor = color; }
void SplashLabelTpz::setDefaultFrameWidth(float width) { frameWidth = width; }
void SplashLabelTpz::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace Vst
} // namespace Steinberg
