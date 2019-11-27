// (c) 2019 Takamitsu Endo
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
#include "guistyle.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

void CreditView::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();
  const double borderWidth = 8.0;

  // Background.
  const auto bgColor = CColor(255, 255, 255, 255);
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(bgColor);
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  const auto textLeft = 160.0;
  pContext->setFont(fontIDTitle);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    UTF8String("TrapezoidSynth " VERSION_STR).getPlatformString(),
    CPoint(textLeft, 50.0));

  pContext->setFont(fontIDText);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    UTF8String("© 2019 Takamitsu Endo (ryukau@gmail.com)").getPlatformString(),
    CPoint(textLeft, 90.0));

  pContext->drawString(
    UTF8String("Shift + Drag: Fine Adjustment").getPlatformString(),
    CPoint(textLeft, 150.0));
  pContext->drawString(
    UTF8String("Ctrl + Click: Reset to Default").getPlatformString(),
    CPoint(textLeft, 180.0));

  pContext->drawString(
    UTF8String("⏢ Have a nice day! ⏢").getPlatformString(), CPoint(textLeft, 240.0));

  // Border.
  const auto borderColor = CColor(0, 0, 0, 255);
  pContext->setFrameColor(borderColor);
  pContext->drawLine(CPoint(100.0, borderWidth), CPoint(width - 100.0, borderWidth));
  pContext->drawLine(
    CPoint(borderWidth, height - borderWidth),
    CPoint(width - borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(100.0, borderWidth), CPoint(borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(width - 100.0, borderWidth),
    CPoint(width - borderWidth, height - borderWidth));

  setDirty(false);
}

void SplashLabel::draw(CDrawContext *pContext)
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

CMouseEventResult SplashLabel::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult SplashLabel::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult SplashLabel::onMouseCancel()
{
  isMouseEntered = false;
  return CSplashScreen::onMouseCancel();
}

void SplashLabel::setDefaultFrameColor(CColor color) { frameColor = color; }
void SplashLabel::setHighlightColor(CColor color) { highlightColor = color; }
void SplashLabel::setDefaultFrameWidth(float width) { frameWidth = width; }
void SplashLabel::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace Vst
} // namespace Steinberg
