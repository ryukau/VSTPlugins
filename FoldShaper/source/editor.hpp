// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/curveview.hpp"
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
  float labelHeight = 20.0f;
  float labelY = 30.0f;
  float knobWidth = 50.0f;
  float knobHeight = 40.0f;
  float knobX = 60.0f;
  float knobY = knobHeight + labelY;
  float checkboxWidth = 90.0f;
  float splashHeight = 20.0f;

  float limiterLabelWidth = knobX + 3 * margin;

  float indicatorHeight = 3 * labelY - 2 * margin;
  float indicatorWidth = float(int(1.5f * knobX)) + checkboxWidth;
  float infoTextSize = 12.0f;
  float infoTextCellWidth = 100.0f;

  uint32_t defaultWidth = uint32_t(4 * knobX + checkboxWidth + 2 * margin + 30);
  uint32_t defaultHeight = uint32_t(knobY + 4 * margin + 3 * labelY + splashHeight + 30);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    midTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 14);
    margin = int(sc * 5);
    labelHeight = int(sc * 20);
    labelY = int(sc * 30);
    knobWidth = int(sc * 50);
    knobHeight = int(sc * 40);
    knobX = int(sc * 60);
    knobY = knobHeight + labelY;
    checkboxWidth = int(sc * 90);
    splashHeight = int(sc * 20);

    limiterLabelWidth = knobX + 3 * margin;

    indicatorHeight = 3 * labelY - 2 * margin;
    indicatorWidth = int(1.5 * knobX + checkboxWidth);
    infoTextSize = int(sc * 12);
    infoTextCellWidth = int(sc * 100);

    defaultWidth = uint32_t(4 * knobX + checkboxWidth + 8 * margin);
    defaultHeight = uint32_t(knobY + 3 * labelY + splashHeight + 10 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  CurveView<decltype(Synth::Scales::guiInputGainScale)> *curveView = nullptr;
  TextTableView *infoTextView = nullptr;

  ParamValue getPlainValue(ParamID id);
  CurveView<decltype(Synth::Scales::guiInputGainScale)> *addCurveView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID tag,
    TextTableView *textView);
  void refreshCurveView(ParamID id);

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
