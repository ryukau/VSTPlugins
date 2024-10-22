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
    knob->setArcWidth(2.0);
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
