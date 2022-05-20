// (c) 2020 Takamitsu Endo
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

#pragma once

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <string>

namespace VSTGUI {

template<Uhhyou::Style style = Uhhyou::Style::common> class CheckBox : public CControl {
public:
  std::string label;
  bool drawBackground = false;

  CheckBox(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener, tag), label(label), fontId(fontId), pal(palette)
  {
  }

  CLASS_METHODS(CheckBox, CControl);

  void draw(CDrawContext *pContext) override
  {
    using namespace Uhhyou;

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    const auto width = getWidth();
    const auto height = getHeight();
    const auto centerY = height / 2;

    // Background.
    if (drawBackground) {
      pContext->setFillColor(pal.background());
      pContext->drawRect(CRect(0, 0, width, height), kDrawFilled);
    }

    // Border.
    pContext->setFillColor(pal.boxBackground());
    if constexpr (style == Style::accent) {
      pContext->setFrameColor(
        isMouseEntered ? pal.highlightAccent() : pal.borderCheckbox());
    } else if (style == Style::warning) {
      pContext->setFrameColor(
        isMouseEntered ? pal.highlightWarning() : pal.borderCheckbox());
    } else {
      pContext->setFrameColor(
        isMouseEntered ? pal.highlightMain() : pal.borderCheckbox());
    }
    pContext->setLineWidth(borderWidth);
    const auto boxLeft = 2;
    const auto boxTop = centerY - boxSize / 2;
    pContext->drawRect(
      CRect(boxLeft, boxTop, boxLeft + boxSize, boxTop + boxSize), kDrawFilledAndStroked);

    if (value) {
      auto innerBoxSize = boxSize - 4;
      if constexpr (style == Style::accent) {
        pContext->setFillColor(isMouseEntered ? pal.highlightAccent() : pal.foreground());
      } else if (style == Style::warning) {
        pContext->setFillColor(
          isMouseEntered ? pal.highlightWarning() : pal.foreground());
      } else {
        pContext->setFillColor(isMouseEntered ? pal.highlightMain() : pal.foreground());
      }
      const auto checkLeft = 2 + (boxSize - innerBoxSize) / 2;
      const auto checkTop = centerY - innerBoxSize / 2;
#ifndef __linux__
      --innerBoxSize;
#endif
      pContext->drawRect(
        CRect(checkLeft, checkTop, checkLeft + innerBoxSize, checkTop + innerBoxSize),
        kDrawFilled);
    }

    // Text.
    if (label.size() == 0) return;
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    const auto textLeft = boxSize + textSize / 2.0f;
    pContext->drawString(label.c_str(), CRect(textLeft, 0, width, height), align, true);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;
    value = value == 0 ? 1 : 0;
    valueChanged();
    invalid();
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void setBorderWidth(float width) { borderWidth = width; }
  void setTextSize(float size) { textSize = size < 0.0f ? 0.0f : size; }

protected:
  CCoord boxSize = 10.0;
  CCoord borderWidth = 2.0;
  CCoord textSize = 14.0;

  CHoriTxtAlign align = kLeftText;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
};

} // namespace VSTGUI
