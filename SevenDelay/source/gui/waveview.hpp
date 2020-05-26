// (c) 2019-2020 Takamitsu Endo
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

#include "../../../common/gui/style.hpp"

namespace VSTGUI {

class WaveView : public CControl {
public:
  WaveView(const CRect &size, Uhhyou::Palette &palette);

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(WaveView, CControl);

  double lfo(double phase);

  double amount = 0.9;
  double shape = 0.0;
  double phase = 0.0;

protected:
  CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};
  CDrawContext::PointList points;

  Uhhyou::Palette &pal;
};

} // namespace VSTGUI
