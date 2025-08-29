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
  float pluginNameTextSize = 12.0f;
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
  float splashHeight = labelHeight;
  uint32_t defaultWidth = uint32_t(40 + 9.0f * knobX + labelY);
  uint32_t defaultHeight = uint32_t(40 + labelHeight + knobY);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 12);
    margin = int(sc * 5);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    barboxWidth = 12 * knobX;
    barboxHeight = 2 * knobY;
    barboxY = barboxHeight + 2 * margin;
    checkboxWidth = int(sc * 60);
    splashHeight = labelHeight;
    defaultWidth = uint32_t(40 + 9 * knobX + labelY);
    defaultHeight = uint32_t(40 + labelHeight + knobY);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
