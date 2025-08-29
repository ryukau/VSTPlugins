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
  pContext->drawString("GenericDrum " VERSION_STR, CPoint(int(sc * 20), int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 60)));

  std::string textColumn0 = R"(- Number Sliders -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

- BarBox -
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

Refer to the manual for a full list of shortcuts.)";

  std::string textColumn1 = R"(GenericDrum can output very loud signal.
Recommend to use with limiter.

There are 3 places to cause oscillation or blow up:

- Wire-Membrane collision.
- Membrane-Membrane collision.
- Membranes.

For wire-membrane collision, a solution is to turn
on Prevent Blow Up. Note that it also changes the
sound quite a bit.

On collisions, try raising Collision Distance to
prevent blow up.

On membranes, high pitch and high Q may cause blow
up. Watch out for following parameters:

- Note -> Pitch
- Cross Feedback Gain
- Cross Feedback Ratio
- Delay
- BP Q

Pitch envelope may cause pop noise when at least
one of Attack or Decay is less than 0.01.

If Note -> Pitch is not 0, and Slide Time is too
short, it may cause pop noise.)";

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
