// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "parameter.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 14.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float knobWidth = 50.0f;
  float knobX = knobWidth + 2 * margin;
  float knobY = knobWidth + labelHeight + 2 * margin;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 2 * knobWidth;
  float groupLabelWidth = 2 * labelWidth + 2 * margin;
  float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
  float splashHeight = int(labelHeight * 3 / 2);

  float barboxWidth = groupLabelWidth;
  float barboxHeight = 5 * labelY - 2 * margin;
  float smallKnobWidth = labelHeight;
  float smallKnobX = smallKnobWidth + 2 * margin;

  int32_t defaultWidth = int32_t(2 * uiMargin + 3 * groupLabelWidth + 8 * margin);
  int32_t defaultHeight = int32_t(2 * uiMargin + 15 * labelY - 2 * margin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    knobWidth = int(sc * 50);
    knobX = knobWidth + 2 * margin;
    knobY = knobWidth + labelHeight + 2 * margin;
    labelY = labelHeight + 2 * margin;
    labelWidth = 2 * knobWidth;
    groupLabelWidth = 2 * labelWidth + 2 * margin;
    splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
    splashHeight = int(labelHeight * 3 / 2);

    barboxWidth = groupLabelWidth;
    barboxHeight = 5 * labelY - 2 * margin;
    smallKnobWidth = labelHeight;
    smallKnobX = smallKnobWidth + 2 * margin;

    defaultWidth = int32_t(2 * uiMargin + 3 * groupLabelWidth + 8 * margin);
    defaultHeight = int32_t(2 * uiMargin + 15 * labelY - 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
