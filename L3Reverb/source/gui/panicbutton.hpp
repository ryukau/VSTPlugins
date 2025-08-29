// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

  ~PanicButton()
  {
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    const auto sc = pal.guiScale();
    const double borderW = int(sc * (isMouseEntered ? 2 * borderWidth : borderWidth));
    const double halfBorderWidth = int(borderW / 2);
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !isPressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(
      CRect(halfBorderWidth, halfBorderWidth, getWidth(), getHeight()),
      kDrawFilledAndStroked);

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
    if (value == 1.0f) {
      value = 0.0f;
    }
    isPressed = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    if (!event.buttonState.isLeft()) return;
    isPressed = true;
    value = 1.0f;

    if (editor) {
      editor->valueChanged(ID::timeMultiply, 0.0f);
      editor->valueChanged(ID::innerFeedMultiply, 0.0f);
      editor->valueChanged(ID::d1FeedMultiply, 0.0f);
      editor->valueChanged(ID::d2FeedMultiply, 0.0f);
      editor->valueChanged(ID::d3FeedMultiply, 0.0f);
      editor->valueChanged(ID::d4FeedMultiply, 0.0f);

      editor->updateUI(ID::timeMultiply, 0.0f);
      editor->updateUI(ID::innerFeedMultiply, 0.0f);
      editor->updateUI(ID::d1FeedMultiply, 0.0f);
      editor->updateUI(ID::d2FeedMultiply, 0.0f);
      editor->updateUI(ID::d3FeedMultiply, 0.0f);
      editor->updateUI(ID::d4FeedMultiply, 0.0f);
    }

    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      invalid();
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0;
      invalid();
    }
    isMouseEntered = false;
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(PanicButton, CControl);

protected:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  CColor colorFore{0, 0, 0};
  CColor colorBack{0xff, 0xff, 0xff};
  CColor colorFocus{0x33, 0xee, 0xee};

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
