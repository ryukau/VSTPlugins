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
  pContext->drawString("UltraSynth " VERSION_STR, CPoint(int(sc * 20), int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2022 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 60)));

  std::string leftText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max

- XY Pad -
Ctrl + Left Click|Reset to Default
Middle Drag|Only Change X
Mouse Wheel|Fine Adjustment X
Shift + Middle Drag|Only Change Y
Shift + Mouse Wheel|Fine Adjustment Y

Have a nice day!)";

  const float top0 = int(sc * 100);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 115);
  drawTextBlock(pContext, int(sc * 20), top0, lineHeight, blockWidth, leftText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
