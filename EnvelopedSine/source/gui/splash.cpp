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
  pContext->drawString(conv("EnvelopedSine " VERSION_STR), CPoint(20.0, 50.0));

  pContext->setFont(fontIDText);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    conv("© 2019-2020 Takamitsu Endo (ryukau@gmail.com)"), CPoint(20.0, 90.0));

  pContext->drawString(conv("- Knob -"), CPoint(20.0f, 150.0f));
  pContext->drawString(conv("Shift + Left Drag: Fine Adjustment"), CPoint(20.0f, 180.0f));
  pContext->drawString(
    conv("Ctrl + Left Click: Reset to Default"), CPoint(20.0f, 210.0f));

  pContext->drawString(conv("- Number -"), CPoint(20.0f, 270.0f));
  pContext->drawString(
    conv("Shares same controls with knob, and:"), CPoint(20.0f, 300.0f));
  pContext->drawString(
    conv("Right Click: Flip Minimum and Maximum"), CPoint(20.0f, 330.0f));

  pContext->drawString(conv("- Overtone -"), CPoint(280.0f, 150.0f));

  pContext->drawString(conv("Ctrl + Left Click"), CPoint(280.0f, 180.0f));
  pContext->drawString(conv("Right Drag"), CPoint(280.0f, 210.0f));
  pContext->drawString(conv("A"), CPoint(280.0f, 240.0f));
  pContext->drawString(conv("D"), CPoint(280.0f, 270.0f));
  pContext->drawString(conv("F"), CPoint(280.0f, 300.0f));
  pContext->drawString(conv("Shift + F"), CPoint(280.0f, 330.0f));
  pContext->drawString(conv("I"), CPoint(280.0f, 360.0f));
  pContext->drawString(conv("Shift + I"), CPoint(280.0f, 390.0f));
  pContext->drawString(conv("N"), CPoint(280.0f, 420.0f));
  pContext->drawString(conv("Shift + N"), CPoint(280.0f, 450.0f));

  pContext->drawString(conv("| Reset to Default"), CPoint(380.0f, 180.0f));
  pContext->drawString(conv("| Draw Line"), CPoint(380.0f, 210.0f));
  pContext->drawString(conv("| Sort Ascending Order"), CPoint(380.0f, 240.0f));
  pContext->drawString(conv("| Sort Decending Order"), CPoint(380.0f, 270.0f));
  pContext->drawString(conv("| Low-pass Filter"), CPoint(380.0f, 300.0f));
  pContext->drawString(conv("| High-pass Filter"), CPoint(380.0f, 330.0f));
  pContext->drawString(conv("| Invert Value"), CPoint(380.0f, 360.0f));
  pContext->drawString(conv("| Invert Value (Minimum to 0)"), CPoint(380.0f, 390.0f));
  pContext->drawString(conv("| Normalize"), CPoint(380.0f, 420.0f));
  pContext->drawString(conv("| Normalize (Minimum to 0)"), CPoint(380.0f, 450.0f));

  pContext->drawString(conv("H"), CPoint(600.0f, 180.0f));
  pContext->drawString(conv("L"), CPoint(600.0f, 210.0f));
  pContext->drawString(conv("P"), CPoint(600.0f, 240.0f));
  pContext->drawString(conv("R"), CPoint(600.0f, 270.0f));
  pContext->drawString(conv("Shift + R"), CPoint(600.0f, 300.0f));
  pContext->drawString(conv("S"), CPoint(600.0f, 330.0f));
  pContext->drawString(conv(", (Comma)"), CPoint(600.0f, 360.0f));
  pContext->drawString(conv(". (Period)"), CPoint(600.0f, 390.0f));
  pContext->drawString(conv("1"), CPoint(600.0f, 420.0f));
  pContext->drawString(conv("2-9"), CPoint(600.0f, 450.0f));

  pContext->drawString(conv("| Emphasize High"), CPoint(680.0f, 180.0f));
  pContext->drawString(conv("| Emphasize Low"), CPoint(680.0f, 210.0f));
  pContext->drawString(conv("| Permute"), CPoint(680.0f, 240.0f));
  pContext->drawString(conv("| Randomize"), CPoint(680.0f, 270.0f));
  pContext->drawString(conv("| Sparse Randomize"), CPoint(680.0f, 300.0f));
  pContext->drawString(conv("| Subtle Randomize"), CPoint(680.0f, 330.0f));
  pContext->drawString(conv("| Rotate Back"), CPoint(680.0f, 360.0f));
  pContext->drawString(conv("| Rotate Forward"), CPoint(680.0f, 390.0f));
  pContext->drawString(conv("| Decrease Odd"), CPoint(680.0f, 420.0f));
  pContext->drawString(conv("| Decrease 2n-9n"), CPoint(680.0f, 450.0f));

  pContext->drawString(conv("Have a nice day!"), CPoint(740.0f, 510.0f));

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

inline IPlatformString *CreditView::conv(const char *text)
{
  return UTF8String(text).getPlatformString();
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
