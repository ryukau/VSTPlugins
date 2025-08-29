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
  float pluginNameTextSize = 24.0f;
  float margin = 5.0f;
  float uiMargin = 4 * margin;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f;
  float knobY = knobHeight + labelY;
  float checkboxWidth = 60.0f;
  uint32_t defaultWidth = uint32_t(10.0 * knobX + 50.0);
  uint32_t defaultHeight = uint32_t(20.0 + 3.0 * labelY + 6.0 * knobY);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 24);
    margin = int(sc * 5);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    checkboxWidth = int(sc * 60);
    defaultWidth = uint32_t(10 * knobX + 2 * margin + 2 * uiMargin);
    defaultHeight = uint32_t(3 * labelY + 6 * knobY + 2 * margin + uiMargin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;

  void addOscillatorSection(
    std::string label,
    double left,
    double top,
    ParamID tagGain,
    ParamID tagSemi,
    ParamID tagCent,
    ParamID tagSync,
    ParamID tagSyncType,
    ParamID tagPTROrder,
    ParamID tagPhase,
    ParamID tagPhaseLock);
};

} // namespace Vst
} // namespace Steinberg
