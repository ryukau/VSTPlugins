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
  pContext->drawString("FeedbackPhaser " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string topText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor)";

  std::string bottomText
    = R"(When feedback is too much, output amplitude reaches the value of
Safety Clip on top of input amplitude.

All the parameters on Feedback and Modulation section may blow up
the feedback. Especially take care of Feedback->Amount, Safety Clip,
and Modulation->Amount.

Changing any parameter resets "Too much feedback" message.

Have a nice day!)";

  const float top0 = 100.0f;
  const float left0 = 20.0f;
  const float lineHeight = 20.0f;
  const float blockWidth = 160.0f;
  drawTextBlock(pContext, left0, top0, lineHeight, blockWidth, topText);
  drawTextBlock(
    pContext, left0, top0 + 6 * lineHeight, lineHeight, blockWidth, bottomText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
