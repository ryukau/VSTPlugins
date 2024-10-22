// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "gui/curveview.hpp"
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

  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  CurveView<decltype(Synth::Scales::guiInputGainScale)> *curveView = nullptr;
  TextTableView *infoTextView = nullptr;

  ParamValue getPlainValue(ParamID id);
  CurveView<decltype(Synth::Scales::guiInputGainScale)> *addCurveView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID tag,
    TextTableView *textView);
  void refreshCurveView(ParamID id);

  bool prepareUI() override;
};

} // namespace Vst
} // namespace Steinberg
