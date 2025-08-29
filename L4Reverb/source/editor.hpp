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
  float scrollBarHeight = 10.0f;
  float barboxWidth = 2 * nDepth1;
  float barboxHeight = 2 * knobY + labelY + scrollBarHeight;
  float barboxWidthSmall = 4 * nDepth2;

  float barboxSectionWidth = barboxWidth + barboxWidthSmall + 3 * labelY;
  float barboxSectionHeight = 3 * scrollBarHeight + 3 * barboxHeight + 2 * labelHeight;

  float leftPanelWidth = 4 * textKnobX + 6 * margin;

  uint32_t defaultWidth
    = uint32_t(leftPanelWidth + labelY + barboxSectionWidth + 2 * uiMargin);
  uint32_t defaultHeight = uint32_t(barboxSectionHeight + 2 * uiMargin);

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
    scrollBarHeight = int(sc * 10);
    barboxWidth = int(sc * 2 * nDepth1);
    barboxHeight = 2 * knobY + labelY + scrollBarHeight;
    barboxWidthSmall = int(sc * 4 * nDepth2);

    barboxSectionWidth = barboxWidth + barboxWidthSmall + 3 * labelY;
    barboxSectionHeight = 3 * scrollBarHeight + 3 * barboxHeight + 2 * labelHeight;

    leftPanelWidth = 4 * textKnobX + 6 * margin;

    defaultWidth = uint32_t(leftPanelWidth + labelY + barboxSectionWidth + 2 * uiMargin);
    defaultHeight = uint32_t(barboxSectionHeight + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
