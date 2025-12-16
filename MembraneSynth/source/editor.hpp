// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/randomizebutton.hpp"
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
  float uiTextSize{};
  float pluginNameTextSize{};
  float margin{};
  float uiMargin{};
  float labelHeight{};
  float knobWidth{};
  float knobX{};
  float knobY{};
  float labelY{};
  float labelWidth{};
  float groupLabelWidth{};
  float splashWidth{};
  float splashHeight{};

  float barboxWidth{};
  float barboxHeight{};
  float smallKnobWidth{};
  float smallKnobX{};

  int32_t defaultWidth{};
  int32_t defaultHeight{};

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
    groupLabelWidth = 2 * labelWidth + labelHeight + 4 * margin;
    splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
    splashHeight = int(labelHeight * 3 / 2);

    barboxWidth = groupLabelWidth;
    barboxHeight = 5 * labelY - 2 * margin;
    smallKnobWidth = labelHeight;
    smallKnobX = smallKnobWidth + 2 * margin;

    defaultWidth = int32_t(2 * uiMargin + 3 * groupLabelWidth + 4 * margin);
    defaultHeight = int32_t(2 * uiMargin + 15 * labelY - 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;

  void addRandomButton(
    CCoord left,
    CCoord top,
    CCoord sideLength,
    std::vector<Steinberg::Vst::ParamID> targetId)
  {
    auto button = new RandomizeButton(
      CRect(left, top, left + sideLength, top + sideLength), this, 0, palette, this,
      targetId);
    frame->addView(button);
  }
};

} // namespace Vst
} // namespace Steinberg
