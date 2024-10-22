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

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  SharedPointer<TextKnob<decltype(Synth::Scales::seed), Uhhyou::Style::warning>>
    seedTextKnob;

  ParamValue getPlainValue(ParamID id);
  void refreshSeed(ParamID id);

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
