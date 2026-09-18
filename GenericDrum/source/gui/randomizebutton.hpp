// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "../../../common/gui/plugeditor.hpp"
#include "../../../common/gui/style.hpp"
#include "../parameter.hpp"

#include <algorithm>
#include <string>

namespace VSTGUI {

class RandomizeButton : public CControl {
public:
  std::string label;

  RandomizeButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor)
    : CControl(size, listener, tag)
    , label(label)
    , fontId(fontId)
    , pal(palette)
    , editor(editor)
  {
    if (editor) editor->remember();
  }

  ~RandomizeButton()
  {
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    const bool pressed = isPressed || getValueNormalized() > 0.5f;

    // Border.
    auto borderW = int(pal.guiScale() * borderWidth * (isMouseEntered ? 2 : 1));
    borderW += 1 ^ (borderW % 2); // Always odd number.
    pContext->setFillColor(pressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !pressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      if (editor) {
        editor->valueChanged(getTag(), 0.0);
        editor->updateUI(getTag(), 0.0);
      }
    }
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;
    isPressed = true;
    value = 1.0f;

    if (editor) {
      editor->valueChanged(getTag(), 1.0);
      editor->updateUI(getTag(), 1.0);
    }

    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      if (editor) {
        editor->valueChanged(getTag(), 0.0);
        editor->updateUI(getTag(), 0.0);
      }
      invalid();
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      if (editor) {
        editor->valueChanged(getTag(), 0.0);
        editor->updateUI(getTag(), 0.0);
      }
      invalid();
    }
    isMouseEntered = false;
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(RandomizeButton, CControl);

private:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
