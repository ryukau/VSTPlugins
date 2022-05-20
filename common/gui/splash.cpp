// (c) 2019-2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

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
