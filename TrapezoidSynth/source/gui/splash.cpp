// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "splash.hpp"
#include "../version.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

void SplashLabelTpz::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setFont(fontId);
  pContext->setFontColor(pal.foreground());
  const auto textOffsetTop = 6.0;
  const auto textOffsetLeft = 3.0;
  pContext->drawString(
    label.c_str(), CRect(textOffsetLeft, textOffsetTop, width, height), kCenterText,
    true);

  const double borderWidth = isMouseEntered ? highlightFrameWidth : frameWidth;
  const double halfBorderWidth = int(borderWidth / 2.0);
  pContext->setFrameColor(isMouseEntered ? pal.highlightButton() : pal.border());
  pContext->setLineStyle(
    CLineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound});
  pContext->setLineWidth(borderWidth);
  pContext->drawLine(CPoint(25.0, borderWidth), CPoint(width - 25.0, borderWidth));
  pContext->drawLine(
    CPoint(borderWidth, height - borderWidth),
    CPoint(width - borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(25.0, borderWidth), CPoint(borderWidth, height - borderWidth));
  pContext->drawLine(
    CPoint(width - 25.0, borderWidth), CPoint(width - borderWidth, height - borderWidth));

  setDirty(false);
}

void SplashLabelTpz::onMouseDownEvent(MouseDownEvent &event)
{
  splashView->setVisible(true);
  event.consumed = true;
}

void SplashLabelTpz::onMouseEnterEvent(MouseEnterEvent &event)
{
  isMouseEntered = true;
  invalid();
  event.consumed = true;
}

void SplashLabelTpz::onMouseExitEvent(MouseExitEvent &event)
{
  isMouseEntered = false;
  invalid();
  event.consumed = true;
}

void SplashLabelTpz::onMouseCancelEvent(MouseCancelEvent &event)
{
  isMouseEntered = false;
  event.consumed = true;
}

void SplashLabelTpz::setDefaultFrameWidth(float width) { frameWidth = width; }
void SplashLabelTpz::setHighlightWidth(float width) { highlightFrameWidth = width; }

} // namespace Vst
} // namespace Steinberg
