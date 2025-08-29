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
  float labelY = labelHeight + 2 * margin;
  float labelWidth = 160.0f;
  float labelWidthHalf = 80.0f;
  float groupLabelWidth = 2 * labelWidth + 2 * margin;

  int32_t defaultWidth = int32_t(4 * uiMargin + 3 * groupLabelWidth);
  int32_t defaultHeight = int32_t(uiMargin + 18 * labelY);

  void setDimensions() override
  {
    const float sc = palette.guiScale();

    uiTextSize = int(sc * 12);
    pluginNameTextSize = int(sc * 16);
    margin = int(sc * 5);
    uiMargin = int(sc * 20);
    labelHeight = int(sc * 20);
    labelY = labelHeight + 2 * margin;
    labelWidth = int(sc * 160);
    labelWidthHalf = int(sc * 80);
    groupLabelWidth = 2 * labelWidth + 2 * margin;

    defaultWidth = int32_t(4 * uiMargin + 3 * groupLabelWidth);
    defaultHeight = int32_t(uiMargin + 18 * labelY);

    viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
    setRect(viewRect);
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setArcWidth(int(palette.guiScale() * 2));
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto offset = height + 2 * margin;
    auto label = addLabel(
      left + offset, top, width - offset, height, textSize, name,
      CHoriTxtAlign::kLeftText);
    return std::make_tuple(knob, label);
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
