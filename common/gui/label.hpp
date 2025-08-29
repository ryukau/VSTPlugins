// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)
// Copyright Konstantin Voinov

#pragma once

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <string>

namespace VSTGUI {

class Label : public CControl {
public:
  Label(
    const CRect &size,
    IControlListener *listener,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    CHoriTxtAlign align = kCenterText)
    : CControl(size, listener), label(label), fontId(fontId), pal(palette), align(align)
  {
  }

  CLASS_METHODS(Label, CControl);

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), align, true);

    setDirty(false);
  }

  void setText(std::string content) { label = content; }

protected:
  std::string label;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CHoriTxtAlign align = kCenterText;
};

class GroupLabel : public CControl {
public:
  GroupLabel(
    const CRect &size,
    IControlListener *listener,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener), label(label), fontId(fontId), pal(palette)
  {
  }

  CLASS_METHODS(GroupLabel, CControl);

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    const auto width = getWidth();
    const auto height = getHeight();
    const auto sc = pal.guiScale();

    // Background.
    pContext->setFillColor(pal.background());
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    const auto textWidth = pContext->getStringWidth(label.c_str());
    const auto textLeft = (width - textWidth) * 0.5;
    const auto textRight = (width + textWidth) * 0.5;
    pContext->drawString(label.c_str(), CRect(textLeft, 0, textRight, height));

    // Border.
    pContext->setFrameColor(pal.borderLabel());
    pContext->setLineWidth(sc * lineWidth);
    pContext->drawLine(
      CPoint(0.0, height * 0.5), CPoint(textLeft - sc * margin, height * 0.5));
    pContext->drawLine(
      CPoint(textRight + sc * margin, height * 0.5), CPoint(width, height * 0.5));

    setDirty(false);
  }

protected:
  std::string label;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  double lineWidth = 2.0;
  double margin = 10.0;
};

class VGroupLabel : public GroupLabel {
public:
  VGroupLabel(
    const CRect &size,
    IControlListener *listener,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : GroupLabel(size, listener, label, fontId, palette)
  {
  }

  CLASS_METHODS(VGroupLabel, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    // It seems like font rendering will be disabled when rotated.
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().rotate(90).translate(getViewSize().getTopRight()));

    // Background.
    pContext->setFillColor(pal.boxBackground()); // This color is set for debug.
    pContext->drawRect(CRect(0.0, 0.0, height, width), kDrawFilled);

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    const auto textWidth = pContext->getStringWidth(label.c_str());
    const auto textLeft = (height - textWidth) * 0.5;
    const auto textRight = (height + textWidth) * 0.5;
    pContext->drawString(label.c_str(), CRect(textLeft, 0, textRight, width));

    setDirty(false);
  }
};

} // namespace VSTGUI
