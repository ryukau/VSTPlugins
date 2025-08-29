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
  float midTextSize = 12.0f;
  float pluginNameTextSize = 24.0f;
  float margin = 5.0f;
  float uiMargin = 4 * margin;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 80.0f;
  float knobHeight = knobWidth - 2.0f * margin;
  float knobX = knobWidth; // With margin.
  float knobY = knobHeight + labelY + 2.0f * margin;
  float sliderWidth = 70.0f;
  float sliderHeight = 2.0f * (knobHeight + labelY) + 75.0f;
  float sliderX = 80.0f;
  float sliderY = sliderHeight + labelY;
  float checkboxWidth = 80.0f;
  uint32_t defaultWidth = uint32_t(2 * 20 + sliderX + 7 * knobX + 10 * margin);
  uint32_t defaultHeight = uint32_t(2 * 15 + 2 * knobY + 2 * labelHeight + 55);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 24);
    margin = int(sc * 5);
    uiMargin = 4 * margin;
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 80);
    knobHeight = knobWidth - 2 * margin;
    knobX = knobWidth; // With margin.
    knobY = knobHeight + labelY + 2 * margin;
    sliderWidth = int(sc * 70);
    sliderHeight = 2 * (knobHeight + labelY) + int(sc * 75);
    sliderX = int(sc * 80);
    sliderY = sliderHeight + labelY;
    checkboxWidth = sliderX;
    defaultWidth = uint32_t(2 * uiMargin + sliderX + 7 * knobX + 10 * margin);
    defaultHeight = uint32_t(17 * margin + 2 * knobY + 2 * labelHeight);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
