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
  pContext->drawString("GlitchSprinkler " VERSION_STR, CPoint(left0, int(sc * 40)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2024 Takamitsu Endo (ryukau@gmail.com)", CPoint(left0, int(sc * 60)));

  std::string textColumn0 = R"(- Number Sliders -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

- Waveform Editor -
Several preset waveforms are available.
Waveform slithers due to the algorithm used.

r|Randomize
1|Sine
2|Sine FM A
3|Sine FM B
4|Sawtooth
5|Triangle
6|Trapezoid
7|Alternating
8|Pulse
9|Saturated Sine
0|Zero

Refer to the manual for a full list of shortcuts.)";

  std::string textColumn1 = R"(To immediately stop the sound, turn off `Release`.

When `Decay to` is set to 0, and `Release` is on,
sound won't stop after note-off.

When `Pulse Width / Bit Mask` is 1.0, and `Pulse
Width Modulation` is on, output becomes silent.

When both of `Pulse Width Modulation` and
`Bitwise And` is on, aliasing may be noticeable.

When `Resonance` is near 1.0, `Polyphonic` is
turned off, and fast sequence is played,
amplitude may change for each note-on.

Maximum voice number is 256. If CPU load is too
high, lower `Decay to`.

This synthesizer is using integer pitch tuning.
Because of this, higher pitches are increasingly
out of tune. `Discrete *` tunings are the most
clear, but they aren't compatible with
traditional tunings.)";

  const float top0 = int(sc * 90);
  const float lineHeight = left0;
  const float blockWidth = int(sc * 115);
  drawTextBlock(pContext, left0, top0, lineHeight, blockWidth, textColumn0);
  drawTextBlock(pContext, int(sc * 320), top0, lineHeight, blockWidth, textColumn1);
  drawTextBlock(
    pContext, int(sc * 620), top0, lineHeight, blockWidth, "Have a nice day!");

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg
