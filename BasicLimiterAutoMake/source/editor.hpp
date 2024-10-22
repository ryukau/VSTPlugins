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
  ~Editor();

  virtual void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  TextTableView *infoTextView = nullptr;
  ParamValue getPlainValue(ParamID id);
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
