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
    knob->setArcWidth(2.0);
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
