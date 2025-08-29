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
  void updateUI(ParamID id, ParamValue normalized) override;

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
  int32_t defaultHeight
    = int32_t(2 * uiMargin + 9 * labelY + 2 * labelWidth + 2 * margin);

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
    defaultHeight = int32_t(2 * uiMargin + 9 * labelY + 2 * labelWidth + 2 * margin);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  struct XYPadAxis {
    size_t index = 0;
    SharedPointer<XYPad> xypad;
    SharedPointer<CControl> control;

    XYPadAxis(size_t index, XYPad *xypad, CControl *control)
      : xypad(xypad), control(control), index(std::min(index, size_t(1)))
    {
    }

    void sync(float normalized)
    {
      xypad->setValueAt(xypad->id[index], normalized);
      xypad->invalid();
      control->setValue(normalized);
    }
  };

  std::unordered_map<ParamID, std::shared_ptr<XYPadAxis>> xyControlMap;
  void syncUI(ParamID id, float normalized);

  bool prepareUI() override;

  template<typename Scale>
  void addAsymXYControls(
    CCoord left0,
    CCoord top0,
    CCoord halfWidth,
    CCoord labelHeight,
    CCoord margin,
    CCoord textSize,
    ParamID idX,
    ParamID idY,
    ParamID idHarshX,
    ParamID idHarshY,
    Scale &scale)
  {
    using Scales = Synth::Scales;

    const auto quarterWidth = int(halfWidth / 2);
    const auto padSize = 2 * halfWidth + 2 * margin;

    const auto left1 = left0 + halfWidth + 2 * margin;
    const auto top1 = top0 + padSize + 2 * margin;
    const auto top2 = top1 + labelHeight + 2 * margin;

    auto xypad = addXYPad(left0, top0, padSize, padSize, idX, idY);
    addLabel(left0, top1, quarterWidth, labelHeight, textSize, "X Pre");
    addLabel(left0, top2, quarterWidth, labelHeight, textSize, "Y Post");

    addCheckbox(
      left0 + int(1.5 * quarterWidth) - 3 * margin, top1, quarterWidth, labelHeight,
      textSize, "", idHarshX);
    addCheckbox(
      left0 + int(1.5 * quarterWidth) - 3 * margin, top2, quarterWidth, labelHeight,
      textSize, "", idHarshY);

    auto xKnob
      = addTextKnob(left1, top1, halfWidth, labelHeight, textSize, idX, scale, false, 5);
    auto yKnob
      = addTextKnob(left1, top2, halfWidth, labelHeight, textSize, idY, scale, false, 5);

    xyControlMap.emplace(idX, std::make_shared<XYPadAxis>(0, xypad, xKnob));
    xyControlMap.emplace(idY, std::make_shared<XYPadAxis>(1, xypad, yKnob));
  }
};

} // namespace Vst
} // namespace Steinberg
