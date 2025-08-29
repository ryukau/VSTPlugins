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
  pContext->drawString("ParallelComb " VERSION_STR, CPoint(int(sc * 20), int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2022 Takamitsu Endo (ryukau@gmail.com)", CPoint(int(sc * 20), int(sc * 60)));

  std::string leftText = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Middle Drag|Draw Line
Shift + D|Toggle Min/Mid/Max
I|Invert
N|Normalize
R|Randomize
T|Random Walk
Z|Undo
, (Comma) / . (Period)|Rotate Back/Forward
1|Decrease

And more! Refer to the manual for full list
of shortcuts.)";

  std::string rightText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max

To stop the output, set Feedback to 0 or
press Panic! button.

To reliably stop the output when Feedback
is non 0, insert a gate before ParallelComb,
so that input signal is all set to 0. Even
dithering noise can saturate feedback.

Have a nice day!)";

  const float top0 = int(sc * 100);
  const float lineHeight = int(sc * 20);
  const float blockWidth = int(sc * 115);
  drawTextBlock(pContext, int(sc * 20), top0, lineHeight, blockWidth, leftText);
  drawTextBlock(
    pContext, int(sc * 20) + int(sc * 240), top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
