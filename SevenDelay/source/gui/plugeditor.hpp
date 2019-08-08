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

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);

  bool PLUGIN_API open(void *parent, const PlatformType &platformType = kDefaultNative);
  void PLUGIN_API close();
  void valueChanged(CControl *pControl);

  void onMouseEntered(CView *view, CFrame *frame) override {}
  void onMouseExited(CView *view, CFrame *frame) override {}
  CMouseEventResult
  onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons) override
  {
    return kMouseEventNotHandled;
  }
  CMouseEventResult
  onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

  void addPluginNameLabel(CCoord left, CCoord top, CCoord width, UTF8String name);
  void addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name);
  void addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  void addWaveView(const CRect &size);

  void addSplashScreen(CRect &buttonRect, CRect splashRect);

  void addVSlider(
    CCoord left,
    CCoord top,
    CColor valueColor,
    UTF8String name,
    ParamID tag,
    UTF8StringPtr tooltip = "",
    bool drawFromCenter = false);

  void addButton(
    CCoord left,
    CCoord top,
    CCoord width,
    UTF8String title,
    ParamID tag,
    int32_t style = CTextButton::kKickStyle);

  void addCheckbox(
    CCoord left,
    CCoord top,
    UTF8String title,
    ParamID tag,
    int32_t style = CCheckBox::Styles::kAutoSizeToFit);

  void addOptionMenu(
    CCoord left, CCoord top, ParamID tag, const std::vector<UTF8String> &items);

  void addKnob(
    CCoord left,
    CCoord top,
    CColor valueColor,
    UTF8String name,
    ParamID tag,
    UTF8StringPtr tooltip = "",
    bool drawFromCenter = false);

protected:
  ViewRect viewRect{0, 0, 970, 400};

  UTF8String fontName{"DejaVu Sans Mono"};
  CCoord fontSize = 18.0;

  CCoord frameWidth = 2.0;

  CColor colorBlack{0, 0, 0, 255};
  CColor colorWhite{255, 255, 255, 255};
  CColor colorBlue{11, 164, 241, 255};
  CColor colorOrange{252, 192, 79, 255};
  CColor colorGreen{19, 193, 54, 255};
  CColor colorFaintGray{237, 237, 237, 255};

  const CRect WaveViewSize = {760.0, 330.0, 940.0, 390.0};
  const CPoint WaveViewPos = {WaveViewSize.left + 1.0, WaveViewSize.top + 1.0};

  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
