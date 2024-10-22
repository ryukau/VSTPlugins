// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

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
  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("SpectralPhaser " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2024 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string leftText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor)";

  std::string rightText = R"(Larger Frame Size may cause stuttering.

To use as a vocoder,
+ Turn on Side Chain
+ Turn on Latency
+ Set Transform to FFT
+ Set Frame Size to 2048

Have a nice day!)";

  const float top0 = 100.0f;
  const float lineHeight = 20.0f;
  const float blockWidth = 120.0f;
  drawTextBlock(pContext, 20.0f, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, 290.0f, 40.0f, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
