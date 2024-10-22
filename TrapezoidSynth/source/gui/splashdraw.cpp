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
  const double borderWidth = 8.0;

  // Background.
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  const auto textLeft = 160.0;
  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("TrapezoidSynth " VERSION_STR, CPoint(textLeft, 50.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2019-2020 Takamitsu Endo (ryukau@gmail.com)", CPoint(textLeft, 90.0));

  pContext->drawString("Shift + Drag: Fine Adjustment", CPoint(textLeft, 150.0));
  pContext->drawString("Ctrl + Click: Reset to Default", CPoint(textLeft, 180.0));

  pContext->drawString("⏢ Have a nice day! ⏢", CPoint(textLeft, 240.0));

  // Border.
  pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
  pContext->setLineStyle(
    CLineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound});
  pContext->drawLine(CPoint(100.0, borderWidth), CPoint(width - 100.0, borderWidth));
  pContext->drawLine(
    CPoint(borderWidth, height - borderWidth),
    CPoint(width - borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(100.0, borderWidth), CPoint(borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(width - 100.0, borderWidth),
    CPoint(width - borderWidth, height - borderWidth));

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
