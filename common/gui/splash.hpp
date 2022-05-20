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

#pragma once

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <sstream>
#include <string>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class CreditView : public CControl {
public:
  CreditView(
    const CRect &size,
    IControlListener *listener,
    const SharedPointer<CFontDesc> &fontIdTitle,
    const SharedPointer<CFontDesc> &fontIdText,
    Uhhyou::Palette &palette)
    : CControl(size, listener)
    , fontIdTitle(fontIdTitle)
    , fontIdText(fontIdText)
    , pal(palette)
  {
    setVisible(false);
  }

  void draw(CDrawContext *pContext) override;
  void onMouseDownEvent(MouseDownEvent &event) override;
  void onMouseEnterEvent(MouseEnterEvent &event) override;
  void onMouseExitEvent(MouseExitEvent &event) override;
  CLASS_METHODS(CreditView, CControl)

private:
  void drawTextBlock(
    CDrawContext *pContext,
    float left,
    float top,
    float lineHeight,
    float blockWidth,
    std::string str,
    char colDelimiter = '|',
    char rowDelimiter = '\n')
  {
    std::stringstream ssText(str);
    std::string line;
    std::string cell;
    float leftStart = left;
    while (std::getline(ssText, line, rowDelimiter)) {
      std::stringstream ssLine(line);
      left = leftStart;
      while (std::getline(ssLine, cell, colDelimiter)) {
        if (cell.size() > 0) pContext->drawString(cell.c_str(), CPoint(left, top));
        left += blockWidth;
      }
      top += lineHeight;
    }
  }

  SharedPointer<CFontDesc> fontIdTitle;
  SharedPointer<CFontDesc> fontIdText;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
};

class SplashLabel : public CControl {
public:
  SplashLabel(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    CControl *splashView,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener, tag)
    , splashView(splashView)
    , label(label)
    , fontId(fontId)
    , pal(palette)
  {
    if (splashView != nullptr) splashView->remember();
  }

  ~SplashLabel()
  {
    if (splashView != nullptr) splashView->forget();
  }

  CLASS_METHODS(SplashLabel, CControl);

  void draw(CDrawContext *pContext) override;

  void onMouseDownEvent(MouseDownEvent &event) override;
  void onMouseEnterEvent(MouseEnterEvent &event) override;
  void onMouseExitEvent(MouseExitEvent &event) override;
  void onMouseCancelEvent(MouseCancelEvent &event) override;

  void setDefaultFrameWidth(CCoord width);
  void setHighlightWidth(CCoord width);

protected:
  CControl *splashView = nullptr;
  std::string label;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord frameWidth = 1.0f;
  CCoord highlightFrameWidth = 2.0f;

  bool isMouseEntered = false;
};

} // namespace Vst
} // namespace Steinberg
