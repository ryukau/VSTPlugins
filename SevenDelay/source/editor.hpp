// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/waveview.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);
  ~Editor();

  void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;
  void refreshWaveView(Vst::ParamID id);
  void refreshTimeTextView(Vst::ParamID id);

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  WaveView *waveView = nullptr;
  TextView *timeTextView = nullptr;

  uint32_t defaultWidth = 960;
  uint32_t defaultHeight = 330;
  float pluginNameTextSize = 24.0f;
  float labelHeight = 30.0f;
  float midTextSize = 14.0f;
  float uiTextSize = 14.0f;
  float infoTextSize = 12.0f;
  float checkboxWidth = 80.0f;
  float sliderWidth = 70.0f;
  float sliderHeight = 230.0f;
  float margin = 0.0f;

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    defaultWidth = int(sc * 960);
    defaultHeight = int(sc * 330);
    pluginNameTextSize = int(sc * 24);
    labelHeight = int(sc * 30);
    midTextSize = int(sc * 14);
    uiTextSize = int(sc * 14);
    infoTextSize = int(sc * 12);
    checkboxWidth = int(sc * 80);
    sliderWidth = int(sc * 70);
    sliderHeight = int(sc * 230);
    margin = int(sc * 0);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  bool prepareUI() override;
  void addWaveView(const CRect &size);
  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
