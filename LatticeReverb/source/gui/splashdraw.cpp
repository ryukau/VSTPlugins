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
  pContext->drawString("LatticeReverb " VERSION_STR, CPoint(int(sc * 20), int(sc * 50)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2020 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 90)));

  std::string leftText = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Middle Drag|Draw Line
Shift + D|Toggle Min/Mid/Max
I|Invert Value
P|Permute
R|Randomize
S|Sort Decending Order
T|Random Walk
Shift + T|Random Walk to 0
Z|Undo
Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1-4|Decrease 1n-4n
5-9|Hold 2n-5n

And more! Refer to the manual for full list
of shortcuts.)";

  std::string rightText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

Changing InnerFeed or OuterFeed may
outputs loud signal.

Use Panic! button in case of blow up.

Have a nice day!)";

  const float top0 = int(sc * 140);
  const float mid = int(sc * 355);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 160);
  drawTextBlock(pContext, int(sc * 20), top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, mid, top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
