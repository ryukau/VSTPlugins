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
  float splashWidth = 2 * labelX;
  float splashHeight = 30.0f;

  float barboxWidth = 320.0f;
  float barboxHeight = 160.0f;

  int32_t defaultWidth = int32_t(2 * uiMargin + 2 * barboxWidth + 4 * margin);
  int32_t defaultHeight
    = int32_t(2 * uiMargin + 2 * barboxHeight + 4 * margin + 4 * labelY + splashHeight);

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
    splashWidth = 2 * labelX;
    splashHeight = int(sc * 30);

    barboxWidth = int(sc * 320);
    barboxHeight = int(sc * 160);

    defaultWidth = int32_t(2 * uiMargin + 2 * barboxWidth + 4 * margin);
    defaultHeight
      = int32_t(2 * uiMargin + 2 * barboxHeight + 4 * margin + 4 * labelY + splashHeight);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  SharedPointer<TextKnob<decltype(Synth::Scales::seed), Uhhyou::Style::warning>>
    seedTextKnob;

  ParamValue getPlainValue(ParamID id);
  void refreshSeed(ParamID id);

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
