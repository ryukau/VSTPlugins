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
  pContext->drawString("GrowlSynth " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string leftText = R"(This plugin is beta version.
Breaking changes may be introduced.

- Do not use for production.
- Do not save your project with this plugin.
- Always render or record the result to audio file.

Click to dismiss this message.)";

  std::string midText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

GrowlSynth can output very loud signal.
Recommend to use with limiter.

Breath.Gain and Pulse.Gain are fed into non-linear
component. Those two affect the character of sound.
To change loudness without affecting the character,
use Mix.Output.

Have a nice day!)";

  std::string rightText = R"()";

  const float top0 = 100.0f;
  const float lineHeight = 20.0f;
  const float blockWidth = 115.0f;
  drawTextBlock(pContext, 20.0f, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, 320.0f, top0, lineHeight, blockWidth, midText);
  drawTextBlock(pContext, 620.0f, top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
