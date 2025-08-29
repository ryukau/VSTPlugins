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

  const float top0 = int(sc * 40);
  const float topTextBlock = int(sc * 120);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 180);
  const float mid = lineHeight + int(width / 2);
  const float left = mid - 2 * blockWidth;

  // Background.
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("ClangSynth " VERSION_STR, CPoint(left, top0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2021 Takamitsu Endo (ryukau@gmail.com)", CPoint(left, top0 + 2 * lineHeight));

  std::string leftText = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Ctrl + Shift + Left Drag|Skip Between Frames
Middle Drag|Draw Line
Ctrl + Shift + Middle Drag|Toggle Lock
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert
Shift + I|Full Invert
L|Toggle Lock
Shift + L|Toggle Lock for All
N|Normalize (Preserve Min)
Shift + N|Normalize
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Random Walk
Shift + T|Random Walk to 0
Z|Undo
Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1-4|Decrease 1n-4n
5-9|Hold 2n-5n)";

  std::string rightText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Toggle Min/Mid/Max
Shift + Middle Click|Take Floor


ClangSynth outputs peaky signal.
Recommend to use with limiter.


Have a nice day!)";

  drawTextBlock(pContext, left, topTextBlock, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, mid, topTextBlock, lineHeight, blockWidth, rightText);

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
