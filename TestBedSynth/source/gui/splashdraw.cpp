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

  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("TestBedSynth " VERSION_STR, CPoint(left0, int(sc * 50)));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(
    "© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(left0, int(sc * 90)));

  std::string leftText = R"(
This plugin is in active development.
Breaking changes will be introduced.

- Do not use for production.
- Do not save your project with this plugin.

Click to dismiss this message.

Have a nice day!)";

  std::string rightText = R"()";

  const float top0 = int(sc * 140);
  const float mid = int(sc * 400);
  const float lineHeight = left0;
  const float blockWidth = int(sc * 180);
  drawTextBlock(pContext, left0, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, mid, top0, lineHeight, blockWidth, rightText);

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
