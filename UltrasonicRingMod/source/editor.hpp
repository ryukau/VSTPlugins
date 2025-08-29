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
  float knobWidth = 100.0f;
  float knobX = knobWidth + 2 * margin;
  float knobY = knobWidth + labelHeight + 2 * margin;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = knobWidth;
  float labelX = labelWidth + margin;
  float splashWidth = int(1.5 * labelWidth) + margin;
  float splashHeight = labelY;

  float barboxWidth = 500.0f;
  float barboxHeight = 160.0f;

  int32_t defaultWidth = int32_t(2 * uiMargin + 4 * knobX - 2 * margin);
  int32_t defaultHeight = int32_t(2 * uiMargin + 2 * knobY + 4 * labelY - 2 * margin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    knobWidth = int(sc * 100);
    knobX = knobWidth + 2 * margin;
    knobY = knobWidth + labelHeight + 2 * margin;
    labelY = labelHeight + 2 * margin;
    labelWidth = knobWidth;
    labelX = labelWidth + margin;
    splashWidth = int(1.5 * labelWidth) + margin;
    splashHeight = labelY;

    barboxWidth = int(sc * 500);
    barboxHeight = int(sc * 160);

    defaultWidth = int32_t(2 * uiMargin + 4 * knobX - 2 * margin);
    defaultHeight = int32_t(2 * uiMargin + 2 * knobY + 4 * labelY - 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
