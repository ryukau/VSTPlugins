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
  pContext->drawString(
    "DoubleLoopCymbal " VERSION_STR, CPoint(int(sc * 20), int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2024 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 60)));

  std::string textColumn0 = R"(- Number Sliders -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

- Tip -
At first, try focus on following parameters.
+ Transpose
+ Seed
+ Highpass in middle column (3 of them)
+ Pitch Ratio
+ Feed 2)";

  std::string textColumn1 = R"(Too much Modulation may cause blow up.

If Feed 1 or Feed 2 are exactly at +1 or -1,
amplitude may slowly rise.

To use External Input, note events are required
to open gate.

To reduce CPU load, uncheck `2x Sampling`.

To get consistent sound, check `Fixed Noise`.

`Release` disables Closing Noise section.)";

  const float top0 = int(sc * 100);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 115);
  drawTextBlock(pContext, int(sc * 20), top0, lineHeight, blockWidth, textColumn0);
  drawTextBlock(pContext, int(sc * 320), top0, lineHeight, blockWidth, textColumn1);
  drawTextBlock(
    pContext, int(sc * 620), top0, lineHeight, blockWidth, "Have a nice day!");

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
