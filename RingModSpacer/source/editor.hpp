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

  virtual void valueChanged(CControl *pControl) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 14.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelWidth = 80.0f;
  float labelHeight = 20.0f;
  float labelY = labelHeight + 2 * margin;
  float halfLabelWidth = int(labelWidth / 2);

  int32_t defaultWidth = int32_t(2 * uiMargin + 6 * labelWidth + 14 * margin);
  int32_t defaultHeight = int32_t(2 * uiMargin + 6 * labelY - 2 * margin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelWidth = int(sc * 80);
    labelHeight = int(sc * 20);
    labelY = labelHeight + 2 * margin;
    halfLabelWidth = int(labelWidth / 2);

    defaultWidth = int32_t(2 * uiMargin + 6 * labelWidth + 14 * margin);
    defaultHeight = int32_t(2 * uiMargin + 6 * labelY - 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
