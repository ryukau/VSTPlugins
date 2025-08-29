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
  const double borderWidth = int(sc * 8);

  // Background.
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  const auto textLeft = int(sc * 160);
  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("TrapezoidSynth " VERSION_STR, CPoint(textLeft, int(sc * 50)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2019-2020 Takamitsu Endo (ryukau@gmail.com)", CPoint(textLeft, int(sc * 90)));

  pContext->drawString("Shift + Drag: Fine Adjustment", CPoint(textLeft, int(sc * 150)));
  pContext->drawString("Ctrl + Click: Reset to Default", CPoint(textLeft, int(sc * 180)));

  pContext->drawString("⏢ Have a nice day! ⏢", CPoint(textLeft, int(sc * 240)));

  // Border.
  const auto trapezoidAdjuster = int(sc * 100);
  pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
  pContext->setLineStyle(
    CLineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound});
  pContext->drawLine(
    CPoint(trapezoidAdjuster, borderWidth),
    CPoint(width - trapezoidAdjuster, borderWidth));
  pContext->drawLine(
    CPoint(borderWidth, height - borderWidth),
    CPoint(width - borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(trapezoidAdjuster, borderWidth), CPoint(borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(width - trapezoidAdjuster, borderWidth),
    CPoint(width - borderWidth, height - borderWidth));

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
