// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"

#include "../parameter.hpp"
#include "arraycontrol.hpp"

#include <tuple>
#include <unordered_map>

namespace VSTGUI {
class BarBox;
class GroupLabel;
class VGroupLabel;
class Slider;
class TextButton;
class MessageButton;
class CheckBox;
class OptionMenu;
class Knob;
class RotaryKnob;
template<typename Scale> class TextKnob;
template<typename Scale> class NumberKnob;
class TextView;
class TextTableView;
} // namespace VSTGUI

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);

  ~PlugEditor()
  {
    for (auto &ctrl : controlMap)
      if (ctrl.second) ctrl.second->forget();

    for (auto &ctrl : arrayControls)
      if (ctrl) ctrl->forget();
  }

  bool PLUGIN_API
  open(void *parent, const PlatformType &platformType = kDefaultNative) override;
  void PLUGIN_API close() override;
  void valueChanged(CControl *pControl) override;
  void valueChanged(ParamID id, ParamValue normalized);
  void updateUI(Vst::ParamID id, ParamValue normalized);

  void onMouseEntered(CView *view, CFrame *frame) override {}
  void onMouseExited(CView *view, CFrame *frame) override {}
  CMouseEventResult
  onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult
  onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

  BarBox *addBarBox(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID id0,
    size_t nBar,
    std::string name);

  CTextLabel *addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  GroupLabel *addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name);

  VGroupLabel *
  addGroupVerticalLabel(CCoord left, CCoord top, CCoord width, UTF8String name);

  std::tuple<Slider *, CTextLabel *> addVSlider(
    CCoord left,
    CCoord top,
    CColor valueColor,
    UTF8String name,
    ParamID tag,
    UTF8StringPtr tooltip = "",
    bool drawFromCenter = false);

  TextButton *addButton(
    CCoord left,
    CCoord top,
    CCoord width,
    UTF8String title,
    ParamID tag,
    int32_t style = CTextButton::kKickStyle);

  MessageButton *addStateButton(
    CCoord left,
    CCoord top,
    CCoord width,
    std::string title,
    std::string message,
    std::string attribute);

  CheckBox *addCheckbox(
    CCoord left,
    CCoord top,
    CCoord width,
    UTF8String title,
    ParamID tag,
    int32_t style = CCheckBox::Styles::kAutoSizeToFit);

  OptionMenu *addOptionMenu(
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

  std::tuple<Knob *, CTextLabel *> addKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    LabelPosition labelPosition = LabelPosition::bottom);

  std::tuple<RotaryKnob *, CTextLabel *> addRotaryKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    LabelPosition labelPosition = LabelPosition::bottom);

  template<typename Scale>
  std::tuple<NumberKnob<Scale> *, CTextLabel *> addNumberKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    Scale &scale,
    int32_t offset,
    LabelPosition labelPosition = LabelPosition::bottom);

  CTextLabel *addKnobLabel(
    CCoord left,
    CCoord top,
    CCoord right,
    CCoord bottom,
    UTF8String name,
    LabelPosition labelPosition);

  template<typename Scale>
  TextKnob<Scale> *addTextKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    ParamID tag,
    Scale &scale,
    bool isDecibel = false,
    uint32_t precision = 0,
    int32_t offset = 0);

  TextView *addTextView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    std::string text,
    CCoord textSize);

  TextTableView *addTextTableView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    std::string text,
    float cellWidth,
    CCoord textSize);

protected:
  Synth::GlobalParameter param;

  void addToControlMap(Vst::ParamID id, CControl *control)
  {
    auto iter = controlMap.find(id);
    if (iter != controlMap.end()) iter->second->forget();
    control->remember();
    controlMap.insert({id, control});
  }

  std::unordered_map<Vst::ParamID, CControl *> controlMap;
  std::vector<ArrayControl *> arrayControls;

  float uiTextSize = 14.0f;
  float midTextSize = 16.0f;
  float pluginNameTextSize = 22.0f;
  float margin = 5.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f; // With margin.
  float knobY = knobHeight + labelY;
  float barboxWidth = 12.0f * knobX;
  float barboxHeight = 2.0f * knobY;
  float barboxY = barboxHeight + 2.0f * margin;
  float checkboxWidth = 60.0f;
  float splashHeight = 40.0f;
  int32 defaultWidth = int32(barboxWidth + labelY + 2 * knobX + 12 * margin + 40);
  int32 defaultHeight = int32(40 + labelY + 4 * barboxY + 9 * margin);

  ViewRect viewRect{0, 0, defaultWidth, defaultHeight};

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
