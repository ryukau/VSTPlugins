// (c) 2022 Takamitsu Endo
//
// This file is part of BasicLimiterAutoMake.
//
// BasicLimiterAutoMake is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiterAutoMake is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiterAutoMake.  If not, see <https://www.gnu.org/licenses/>.

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
  const double borderWidth = 2.0;
  const double halfBorderWidth = borderWidth / 2.0;

  // Background.
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("BasicLimiterAutoMake", CPoint(20.0, 30.0));
  pContext->drawString("  " VERSION_STR, CPoint(20.0, 50.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2022 Takamitsu Endo", CPoint(20.0, 70.0));
  pContext->drawString("  (ryukau@gmail.com)", CPoint(20.0, 90.0));

  pContext->drawString("- Shift + Left Drag: Fine Adjustment", CPoint(20.0f, 120.0f));
  pContext->drawString("- Ctrl + Left Click: Reset to Default", CPoint(20.0f, 140.0f));
  pContext->drawString("- Middle Click: Toggle Min/Mid/Max", CPoint(20.0f, 160.0f));
  pContext->drawString("- Shift + Middle Click: Take Floor", CPoint(20.0f, 180.0f));

  pContext->drawString("If Overshoot is greater than 0 dB,", CPoint(20.0f, 210.0f));
  pContext->drawString("lower Threshold to avoid clipping.", CPoint(20.0f, 230.0f));
  pContext->drawString("It shows max sample peak over 0 dB.", CPoint(20.0f, 250.0f));

  pContext->drawString("Sidechain disables Auto Make Up.", CPoint(20.0f, 280.0f));

  pContext->drawString("Set Auto Make Up to -6.1 dB or lower", CPoint(20.0f, 310.0f));
  pContext->drawString("when comparing L-R and M-S.", CPoint(20.0f, 330.0f));

  // Border.
  pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
