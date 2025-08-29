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
  const auto left0 = int(sc * 20);

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
  pContext->drawString("GrowlSynth " VERSION_STR, CPoint(left0, int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(left0, int(sc * 60)));

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

  const float top0 = int(sc * 100);
  const float lineHeight = left0;
  const float blockWidth = int(sc * 115);
  drawTextBlock(pContext, left0, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, int(sc * 320), top0, lineHeight, blockWidth, midText);
  drawTextBlock(pContext, int(sc * 620), top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
