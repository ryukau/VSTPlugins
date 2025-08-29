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
  ~Editor();

  virtual void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  float uiTextSize = 12.0f;
  float midTextSize = 12.0f;
  float pluginNameTextSize = 14.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float splashHeight = 30.0f;

  float limiterLabelWidth = 100.0f;
  float checkboxWidth = 2.0f * limiterLabelWidth;

  uint32_t defaultWidth = uint32_t(2 * uiMargin + 2 * limiterLabelWidth);
  uint32_t defaultHeight = uint32_t(2 * uiMargin + 8 * labelY + splashHeight);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = sc * 12;
    midTextSize = sc * 12;
    pluginNameTextSize = sc * 14;
    margin = sc * 5;
    uiMargin = sc * 20;
    labelHeight = sc * 20;
    labelY = sc * 30;
    splashHeight = sc * 30;

    limiterLabelWidth = sc * 100;
    checkboxWidth = 2 * limiterLabelWidth;

    defaultWidth = uint32_t(2 * uiMargin + 2 * limiterLabelWidth);
    defaultHeight = uint32_t(2 * uiMargin + 8 * labelY + splashHeight);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  TextTableView *infoTextView = nullptr;
  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
