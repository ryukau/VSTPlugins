// (c) 2019-2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

#include "../../../common/gui/style.hpp"

namespace VSTGUI {

class GroupLabelTpz : public CControl {
public:
  GroupLabelTpz(
    const CRect &size,
    IControlListener *listener,
    std::string text,
    CFontRef fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener), text(text), fontId(fontId), pal(palette)
  {
    this->fontId->remember();
  }

  ~GroupLabelTpz()
  {
    if (fontId) fontId->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    const auto width = getWidth();
    const auto height = getHeight();

    // Background.
    pContext->setFillColor(pal.background());
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    const auto textWidth = pContext->getStringWidth(text.c_str());
    const auto textLeft = 0;
    const auto textRight = textWidth;
    pContext->drawString(text.c_str(), CRect(textLeft, 0, textRight, height));

    // Border.
    pContext->setFrameColor(pal.border());
    pContext->setLineWidth(lineWidth);
    pContext->drawLine(
      CPoint(textWidth + margin, height * 0.5), CPoint(width - 5.0, height * 0.5));

    setDirty(false);
  }

  CLASS_METHODS(GroupLabelTpz, CControl);

  void setMargin(double margin) { this->margin = margin; }

protected:
  double fontSize = 14.0;
  std::string text;
  CFontRef fontId = nullptr;

  double lineWidth = 1.0;
  double margin = 10.0;
  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  Uhhyou::Palette &pal;
};

} // namespace VSTGUI
