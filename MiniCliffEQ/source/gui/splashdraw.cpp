// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
