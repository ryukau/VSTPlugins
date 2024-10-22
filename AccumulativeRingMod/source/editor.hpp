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
