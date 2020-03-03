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

#include "../../../common/gui/splash.hpp"
#include "../version.hpp"

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
    UTF8String("IterativeSinCluster " VERSION_STR).getPlatformString(),
    CPoint(20.0, 50.0));

  pContext->setFont(fontIDText);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  pContext->drawString(
    UTF8String("© 2019 Takamitsu Endo (ryukau@gmail.com)").getPlatformString(),
    CPoint(20.0, 90.0));

  pContext->drawString(UTF8String("- Knob -").getPlatformString(), CPoint(20.0, 150.0));
  pContext->drawString(
    UTF8String("Shift + Left Drag: Fine Adjustment").getPlatformString(),
    CPoint(20.0, 180.0));
  pContext->drawString(
    UTF8String("Ctrl + Left Click: Reset to Default").getPlatformString(),
    CPoint(20.0, 210.0));

  pContext->drawString(UTF8String("- Number -").getPlatformString(), CPoint(20.0, 270.0));
  pContext->drawString(
    UTF8String("Shares same controls with knob, and:").getPlatformString(),
    CPoint(20.0, 300.0));
  pContext->drawString(
    UTF8String("Middle Click: Flip Minimum and Maximum").getPlatformString(),
    CPoint(20.0, 330.0));

  pContext->drawString(
    UTF8String("- Overtone -").getPlatformString(), CPoint(380.0, 150.0));
  pContext->drawString(
    UTF8String("Ctrl + Left Drag: Reset to Default").getPlatformString(),
    CPoint(380.0, 180.0));
  pContext->drawString(
    UTF8String("Middle Drag: Draw Line").getPlatformString(), CPoint(380.0, 210.0));

  pContext->drawString(
    UTF8String("Have a nice day!").getPlatformString(), CPoint(380.0, 300.0));

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

} // namespace Vst
} // namespace Steinberg
