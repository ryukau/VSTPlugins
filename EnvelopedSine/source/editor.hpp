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
  float pluginNameTextSize = 18.0f;
  float margin = 5.0f;
  float uiMargin = 4 * margin;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f; // With margin.
  float knobY = knobHeight + labelY;
  float barboxWidth = 704; // Approx. 12.0f * knobX;
  float barboxHeight = 2.0f * knobY;
  float barboxY = barboxHeight + 2.0f * margin;
  float checkboxWidth = 60.0f;
  float splashHeight = 40.0f;
  uint32_t defaultWidth = uint32_t(barboxWidth + 2 * knobX + labelY + 4 * margin + 40);
  uint32_t defaultHeight = uint32_t(40 + 2 * labelY + knobY + 4 * barboxY + 2 * margin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 18);
    margin = int(sc * 5);
    uiMargin = 4 * margin;
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    barboxWidth = int(sc * 704); // Approx. 12 * knobX;
    barboxHeight = 2 * knobY;
    barboxY = barboxHeight + 2 * margin;
    checkboxWidth = int(sc * 60);
    splashHeight = int(sc * 40);

    defaultWidth = uint32_t(barboxWidth + 2 * knobX + labelY + 4 * margin + 2 * uiMargin);
    defaultHeight
      = uint32_t(2 * labelY + knobY + 4 * barboxY + 2 * margin + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
