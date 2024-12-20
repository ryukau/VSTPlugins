// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "splash.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

void CreditView::onMouseDownEvent(MouseDownEvent &event)
{
  if (!event.buttonState.isLeft()) return;
  setVisible(false);
  event.consumed = true;
  event.ignoreFollowUpMoveAndUpEvents(true);
}

void CreditView::onMouseEnterEvent(MouseEnterEvent &event)
{
  isMouseEntered = true;
  invalid();
  event.consumed = true;
}

void CreditView::onMouseExitEvent(MouseExitEvent &event)
{
  isMouseEntered = false;
  invalid();
  event.consumed = true;
}

void SplashLabel::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();

  const double borderWidth = isMouseEntered ? highlightFrameWidth : frameWidth;
  const double halfBorderWidth = int(borderWidth / 2.0);
  pContext->setFillColor(pal.boxBackground());
  pContext->setFrameColor(isMouseEntered ? pal.highlightButton() : pal.border());
  pContext->setLineWidth(borderWidth);
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawFilledAndStroked);

  pContext->setFont(fontId);
  pContext->setFontColor(pal.foreground());
  pContext->drawString(label.c_str(), CRect(0.0, 0.0, width, height), kCenterText, true);

  setDirty(false);
}

void SplashLabel::onMouseDownEvent(MouseDownEvent &event)
{
  splashView->setVisible(true);
  event.consumed = true;
}

void SplashLabel::onMouseEnterEvent(MouseEnterEvent &event)
{
  isMouseEntered = true;
  invalid();
  event.consumed = true;
}

void SplashLabel::onMouseExitEvent(MouseExitEvent &event)
{
  isMouseEntered = false;
  invalid();
  event.consumed = true;
}

void SplashLabel::onMouseCancelEvent(MouseCancelEvent &event)
{
  isMouseEntered = false;
  event.consumed = true;
}

void SplashLabel::setDefaultFrameWidth(CCoord width) { frameWidth = width; }
void SplashLabel::setHighlightWidth(CCoord width) { highlightFrameWidth = width; }

} // namespace Vst
} // namespace Steinberg
