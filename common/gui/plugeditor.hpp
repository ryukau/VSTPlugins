// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"

#include "../parameterInterface.hpp"
#include "arraycontrol.hpp"

#include "barbox.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "tabview.hpp"
#include "textview.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);
  ~PlugEditor();

  bool PLUGIN_API
  open(void *parent, const PlatformType &platformType = kDefaultNative) override;
  void PLUGIN_API close() override;
  virtual void valueChanged(CControl *pControl) override;
  virtual void valueChanged(ParamID id, ParamValue normalized);
  virtual void updateUI(Vst::ParamID id, ParamValue normalized);

  virtual void onMouseEntered(CView *view, CFrame *frame) override {}
  virtual void onMouseExited(CView *view, CFrame *frame) override {}
  virtual CMouseEventResult
  onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;
  virtual CMouseEventResult
  onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

  void addSplashScreen(
    float buttonLeft,
    float buttonTop,
    float buttonWidth,
    float buttonHeight,
    float splashLeft,
    float splashTop,
    float splashWidth,
    float splashHeight,
    const char *pluginName,
    float buttonFontSize = 18.0);

  template<typename Scale>
  BarBox<Scale> *addBarBox(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID id0,
    size_t nBar,
    Scale &scale,
    std::string name)
  {
    std::vector<ParamID> id(nBar);
    for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      value[i] = controller->getParamNormalized(id[i]);
    std::vector<double> defaultValue(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      defaultValue[i] = param->getDefaultNormalized(id[i]);

    auto barBox = new BarBox<Scale>(
      this, CRect(left, top, left + width, top + height), id, value, defaultValue);
    barBox->setIndexFont(
      new CFontDesc(PlugEditorStyle::fontName(), 10.0, CTxtFace::kBoldFace));
    barBox->setNameFont(
      new CFontDesc(PlugEditorStyle::fontName(), 24.0, CTxtFace::kNormalFace));
    barBox->setBorderColor(colorBlack);
    barBox->setValueColor(colorBlue);
    barBox->setName(name);
    frame->addView(barBox);

    auto iter = arrayControlInstances.find(id0);
    if (iter != arrayControlInstances.end()) {
      iter->second->forget();
      arrayControlInstances.erase(iter);
    }
    barBox->remember();
    arrayControlInstances.emplace(std::make_pair(id0, barBox));
    for (size_t i = 0; i < id.size(); ++i)
      arrayControlMap.emplace(std::make_pair(id0 + i, barBox));
    return barBox;
  }

  CTextLabel *addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  GroupLabel *addGroupLabel(CCoord left, CCoord top, CCoord width, std::string name);

  VGroupLabel *
  addGroupVerticalLabel(CCoord left, CCoord top, CCoord width, std::string name);

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

  KickButton *
  addKickButton(CCoord left, CCoord top, CCoord width, std::string title, ParamID tag);

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
    LabelPosition labelPosition = LabelPosition::bottom)
  {
    auto bottom = top + width - 10.0;
    auto right = left + width;

    auto knob = new NumberKnob<Scale>(
      CRect(left + 5.0, top, right - 5.0, bottom), this, tag, scale, offset);
    knob->setFont(
      new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
    knob->setSlitWidth(8.0);
    knob->setHighlightColor(highlightColor);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
    return std::make_tuple(knob, label);
  }

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
    int32_t offset = 0)
  {
    auto bottom = top + labelHeight;
    auto right = left + width;

    auto knob
      = new TextKnob<Scale>(CRect(left, top, right, bottom), this, tag, scale, isDecibel);
    knob->setFont(
      new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
    knob->setHighlightColor(highlightColor);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    knob->setPrecision(precision);
    knob->offset = offset;
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }

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

  TabView *addTabView(
    float left,
    float top,
    float width,
    float hegiht,
    float tabHeight,
    CColor highlightColor,
    std::vector<std::string> tabs);

protected:
  void addToControlMap(Vst::ParamID id, CControl *control);
  virtual bool prepareUI() = 0;

  std::unique_ptr<ParameterInterface> param;

  std::unordered_map<Vst::ParamID, CControl *> controlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlInstances; // Dirty hack.

  float uiMargin = 20.0f;
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
  float textKnobX = knobX;
  float checkboxWidth = knobX;
  float sliderWidth = 70.0f;
  float sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
  float sliderX = 80.0f;
  float sliderY = sliderHeight + labelY;
  float barboxWidth = 12.0f * knobX;
  float barboxHeight = 2.0f * knobY;
  float barboxY = barboxHeight + 2.0f * margin;
  float tabViewWidth = 200.0f;
  float tabViewHeight = 200.0f;
  float splashHeight = 40.0f;
  int32 defaultWidth = int32(512);
  int32 defaultHeight = int32(512);

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
};

} // namespace Vst
} // namespace Steinberg
