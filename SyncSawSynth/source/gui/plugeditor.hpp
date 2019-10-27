// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

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

  void addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name);
  void addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  void addWaveView(const CRect &size);

  void addSplashScreen(CRect buttonRect, CRect splashRect);

  void addVSlider(
    CCoord left,
    CCoord top,
    CColor valueColor,
    UTF8String name,
    ParamID tag,
    float defaultValue,
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
    CCoord left,
    CCoord top,
    CCoord width,
    ParamID tag,
    const std::vector<UTF8String> &items);

  enum class LabelPosition {
    top,
    left,
    bottom,
    right,
  };

  void addKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    float defaultValue,
    LabelPosition labelPosition = LabelPosition::bottom);

  template<typename Scale>
  void addNumberKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    Scale &scale,
    int32_t offset,
    float defaultValue,
    LabelPosition labelPosition = LabelPosition::bottom);

  void addKnobLabel(
    float left,
    float top,
    float right,
    float bottom,
    UTF8String name,
    LabelPosition labelPosition);

  void addOscillatorSection(
    UTF8String label,
    double left,
    double top,
    ParamID tagGain,
    ParamID tagSemi,
    ParamID tagCent,
    ParamID tagSync,
    ParamID tagSyncType,
    ParamID tagPTROrder,
    ParamID tagPhase,
    ParamID tagPhaseLock,
    float defaultGain,
    float defaultSemi,
    float defaultCent,
    float defaultSync,
    float defaultPhase);

protected:
  const double margin = 5.0;
  const double labelHeight = 20.0;
  const double labelY = 30.0;
  const double knobWidth = 50.0;
  const double knobHeight = 40.0;
  const double knobX = 60.0; // With margin.
  const double knobY = knobHeight + labelY;

  ViewRect viewRect{0, 0, int32(10.0 * knobX + 50.0),
                    int32(20.0 + 3.0 * labelY + 6.0 * knobY)};

  CCoord fontSize = 12.0;

  CCoord frameWidth = 2.0;

  CColor colorBlack{0, 0, 0, 255};
  CColor colorWhite{255, 255, 255, 255};
  CColor colorBlue{11, 164, 241, 255};
  CColor colorOrange{252, 192, 79, 255};
  CColor colorGreen{19, 193, 54, 255};
  CColor colorFaintGray{237, 237, 237, 255};

  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
