// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

  void onMouseDownEvent(MouseDownEvent &event) override;
  void onMouseEnterEvent(MouseEnterEvent &event) override;
  void onMouseExitEvent(MouseExitEvent &event) override;
  void onMouseCancelEvent(MouseCancelEvent &event) override;

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
