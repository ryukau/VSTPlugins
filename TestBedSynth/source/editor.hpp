// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  void valueChanged(CControl *pControl) override;
  void updateUI(ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiTextSize = 12.0f;
  float midTextSize = 12.0f;
  float pluginNameTextSize = 18.0f;
  float uiMargin = 20.0f;
  float margin = 5.0f;
  float labelWidth = 100.0f;
  float labelHeight = 20.0f;
  float labelY = labelHeight + 2 * margin;
  float knobWidth = 50.0f;
  float smallKnobWidth = int(knobWidth / 2);
  float knobX = knobWidth + 2 * margin;
  float knobY = knobWidth + labelY;
  float barBoxWidth = 512.0f;
  float barBoxHeight = 200.0f;
  float innerWidth = 2 * labelWidth + 2 * margin + 2 * barBoxWidth + 6 * uiMargin;
  float innerHeight = 3 * labelY + 2 * knobY + 2 * barBoxHeight + 3 * uiMargin;
  int32_t defaultWidth = int32_t(innerWidth + 2 * uiMargin);
  int32_t defaultHeight = int32_t(innerHeight + 2 * uiMargin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 18);
    uiMargin = int(sc * 20);
    margin = int(sc * 5);
    labelWidth = int(sc * 100);
    labelHeight = int(sc * 20);
    labelY = labelHeight + 2 * margin;
    knobWidth = int(sc * 50);
    smallKnobWidth = int(knobWidth / 2);
    knobX = knobWidth + 2 * margin;
    knobY = knobWidth + labelY;
    barBoxWidth = int(sc * 512);
    barBoxHeight = int(sc * 200);
    innerWidth = 2 * labelWidth + 2 * margin + 2 * barBoxWidth + 6 * uiMargin;
    innerHeight = 3 * labelY + 2 * knobY + 2 * barBoxHeight + 3 * uiMargin;
    defaultWidth = int32_t(innerWidth + 2 * uiMargin);
    defaultHeight = int32_t(innerHeight + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  struct XYPadAxis {
    size_t index = 0;
    SharedPointer<XYPad> xypad;
    SharedPointer<CControl> control;

    XYPadAxis(size_t index, XYPad *xypad, CControl *control)
      : xypad(xypad), control(control), index(std::min(index, size_t(1)))
    {
    }

    void sync(float normalized)
    {
      xypad->setValueAt(xypad->id[index], normalized);
      xypad->invalid();
      control->setValue(normalized);
    }
  };

  std::unordered_map<ParamID, std::unique_ptr<XYPadAxis>> xyControlMap;
  void syncUI(ParamID id, float normalized);

  bool prepareUI() override;

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnobVert(
    TabView *tabView,
    int tabIndex,
    CCoord left,
    CCoord top,
    CCoord size,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto knob
      = new Knob<style>(CRect(left, top, left + size, top + size), this, tag, palette);
    knob->setArcWidth(int(palette.guiScale() * 2));
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addLabel(
      left, top + size, size, size, textSize, name, CHoriTxtAlign::kCenterText);

    tabView->addWidget(tabIndex, knob);
    tabView->addWidget(tabIndex, label);

    return knob;
  }

  void addXYControls(
    TabView *tabView,
    int tabIndex,
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord smallKnobWidth,
    CCoord margin,
    CCoord textSize,
    ParamID idX,
    ParamID idY)
  {
    auto xypad = addXYPad(left, top, width, width, idX, idY);
    auto xKnob = addSmallKnobVert(
      tabView, tabIndex, left + width + margin, top + margin, smallKnobWidth, margin,
      textSize, "0", idX);
    auto yKnob = addSmallKnobVert(
      tabView, tabIndex, left + width + margin, top + margin + int(width / 2),
      smallKnobWidth, margin, textSize, "1", idY);

    tabView->addWidget(tabIndex, xypad);

    xyControlMap.emplace(idX, std::make_unique<XYPadAxis>(0, xypad, xKnob));
    xyControlMap.emplace(idY, std::make_unique<XYPadAxis>(1, xypad, yKnob));
  }
};

} // namespace Vst
} // namespace Steinberg
