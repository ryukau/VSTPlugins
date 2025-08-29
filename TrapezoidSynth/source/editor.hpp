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
  float margin = 5.0f;
  float uiMargin = 4 * margin;
  float groupLabelMargin = 10.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = knobWidth - 2.0f * margin;
  float knobX = knobWidth; // With margin.
  float knobY = knobHeight + labelY + 2.0f * margin;
  float sliderWidth = 70.0f;
  float sliderHeight = 2.0f * (knobHeight + labelY) + 67.5f;
  float sliderX = 80.0f;
  float sliderY = sliderHeight + labelY;
  float checkboxWidth = 80.0f;
  uint32_t defaultWidth = uint32_t(40 + 13 * knobX);
  uint32_t defaultHeight = uint32_t(20 + 5 * (knobY + labelHeight));

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 14);
    pluginNameTextSize = int(sc * 18);
    margin = int(sc * 5);
    uiMargin = 4 * margin;
    groupLabelMargin = int(sc * 10);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = knobWidth - 2 * margin;
    knobX = knobWidth; // With margin.
    knobY = knobHeight + labelY + 2 * margin;
    sliderWidth = int(sc * 70);
    sliderHeight = 2 * (knobHeight + labelY) + int(sc * 67.5);
    sliderX = int(sc * 80);
    sliderY = sliderHeight + labelY;
    checkboxWidth = int(sc * 80);
    defaultWidth = uint32_t(2 * uiMargin + 13 * knobX);
    defaultHeight = uint32_t(uiMargin + 5 * (knobY + labelHeight));

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;

  void addTpzLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name);

  void addSplashScreenTpz(
    CCoord buttonLeft,
    CCoord buttonTop,
    CCoord buttonWidth,
    CCoord buttonHeight,
    CCoord splashLeft,
    CCoord splashTop,
    CCoord splashWidth,
    CCoord splashHeight,
    CCoord textSize,
    const char *pluginName);
};

} // namespace Vst
} // namespace Steinberg
