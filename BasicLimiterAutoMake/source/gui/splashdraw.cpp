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

  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("BasicLimiterAutoMake", CPoint(int(sc * 20), int(sc * 30)));
  pContext->drawString("  " VERSION_STR, CPoint(int(sc * 20), int(sc * 50)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2022 Takamitsu Endo", CPoint(int(sc * 20), int(sc * 70)));
  pContext->drawString("  (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 90)));

  pContext->drawString(
    "- Shift + Left Drag: Fine Adjustment", CPoint(int(sc * 20), int(sc * 120)));
  pContext->drawString(
    "- Ctrl + Left Click: Reset to Default", CPoint(int(sc * 20), int(sc * 140)));
  pContext->drawString(
    "- Middle Click: Toggle Min/Mid/Max", CPoint(int(sc * 20), int(sc * 160)));
  pContext->drawString(
    "- Shift + Middle Click: Take Floor", CPoint(int(sc * 20), int(sc * 180)));

  pContext->drawString(
    "If Overshoot is greater than 0 dB,", CPoint(int(sc * 20), int(sc * 210)));
  pContext->drawString(
    "lower Threshold to avoid clipping.", CPoint(int(sc * 20), int(sc * 230)));
  pContext->drawString(
    "It shows max sample peak over 0 dB.", CPoint(int(sc * 20), int(sc * 250)));

  pContext->drawString(
    "Sidechain disables Auto Make Up.", CPoint(int(sc * 20), int(sc * 280)));

  pContext->drawString(
    "Set Auto Make Up to -6.1 dB or lower", CPoint(int(sc * 20), int(sc * 310)));
  pContext->drawString(
    "when comparing L-R and M-S.", CPoint(int(sc * 20), int(sc * 330)));

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
