// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace VSTGUI {

class TextView : public CControl {
public:
  CCoord lineHeight = 20.0;

  TextView(
    const CRect &size,
    std::string text,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, nullptr, -1), fontId(fontId), pal(palette)
  {
    setText(text);
  }

  void setText(std::string text)
  {
    str.clear();

    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line, '\n')) {
      if (line.size() <= 0)
        str.push_back(" ");
      else
        str.push_back(line);
    }
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Text.
    const float lh = pal.guiScale() * lineHeight;
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    for (size_t idx = 0; idx < str.size(); ++idx) {
      pContext->drawString(
        str[idx].c_str(), CRect(0, idx * lh, getWidth(), (idx + 1) * lh), kLeftText);
    }
  }

  //// TODO: Implement line highlight.
  // void onMouseEnterEvent (MouseEnterEvent& event)
  // override; void onMouseExitEvent (MouseExitEvent& event)
  // override; void onMouseMoveEvent (MouseMoveEvent& event)
  // override;

  CLASS_METHODS(TextView, CControl);

protected:
  std::vector<std::string> str;
  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
};

class TextTableView : public CControl {
public:
  static constexpr char rowDelimiter = '\n';
  static constexpr char colDelimiter = '|';

  CCoord cellWidth = 100.0;
  CCoord lineHeight = 20.0;

  TextTableView(
    const CRect &size,
    std::string content,
    CCoord cellWidth,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, nullptr, -1), cellWidth(cellWidth), fontId(fontId), pal(palette)
  {
    setText(content);
  }

  void setText(std::string content)
  {
    std::stringstream ssContent(content);
    std::string line;
    std::string cell;
    table.resize(0);
    while (std::getline(ssContent, line, rowDelimiter)) {
      table.push_back(std::vector<std::string>());
      std::stringstream ssLine(line);
      while (std::getline(ssLine, cell, colDelimiter)) {
        if (cell.size() <= 0)
          table.back().push_back(" ");
        else
          table.back().push_back(cell);
      }
    }
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Text.
    const float lh = pal.guiScale() * lineHeight;
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    for (size_t row = 0; row < table.size(); ++row) {
      for (size_t col = 0; col < table[row].size(); ++col) {
        const CCoord left = col * cellWidth;
        const CCoord top = row * lh;
        const CRect rect = CRect(left, top, left + cellWidth, top + lh);
        pContext->drawString(table[row][col].c_str(), rect, kLeftText);
      }
    }
  }

  //// TODO: Implement line highlight.

  CLASS_METHODS(TextTableView, CControl);

protected:
  std::vector<std::vector<std::string>> table;
  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
};

template<typename Scale> class ValueTextView : public CControl {
public:
  double lineHeight = 20.0;

  ValueTextView(
    const CRect &size,
    std::string name,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Scale &scale)
    : CControl(size, nullptr, -1), fontId(fontId), pal(palette), scale(scale)
  {
    setName(name);
  }

  void setName(std::string name) { name_ = name; }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Text.
    const float lh = pal.guiScale() * lineHeight;
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(name_.c_str(), CRect(0, 0, getWidth(), lh), kLeftText);

    std::string value_str = std::to_string(scale.map(value));
    pContext->drawString(
      value_str.c_str(), CRect(getWidth() / 2, 0, getWidth(), lh), kLeftText);
  }

  CLASS_METHODS(ValueTextView, CControl);

protected:
  std::string name_;
  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;
  Scale &scale;
};

} // namespace VSTGUI
