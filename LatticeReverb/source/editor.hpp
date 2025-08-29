// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/panicbutton.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);
  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiMargin = 20.0f;
  float uiTextSize = 12.0f;
  float midTextSize = 12.0f;
  float pluginNameTextSize = 18.0f;
  float margin = 5.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f; // With margin.
  float knobY = knobHeight + labelY;
  float textKnobX = 80.0f;
  float splashHeight = 40.0f;
  float barboxWidth = 4 * textKnobX;
  float barboxHeight = 2 * knobY;

  float tabViewWidth = barboxWidth + labelY + 2 * uiMargin;
  float tabViewHeight = labelY + 3 * barboxHeight + 2 * labelHeight + 2 * uiMargin;

  float leftPanelWidth = 4 * knobX + 6 * margin + labelHeight;

  uint32_t defaultWidth = uint32_t(leftPanelWidth + labelY + tabViewWidth + 2 * uiMargin);
  uint32_t defaultHeight = uint32_t(tabViewHeight + 2 * uiMargin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiMargin = int(sc * 20);
    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 18);
    margin = int(sc * 5);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60); // With margin.
    knobY = knobHeight + labelY;
    textKnobX = int(sc * 80);
    splashHeight = int(sc * 40);
    barboxWidth = 4 * textKnobX;
    barboxHeight = 2 * knobY;

    tabViewWidth = barboxWidth + labelY + 2 * uiMargin;
    tabViewHeight = labelY + 3 * barboxHeight + 2 * labelHeight + 2 * uiMargin;

    leftPanelWidth = 4 * knobX + 6 * margin + labelHeight;

    defaultWidth = uint32_t(leftPanelWidth + labelY + tabViewWidth + 2 * uiMargin);
    defaultHeight = uint32_t(tabViewHeight + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
