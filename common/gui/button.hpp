// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "style.hpp"

#include <sstream>
#include <string>

namespace VSTGUI {

template<Uhhyou::Style style = Uhhyou::Style::common> class ButtonBase : public CControl {
public:
  std::string label;

  ButtonBase(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener, tag), label(label), fontId(fontId), pal(palette)
  {
  }

  CLASS_METHODS(ButtonBase, CControl);

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border and background.
    if constexpr (style == Uhhyou::Style::accent) {
      pContext->setFillColor(value ? pal.highlightAccent() : pal.boxBackground());
      pContext->setFrameColor(isMouseEntered ? pal.highlightAccent() : pal.border());
    } else if (style == Uhhyou::Style::warning) {
      pContext->setFillColor(value ? pal.highlightWarning() : pal.boxBackground());
      pContext->setFrameColor(isMouseEntered ? pal.highlightWarning() : pal.border());
    } else {
      pContext->setFillColor(value ? pal.highlightButton() : pal.boxBackground());
      pContext->setFrameColor(isMouseEntered ? pal.highlightButton() : pal.border());
    }

    auto borderW = int(pal.guiScale() * borderWidth * (isMouseEntered ? 2 : 1));
    borderW += 1 ^ (borderW % 2); // Always odd number.
    pContext->setLineWidth(borderW);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText, true);
  }

  virtual void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  virtual void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  virtual void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;
    value = value == 0 ? 1 : 0;
    valueChanged();
    invalid();
    event.consumed = true;
  }

  virtual void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

protected:
  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isMouseEntered = false;
};

template<Uhhyou::Style style = Uhhyou::Style::common>
class ToggleButton : public ButtonBase<style> {
public:
  ToggleButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : ButtonBase<style>(size, listener, tag, label, fontId, palette)
  {
  }
};

template<Uhhyou::Style style = Uhhyou::Style::common>
class KickButton : public ButtonBase<style> {
public:
  using Btn = ButtonBase<style>;

  KickButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : ButtonBase<style>(size, listener, tag, label, fontId, palette)
  {
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
    }
    Btn::isMouseEntered = false;
    Btn::invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;
    Btn::value = 1;
    Btn::valueChanged();
    Btn::invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
      Btn::invalid();
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
    }
    Btn::isMouseEntered = false;
    Btn::invalid();
    event.consumed = true;
  }
};

class MessageButton : public CControl {
public:
  std::string label;
  std::string messageID;

  MessageButton(
    Steinberg::Vst::EditController *controller,
    const CRect &size,
    std::string label,
    std::string messageID,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette)
    : CControl(size, nullptr, -1)
    , label(label)
    , messageID(messageID)
    , controller(controller)
    , fontId(fontId)
    , pal(palette)
  {
    if (controller != nullptr) controller->addRef();
  }

  virtual ~MessageButton()
  {
    if (controller != nullptr) controller->release();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border and background.
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(isMouseEntered ? pal.highlightButton() : pal.border());
    auto borderW = int(pal.guiScale() * borderWidth);
    borderW += 1 ^ (borderW % 2); // Always odd number.
    pContext->setLineWidth(isMouseEntered ? 2 * borderW : borderW);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText, true);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isPressed = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!event.buttonState.isLeft()) return;

    isPressed = true;
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isPressed) {
      controller->sendTextMessage(messageID.c_str());
      isPressed = false;
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    isPressed = false;
    isMouseEntered = false;
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(MessageButton, CControl);

protected:
  Steinberg::Vst::EditController *controller = nullptr;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 2.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
