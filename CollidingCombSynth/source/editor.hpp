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
  float uiMargin = 20.0f;
  float margin = 5.0f;
  float labelHeight = 20.0f;
  float splashHeight = 40.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f; // With margin.
  float knobY = knobHeight + labelY;
  float barboxHeight = 3.0f * knobY - 2 * margin;
  float innerWidth = 12 * knobX + 6 * margin;
  float innerHeight = 15 * labelY + 2 * knobY + 14 * margin;
  uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
  uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 18);
    uiMargin = int(sc * 20);
    margin = int(sc * 5);
    labelHeight = int(sc * 20);
    splashHeight = int(sc * 40);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    barboxHeight = 3 * knobY - 2 * margin;
    innerWidth = 12 * knobX + 6 * margin;
    innerHeight = 15 * labelY + 2 * knobY + 14 * margin;
    defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
    defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
