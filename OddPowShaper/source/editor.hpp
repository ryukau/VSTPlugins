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
  float checkboxWidth = 60.0f;
  float splashHeight = 20.0f;
  float indicatorHeight = 120.0f;
  float indicatorWidth = 5 * knobX;
  float infoTextSize = 12.0f;
  float infoTextCellWidth = 100.0f;

  float limiterLabelWidth = knobX + 3 * margin;

  int32_t defaultWidth = int32_t(5 * knobX + 2 * limiterLabelWidth + 2 * margin + 30);
  int32_t defaultHeight = int32_t(30 + knobY + 2 * labelY + indicatorHeight);

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
    checkboxWidth = int(sc * 60);
    splashHeight = int(sc * 20);
    indicatorHeight = int(sc * 120);
    indicatorWidth = 5 * knobX;
    infoTextSize = int(sc * 12);
    infoTextCellWidth = int(sc * 100);

    limiterLabelWidth = knobX + 3 * margin;

    defaultWidth = int32_t(5 * knobX + 2 * limiterLabelWidth + 2 * margin + 6 * margin);
    defaultHeight = int32_t(knobY + 2 * labelY + indicatorHeight + 6 * margin);

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
