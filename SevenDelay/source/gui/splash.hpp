// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class CreditView : public CControl {
public:
  CreditView(const CRect &size, IControlListener *listener) : CControl(size, listener) {}

  void draw(CDrawContext *pContext) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) {
      valueChanged();
      return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }
    return kMouseEventNotHandled;
  }

  CLASS_METHODS(CreditView, CControl)

private:
  UTF8String fontName{"DejaVu Sans Mono"};
  CCoord fontSize = 18.0;
  CCoord fontSizeTitle = 24.0;
};

} // namespace Vst
} // namespace Steinberg
