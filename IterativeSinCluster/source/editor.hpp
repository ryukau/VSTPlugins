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
  float checkboxWidth = 60.0f;
  float splashHeight = 40.0f;
  uint32_t defaultWidth = uint32_t(12 * knobX + 4 * margin + 40);
  uint32_t defaultHeight
    = uint32_t(40 + 10 * labelY + 2 * knobY + 1 * knobHeight + 2 * margin);

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
    knobX = int(sc * 60); // With margin.
    knobY = knobHeight + labelY;
    checkboxWidth = int(sc * 60);
    splashHeight = int(sc * 40);
    defaultWidth = uint32_t(12 * knobX + 4 * margin + 2 * uiMargin);
    defaultHeight
      = uint32_t(10 * labelY + 2 * knobY + 1 * knobHeight + 2 * margin + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
