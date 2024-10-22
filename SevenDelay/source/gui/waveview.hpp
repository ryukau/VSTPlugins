// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
