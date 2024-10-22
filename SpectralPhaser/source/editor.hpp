// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

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
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(CCoord left, CCoord top, CCoord width, CCoord height, ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setArcWidth(2.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }

  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
