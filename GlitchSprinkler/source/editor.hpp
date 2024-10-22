// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/polynomialxypad.hpp"
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

  virtual void updateUI(Vst::ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  bool prepareUI() override;

  SharedPointer<PolynomialXYPad> polyXYPad;
  SharedPointer<Label> polyXLabel;
  SharedPointer<Label> polyYLabel;
  SharedPointer<CControl> polyXControl;
  SharedPointer<CControl> polyYControl;
};

} // namespace Vst
} // namespace Steinberg
