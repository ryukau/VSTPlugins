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
  float midTextSize = 14.0f;
  float pluginNameTextSize = 18.0f;
  float uiMargin = 20.0f;
  float margin = 5.0f;
  float checkBoxWidth = 10.0f;
  float labelWidth = 80.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float splashHeight = 40.0f;
  float barboxWidth = int(64) * int(1 + (7 * labelWidth + 6 * margin) / 64);
  float barboxHeight = 8 * labelHeight + 14 * margin;
  float lfoWidthFix = 25.0f;
  float innerWidth = 6 * labelWidth + 10 * margin + barboxWidth;
  float innerHeight = 8 * labelY + 2 * barboxHeight + 9 * margin;
  uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
  uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

  void setDimensions() override
  {
    const auto sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 14);
    pluginNameTextSize = int(sc * 18);
    uiMargin = int(sc * 20);
    margin = int(sc * 5);
    checkBoxWidth = int(sc * 10);
    labelWidth = int(sc * 80);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    splashHeight = int(sc * 40);
    barboxWidth = int(64) * int(1 + (7 * labelWidth + 6 * margin) / 64);
    barboxHeight = 8 * labelHeight + 14 * margin;
    lfoWidthFix = int(sc * 25);
    innerWidth = 6 * labelWidth + 10 * margin + barboxWidth;
    innerHeight = 8 * labelY + 2 * barboxHeight + 9 * margin;
    defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
    defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
