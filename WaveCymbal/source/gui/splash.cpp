// (c) 2019 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

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
  const double halfBorderWidth = borderWidth / 2.0;

  // Background.
  const auto bgColor = CColor(255, 255, 255, 255);
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(bgColor);
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  pContext->setFont(fontIDTitle);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    UTF8String("WaveCymbal " VERSION_STR).getPlatformString(), CPoint(20.0, 50.0));

  pContext->setFont(fontIDText);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    UTF8String("© 2019 Takamitsu Endo (ryukau@gmail.com)").getPlatformString(),
    CPoint(20.0, 90.0));

  pContext->drawString(
    UTF8String("Shift + Drag: Fine Adjustment").getPlatformString(), CPoint(20.0, 150.0));
  pContext->drawString(
    UTF8String("Ctrl + Click: Reset to Default").getPlatformString(),
    CPoint(20.0, 180.0));

  pContext->drawString(
    UTF8String("Have a nice day! 🍻").getPlatformString(), CPoint(20.0, 240.0));

  // Border.
  const auto borderColor = CColor(0, 0, 0, 255);
  pContext->setFrameColor(borderColor);
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

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
  pContext->drawString(txt, CRect(0.0, 0.0, width, height), kCenterText, true);

  const double borderWidth = isMouseEntered ? highlightFrameWidth : frameWidth;
  const double halfBorderWidth = int(borderWidth / 2.0);
  pContext->setFrameColor(isMouseEntered ? highlightColor : frameColor);
  pContext->setLineWidth(borderWidth);
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

  setDirty(false);
}

CMouseEventResult SplashLabel::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  splashView->setVisible(true);
  return kMouseEventHandled;
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
  return kMouseEventHandled;
}

void SplashLabel::setDefaultFrameColor(CColor color) { frameColor = color; }
void SplashLabel::setHighlightColor(CColor color) { highlightColor = color; }
void SplashLabel::setDefaultFrameWidth(float width) { frameWidth = width; }
void SplashLabel::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace Vst
} // namespace Steinberg
