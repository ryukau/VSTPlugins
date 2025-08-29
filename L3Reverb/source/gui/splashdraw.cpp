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
  pContext->drawString("L3Reverb " VERSION_STR, CPoint(int(sc * 20), int(sc * 50)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2020 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 90)));

  std::string barboxTextLeft = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Shift + Left Drag|Skip Between Frames
Middle Drag|Draw Line
A|Alternate Sign
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert
Shift + I|Full Invert
N|Normalize (Preserve Min)
Shift + N|Normalize
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Random Walk
Shift + T|Random Walk to 0)";

  std::string barboxTextRight = R"(Z|Undo
Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1|Decrease
2-4|Decrease 2n-4n
5-9|Hold 2n-5n

- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor)";

  std::string miscText = R"(Changing InnerFeed or OuterFeed may
outputs loud signal.

Use Panic! button in case of blow up.

Have a nice day!)";

  const float top0 = int(sc * 140);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 160);
  drawTextBlock(pContext, int(sc * 20), top0, lineHeight, blockWidth, barboxTextLeft);
  drawTextBlock(pContext, int(sc * 390), top0, lineHeight, blockWidth, barboxTextRight);
  drawTextBlock(pContext, int(sc * 760), top0, lineHeight, blockWidth, miscText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
