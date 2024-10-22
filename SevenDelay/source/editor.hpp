// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/waveview.hpp"
#include "parameter.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);
  ~Editor();

  void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized) override;
  void refreshWaveView(Vst::ParamID id);
  void refreshTimeTextView(Vst::ParamID id);

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  WaveView *waveView = nullptr;
  TextView *timeTextView = nullptr;

  bool prepareUI() override;
  void addWaveView(const CRect &size);
  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
