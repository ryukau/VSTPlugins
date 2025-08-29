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
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float uiTextSize = 12.0f;
  float midTextSize = 12.0f;
  float pluginNameTextSize = 12.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float matrixKnobSize = 40.0f;
  float knobX = 60.0f;
  float knobY = knobHeight + labelY;
  float splashHeight = labelHeight;

  float halfKnobX = int(knobX / 2);

  float shiftMatrixWidth = nParallel * matrixKnobSize;
  float shiftMatrixHeight = nSerial * matrixKnobSize;
  float shiftBarBoxWidth = shiftMatrixWidth;
  float shiftBarBoxHeight = 2 * knobY;

  float innerWidth = shiftMatrixWidth + 8 * margin + 4 * knobX;
  float innerHeight = labelY + shiftMatrixHeight + 2 * shiftBarBoxHeight + 4 * margin;

  int32_t defaultWidth = int32_t(2 * uiMargin + innerWidth);
  int32_t defaultHeight = int32_t(2 * uiMargin + innerHeight);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 12);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    matrixKnobSize = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    splashHeight = labelHeight;

    halfKnobX = int(knobX / 2);

    shiftMatrixWidth = nParallel * matrixKnobSize;
    shiftMatrixHeight = nSerial * matrixKnobSize;
    shiftBarBoxWidth = shiftMatrixWidth;
    shiftBarBoxHeight = 2 * knobY;

    innerWidth = shiftMatrixWidth + 8 * margin + 4 * knobX;
    innerHeight = labelY + shiftMatrixHeight + 2 * shiftBarBoxHeight + 4 * margin;

    defaultWidth = int32_t(2 * uiMargin + innerWidth);
    defaultHeight = int32_t(2 * uiMargin + innerHeight);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
