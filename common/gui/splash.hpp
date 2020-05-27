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

#include <string>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class CreditView : public CControl {
public:
  CreditView(const CRect &size, IControlListener *listener, Uhhyou::Palette &palette)
    : CControl(size, listener), pal(palette)
  {
    setVisible(false);

    fontIdTitle = new CFontDesc(Uhhyou::Font::name(), fontSizeTitle, CTxtFace::kBoldFace);
    fontIdText = new CFontDesc(Uhhyou::Font::name(), fontSize);
  }

  ~CreditView()
  {
    if (fontIdTitle) fontIdTitle->forget();
    if (fontIdText) fontIdText->forget();
  }

  void draw(CDrawContext *pContext) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CLASS_METHODS(CreditView, CControl)

private:
  CCoord fontSize = 12.0;
  CCoord fontSizeTitle = 18.0;

  CFontRef fontIdTitle = nullptr;
  CFontRef fontIdText = nullptr;
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
    CCoord fontSize,
    Uhhyou::Palette &palette)
    : CControl(size, listener, tag)
    , splashView(splashView)
    , label(label)
    , fontSize(fontSize)
    , pal(palette)
  {
    if (splashView != nullptr) splashView->remember();
    fontId = new CFontDesc(Uhhyou::Font::name(), fontSize, CTxtFace::kBoldFace);
  }

  ~SplashLabel()
  {
    if (splashView != nullptr) splashView->forget();
    if (fontId) fontId->forget();
  }

  CLASS_METHODS(SplashLabel, CControl);

  void draw(CDrawContext *pContext) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameWidth(CCoord width);
  void setHighlightWidth(CCoord width);

protected:
  CControl *splashView = nullptr;
  std::string label;

  CCoord fontSize = 18.0;
  CFontRef fontId = nullptr;
  Uhhyou::Palette &pal;

  CCoord frameWidth = 1.0f;
  CCoord highlightFrameWidth = 2.0f;

  bool isMouseEntered = false;
};

} // namespace Vst
} // namespace Steinberg
