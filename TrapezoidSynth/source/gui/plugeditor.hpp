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

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"

#include "../parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);

  bool PLUGIN_API
  open(void *parent, const PlatformType &platformType = kDefaultNative) override;
  void PLUGIN_API close() override;
  void valueChanged(CControl *pControl) override;

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

  void addGroupLabel(
    CCoord left, CCoord top, CCoord width, CCoord lineMargin, UTF8String name);
  void addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  void addWaveView(const CRect &size);

  void addSplashScreen(
    float buttonLeft,
    float buttonTop,
    float buttonWidth,
    float buttonHeight,
    float splashLeft,
    float splashTop,
    float splashWidth,
    float splashHeight,
    const char *pluginName);

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
    CCoord width,
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
    Synth::GlobalParameter &param,
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
    Synth::GlobalParameter &param,
    LabelPosition labelPosition = LabelPosition::bottom);

  void addKnobLabel(
    float left,
    float top,
    float right,
    float bottom,
    UTF8String name,
    LabelPosition labelPosition);

protected:
  const float uiTextSize = 12.0f;
  const float midTextSize = 14.0f;
  const float pluginNameTextSize = 24.0f;
  const float margin = 5.0f;
  const float labelHeight = 20.0f;
  const float labelY = 30.0f;
  const float knobWidth = 50.0f;
  const float knobHeight = knobWidth - 2.0f * margin;
  const float knobX = knobWidth; // With margin.
  const float knobY = knobHeight + labelY + 2.0f * margin;
  const float sliderWidth = 70.0f;
  const float sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
  const float sliderX = 80.0f;
  const float sliderY = sliderHeight + labelY;
  const float checkboxWidth = 80.0f;
  const float groupLabelMargin = 10.0f;

  ViewRect viewRect{0, 0, int32(40 + 13 * knobX), int32(20 + 5 * (knobY + labelHeight))};

  CCoord fontSize = 12.0;

  CCoord frameWidth = 2.0;

  CColor colorBlack{0, 0, 0, 255};
  CColor colorWhite{255, 255, 255, 255};
  CColor colorBlue{11, 164, 241, 255};
  CColor colorOrange{252, 192, 79, 255};
  CColor colorRed{252, 128, 128, 255};
  CColor colorGreen{19, 193, 54, 255};
  CColor colorFaintGray{237, 237, 237, 255};

  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
