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

#include "../../../common/gui/style.hpp"
#include "vstgui/vstgui.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class SplashLabelTpz : public CControl {
public:
  SplashLabelTpz(
    const CRect &size,
    IControlListener *listener,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    int32_t tag,
    CControl *splashView,
    std::string label)
    : CControl(size, listener, tag)
    , fontId(fontId)
    , pal(palette)
    , splashView(splashView)
    , label(label)
  {
    if (splashView != nullptr) splashView->remember();
  }

  ~SplashLabelTpz()
  {
    if (splashView != nullptr) splashView->forget();
  }

  CLASS_METHODS(SplashLabelTpz, CControl);

  void draw(CDrawContext *pContext) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setDefaultFrameWidth(float width);
  void setHighlightWidth(float width);

protected:
  CControl *splashView = nullptr;
  std::string label;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  float frameWidth = 4.0f;
  float highlightFrameWidth = 4.0f;

  bool isMouseEntered = false;
};

} // namespace Vst
} // namespace Steinberg
