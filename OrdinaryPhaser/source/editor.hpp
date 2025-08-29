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

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 14.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float knobWidth = 60.0f;
  float knobX = knobWidth + 2 * margin;
  float knobY = knobWidth + labelHeight + 2 * margin;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 100.0f;
  float labelX = labelWidth + margin;

  float barboxWidth = 512.0f;
  float barboxHeight = 200.0f;

  int32_t defaultWidth = int32_t(2 * uiMargin + 3 * knobX + barboxWidth + 2 * margin);
  int32_t defaultHeight
    = int32_t(2 * uiMargin + 4 * labelY - 3 * margin + knobY + barboxHeight);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    knobWidth = int(sc * 60);
    knobX = knobWidth + 2 * margin;
    knobY = knobWidth + labelHeight + 2 * margin;
    labelY = labelHeight + 2 * margin;
    labelWidth = int(sc * 100);
    labelX = labelWidth + margin;

    barboxWidth = int(sc * 512);
    barboxHeight = int(sc * 200);

    defaultWidth = int32_t(2 * uiMargin + 3 * knobX + barboxWidth + 2 * margin);
    defaultHeight
      = int32_t(2 * uiMargin + 4 * labelY - 3 * margin + knobY + barboxHeight);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(CCoord left, CCoord top, CCoord width, CCoord height, ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setArcWidth(std::max(int(2), int(palette.guiScale() * 2)));
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }
};

} // namespace Vst
} // namespace Steinberg
