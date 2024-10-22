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

  void addTpzLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name);

  void addSplashScreenTpz(
    CCoord buttonLeft,
    CCoord buttonTop,
    CCoord buttonWidth,
    CCoord buttonHeight,
    CCoord splashLeft,
    CCoord splashTop,
    CCoord splashWidth,
    CCoord splashHeight,
    CCoord textSize,
    const char *pluginName);
};

} // namespace Vst
} // namespace Steinberg
