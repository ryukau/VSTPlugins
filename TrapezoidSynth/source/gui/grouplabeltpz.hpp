// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener), text(text), fontId(fontId), pal(palette)
  {
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
  SharedPointer<CFontDesc> fontId;

  double lineWidth = 1.0;
  double margin = 10.0;
  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  Uhhyou::Palette &pal;
};

} // namespace VSTGUI
