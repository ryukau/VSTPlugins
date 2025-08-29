// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

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
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 16.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 120.0f;
  float labelWidthHalf = int(labelWidth / 2);
  float lfoKnobWidth = 2 * labelHeight;
  float groupLabelWidth = 2 * labelWidth + 2 * margin;

  int32_t defaultWidth = int32_t(3 * uiMargin + 2 * groupLabelWidth + lfoKnobWidth);
  int32_t defaultHeight = int32_t(uiMargin + 13 * labelY);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 16);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    labelY = labelHeight + 2 * margin;
    labelWidth = int(sc * 120);
    labelWidthHalf = int(labelWidth / 2);
    lfoKnobWidth = 2 * labelHeight;
    groupLabelWidth = 2 * labelWidth + 2 * margin;

    defaultWidth = int32_t(3 * uiMargin + 2 * groupLabelWidth + lfoKnobWidth);
    defaultHeight = int32_t(uiMargin + 13 * labelY);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(CCoord left, CCoord top, CCoord width, CCoord height, ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setArcWidth(2.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
