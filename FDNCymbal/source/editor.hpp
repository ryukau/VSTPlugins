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
  float knobRightMargin = 0.0f;
  float knobX = knobWidth; // With margin.
  float knobY = knobHeight + labelY + 2.0f * margin;
  float sliderWidth = 70.0f;
  float sliderHeight = int(2.0f * (knobHeight + labelY) + 67.5f);
  float sliderX = 80.0f;
  float sliderY = sliderHeight + labelY;
  float checkboxWidth = 80.0f;
  uint32_t defaultWidth = uint32_t(40 + sliderX + 7 * knobX + 6 * margin);
  uint32_t defaultHeight = uint32_t(20 + 3 * knobY + 3 * labelHeight + 3 * margin);

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
    knobRightMargin = 0;
    knobX = knobWidth; // With margin.
    knobY = knobHeight + labelY + 2 * margin;
    sliderWidth = int(sc * 70);
    sliderHeight = int(2 * (knobHeight + labelY) + sc * 67.5);
    sliderX = int(sc * 80);
    sliderY = sliderHeight + labelY;
    checkboxWidth = sliderX;
    defaultWidth = uint32_t(2 * uiMargin + sliderX + 7 * knobX + 6 * margin);
    defaultHeight = uint32_t(uiMargin + 3 * knobY + 3 * labelHeight + 3 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
