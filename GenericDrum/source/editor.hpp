// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "parameter.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  virtual void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  float uiTextSize = 12.0f;
  float pluginNameTextSize = 16.0f;
  float margin = 5.0f;
  float uiMargin = 20.0f;
  float labelHeight = 20.0f;
  float knobWidth = 80.0f;
  float knobX = knobWidth + 2 * margin;
  float knobY = knobWidth + labelHeight + 2 * margin;
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 2 * knobWidth;
  float groupLabelWidth = 2 * labelWidth + 2 * margin;
  float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
  float splashHeight = int(2 * labelHeight + 2 * margin);

  float barBoxWidth = groupLabelWidth;
  float barBoxHeight = 5 * labelY - 2 * margin;
  float smallKnobWidth = labelHeight;
  float smallKnobX = smallKnobWidth + 2 * margin;

  float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
  float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

  int32_t defaultWidth = int32_t(4 * uiMargin + 3 * groupLabelWidth);
  int32_t defaultHeight = int32_t(2 * uiMargin + 21 * labelY - 2 * margin);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 16);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    knobWidth = int(sc * 80);
    knobX = knobWidth + 2 * margin;
    knobY = knobWidth + labelHeight + 2 * margin;
    labelY = labelHeight + 2 * margin;
    labelWidth = 2 * knobWidth;
    groupLabelWidth = 2 * labelWidth + 2 * margin;
    splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
    splashHeight = int(2 * labelHeight + 2 * margin);

    barBoxWidth = groupLabelWidth;
    barBoxHeight = 5 * labelY - 2 * margin;
    smallKnobWidth = labelHeight;
    smallKnobX = smallKnobWidth + 2 * margin;

    tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
    tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

    defaultWidth = int32_t(4 * uiMargin + 3 * groupLabelWidth);
    defaultHeight = int32_t(2 * uiMargin + 21 * labelY - 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;

  size_t extInPeakHoldCounter = 0;
  double extInPeakDecibel = 0;
  SharedPointer<Label> labelExternalInputAmplitude;
  SharedPointer<Label> labelWireCollision;
  SharedPointer<Label> labelMembraneCollision;
};

} // namespace Vst
} // namespace Steinberg
