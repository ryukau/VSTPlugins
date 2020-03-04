// (c) 2019 Takamitsu Endo
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

#include "../../../common/gui/guistyle.hpp"
#include "vstgui/vstgui.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class SplashLabelTpz : public CControl {
public:
  SplashLabelTpz(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    CControl *splashView,
    UTF8StringPtr txt = nullptr)
    : CControl(size, listener, tag), splashView(splashView), txt(txt)
  {
    if (splashView != nullptr) splashView->remember();
    fontID = new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kBoldFace);
  }

  ~SplashLabelTpz()
  {
    if (splashView != nullptr) splashView->forget();
    if (fontID) fontID->forget();
  }

  CLASS_METHODS(SplashLabelTpz, CControl);

  void draw(CDrawContext *pContext) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameColor(CColor color);
  void setHighlightColor(CColor color);
  void setDefaultFrameWidth(float width);
  void setHighlightWidth(float width);

protected:
  CControl *splashView = nullptr;
  UTF8StringPtr txt = nullptr;

  CFontRef fontID = nullptr;
  double fontSize = 18.0;
  CColor fontColor = CColor(0, 0, 0, 255);

  CColor frameColor = CColor(0, 0, 0, 255);
  CColor highlightColor = CColor(0, 0, 0, 255);
  float frameWidth = 4.0f;
  float highlightFrameWidth = 4.0f;

  bool isMouseEntered = false;
};

} // namespace Vst
} // namespace Steinberg
