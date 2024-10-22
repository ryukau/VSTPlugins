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
