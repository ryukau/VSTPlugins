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

  void valueChanged(CControl *pControl) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 14.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 75.0f;
  float labelX = labelWidth + margin;
  float splashWidth = 2 * labelWidth + margin;
  float splashHeight = 30.0f;

  float barboxWidth = 500.0f;
  float barboxHeight = 160.0f;

  int32_t defaultWidth = int32_t(2 * uiMargin + barboxWidth);
  int32_t defaultHeight = int32_t(2 * uiMargin + barboxHeight + 2 * margin + 7 * labelY);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    labelY = labelHeight + 2 * margin;
    labelWidth = int(sc * 75);
    labelX = labelWidth + margin;
    splashWidth = 2 * labelWidth + margin;
    splashHeight = int(sc * 30);

    barboxWidth = int(sc * 500);
    barboxHeight = int(sc * 160);

    defaultWidth = int32_t(2 * uiMargin + barboxWidth);
    defaultHeight = int32_t(2 * uiMargin + barboxHeight + 2 * margin + 7 * labelY);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
