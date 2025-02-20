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
  DELEGATE_REFCOUNT(VSTGUIEditor);

protected:
  bool prepareUI() override;

  void addOscillatorSection(
    std::string label,
    double left,
    double top,
    ParamID tagGain,
    ParamID tagSemi,
    ParamID tagCent,
    ParamID tagSync,
    ParamID tagSyncType,
    ParamID tagPTROrder,
    ParamID tagPhase,
    ParamID tagPhaseLock);
};

} // namespace Vst
} // namespace Steinberg
