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

  virtual void valueChanged(CControl *pControl) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
