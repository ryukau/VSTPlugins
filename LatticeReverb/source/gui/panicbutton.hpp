// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "../../../common/gui/plugeditor.hpp"
#include "../../../common/gui/style.hpp"
#include "../parameter.hpp"

#include <string>

namespace VSTGUI {

class PanicButton : public CControl {
public:
  std::string label;

  PanicButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    CFontRef fontId,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor)
    : CControl(size, listener, tag)
    , label(label)
    , fontId(fontId)
    , pal(palette)
    , editor(editor)
  {
    this->fontId->remember();
    if (editor) editor->remember();
  }

  ~PanicButton()
  {
    if (fontId) fontId->forget();
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !isPressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(isMouseEntered ? 2 * borderWidth : borderWidth);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    if (value == 1.0f) {
      value = 0.0f;
    }
    isPressed = false;
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    if (!buttons.isLeftButton()) return kMouseEventNotHandled;
    isPressed = true;
    value = 1.0f;

    if (editor) {
      editor->valueChanged(ID::timeMultiply, 0.0f);
      editor->valueChanged(ID::innerFeedMultiply, 0.0f);
      editor->valueChanged(ID::outerFeedMultiply, 0.0f);

      editor->updateUI(ID::timeMultiply, 0.0f);
      editor->updateUI(ID::innerFeedMultiply, 0.0f);
      editor->updateUI(ID::outerFeedMultiply, 0.0f);
    }

    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      invalid();
    }
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    if (isPressed) {
      isPressed = false;
      value = 0;
      invalid();
    }
    isMouseEntered = false;
    return kMouseEventHandled;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(PanicButton, CControl);

protected:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  CColor colorFore{0, 0, 0};
  CColor colorBack{0xff, 0xff, 0xff};
  CColor colorFocus{0x33, 0xee, 0xee};

  CFontRef fontId = nullptr;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
