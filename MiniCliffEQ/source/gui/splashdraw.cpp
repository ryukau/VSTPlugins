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
  const auto sc = pal.guiScale();
  const double borderWidth = int(sc * 2);
  const double halfBorderWidth = int(borderWidth / 2);

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
  pContext->drawString("MiniCliffEQ", CPoint(int(sc * 20), int(sc * 40)));
  pContext->drawString("  " VERSION_STR, CPoint(int(sc * 20), int(sc * 60)));
  pContext->drawString("© 2022 Takamitsu Endo", CPoint(int(sc * 20), int(sc * 80)));
  pContext->drawString("  (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 100)));

  pContext->drawString("Press Refresh FIR button", CPoint(int(sc * 20), int(sc * 130)));
  pContext->drawString("to apply Cutoff value.", CPoint(int(sc * 20), int(sc * 150)));

  pContext->drawString(
    "Beware the massive latency.", CPoint(int(sc * 20), int(sc * 180)));

  pContext->drawString("Have a nice day!", CPoint(int(sc * 20), int(sc * 210)));

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
