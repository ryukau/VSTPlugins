// (c) 2019-2020 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

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
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("IterativeSinCluster " VERSION_STR, CPoint(20.0, 50.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2019-2020 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0, 90.0));

  pContext->drawString("- Knob -", CPoint(20.0, 150.0));
  pContext->drawString("Shift + Left Drag: Fine Adjustment", CPoint(20.0, 180.0));
  pContext->drawString("Ctrl + Left Click: Reset to Default", CPoint(20.0, 210.0));

  pContext->drawString("- Number -", CPoint(20.0, 270.0));
  pContext->drawString("Shares same controls with knob, and:", CPoint(20.0, 300.0));
  pContext->drawString("Middle Click: Flip Minimum and Maximum", CPoint(20.0, 330.0));

  pContext->drawString("- Overtone -", CPoint(380.0, 150.0));
  pContext->drawString("Ctrl + Left Drag: Reset to Default", CPoint(380.0, 180.0));
  pContext->drawString("Middle Drag: Draw Line", CPoint(380.0, 210.0));

  pContext->drawString("Have a nice day!", CPoint(380.0, 300.0));

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
