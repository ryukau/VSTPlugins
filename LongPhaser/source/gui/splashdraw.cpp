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
  pContext->drawString("LongPhaser " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2022 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string leftText = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Shift + Left Drag|Skip Between Frames
Middle Drag|Draw Line
Ctrl + Shift + Middle Drag|Toggle Lock
Shift + D|Toggle Min/Mid/Max
F|Low-pass Filter
I|Invert
N|Normalize (Preserve Min)
P|Permute
R|Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Random Walk
Z|Undo)";

  std::string rightText = R"(Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1-4|Decrease 1n-4n
5-9|Hold 2n-5n

And more! Refer to the manual for full list
of shortcuts.

- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

Note event can be used to control delay time,
however it may add pop noise for each note-on
and note-off.

Have a nice day!)";

  const float top0 = 100.0f;
  const float lineHeight = 20.0f;
  const float blockWidth = 160.0f;
  drawTextBlock(pContext, 20.0f, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, 400.0f, 40.0f, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
