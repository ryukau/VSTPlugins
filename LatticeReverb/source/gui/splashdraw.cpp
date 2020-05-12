// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../common/gui/splash.hpp"
#include "../version.hpp"

#define TEXT(X, Y, STR) pContext->drawString(STR, CPoint(X, Y));

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
  const auto bgColor = CColor(255, 255, 255, 255);
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(bgColor);
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  // Border.
  const auto borderColor = CColor(0, 0, 0, 255);
  pContext->setFrameColor(borderColor);
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

  // Text.
  pContext->setFont(fontIDTitle);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  TEXT(20.0, 50.0, "LatticeReverb " VERSION_STR);

  pContext->setFont(fontIDText);
  pContext->setFontColor(CColor(0, 0, 0, 255));
  TEXT(20.0f, 90.0f, "© 2020 Takamitsu Endo (ryukau@gmail.com)");

  TEXT(20.0f, 140.0f, "- Knob -");
  TEXT(20.0f, 160.0f, "Shift + Left Drag");
  TEXT(180.0f, 160.0f, "Fine Adjustment");
  TEXT(20.0f, 180.0f, "Ctrl + Left Click");
  TEXT(180.0f, 180.0f, "Reset to Default");

  TEXT(20.0f, 220.0f, "- Number -");
  TEXT(20.0f, 240.0f, "Shares same controls with knob, and:");
  TEXT(20.0f, 260.0f, "Right Click");
  TEXT(180.0f, 260.0f, "Flip Min/Max");

  TEXT(20.0f, 300.0f, "- BarBox -");
  TEXT(20.0f, 320.0f, "Ctrl + Left Click");
  TEXT(180.0f, 320.0f, "Reset to Default");
  TEXT(20.0f, 340.0f, "Right Drag");
  TEXT(180.0f, 340.0f, "Draw Line");
  TEXT(20.0f, 360.0f, "Shift + D");
  TEXT(180.0f, 360.0f, "Toggle Min/Mid/Max");
  TEXT(20.0f, 380.0f, "I");
  TEXT(180.0f, 380.0f, "Invert Value");
  TEXT(20.0f, 400.0f, "R");
  TEXT(180.0f, 400.0f, "Randomize");
  TEXT(20.0f, 420.0f, "T");
  TEXT(180.0f, 420.0f, "Subtle Randomize");
  TEXT(20.0f, 440.0f, ", (Comma)");
  TEXT(180.0f, 440.0f, "Rotate Back");
  TEXT(20.0f, 460.0f, ". (Period)");
  TEXT(180.0f, 460.0f, "Rotate Forward");
  TEXT(20.0f, 480.0f, "1");
  TEXT(180.0f, 480.0f, "Decrease");
  TEXT(20.0f, 500.0f, "2-9");
  TEXT(180.0f, 500.0f, "Decrease 2n-9n");

  const float mid = 355;

  TEXT(mid, 140.0f, "Changing InnerFeed or OuterFeed may outputs");
  TEXT(mid, 160.0f, "loud signal.");

  TEXT(mid, 200.0f, "Use Panic! button in case of blow up.");

  TEXT(mid, 240.0f, "Have a nice day!");

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
