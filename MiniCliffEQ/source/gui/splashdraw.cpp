// (c) 2022 Takamitsu Endo
//
// This file is part of MiniCliffEQ.
//
// MiniCliffEQ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MiniCliffEQ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MiniCliffEQ.  If not, see <https://www.gnu.org/licenses/>.

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

  // Border.
  pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

  // Text.
  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("MiniCliffEQ", CPoint(20.0, 40.0));
  pContext->drawString("  " VERSION_STR, CPoint(20.0, 60.0));
  pContext->drawString("© 2022 Takamitsu Endo", CPoint(20.0f, 80.0f));
  pContext->drawString("  (ryukau@gmail.com)", CPoint(20.0f, 100.0f));

  pContext->drawString("Press Refresh FIR button", CPoint(20.0f, 130.0f));
  pContext->drawString("to apply Cutoff value.", CPoint(20.0f, 150.0f));

  pContext->drawString("Beware the massive latency.", CPoint(20.0f, 180.0f));

  pContext->drawString("Have a nice day!", CPoint(20.0f, 210.0f));

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
