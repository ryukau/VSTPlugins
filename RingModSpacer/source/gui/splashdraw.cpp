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
  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("RingModSpacer " VERSION_STR, CPoint(int(sc * 20), int(sc * 30)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 50)));

  std::string leftText = R"(This plugin mixes input and side chain signal
with distortion, while keeping peak amplitude
under 0 dB. To remove side chain signal, set
Side Mix to 0.)";

  std::string rightText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

Have a nice day!)";

  const float lineHeight = int(sc * 20);
  drawTextBlock(
    pContext, int(sc * 20), int(sc * 90), lineHeight, int(sc * 200), leftText);
  drawTextBlock(
    pContext, int(sc * 280), int(sc * 30), lineHeight, int(sc * 140), rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
