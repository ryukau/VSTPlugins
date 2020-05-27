// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

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
    CFontRef fontId,
    Uhhyou::Palette &palette)
    : CControl(size, listener, tag), label(label), fontId(fontId), pal(palette)
  {
    this->fontId->remember();
  }

  ~ButtonBase()
  {
    if (fontId != nullptr) fontId->forget();
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
    pContext->setLineWidth(isMouseEntered ? 2 * borderWidth : borderWidth);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText, true);
  }

  virtual CMouseEventResult
  onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  virtual CMouseEventResult
  onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  virtual CMouseEventResult
  onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (!buttons.isLeftButton()) return kMouseEventNotHandled;
    value = value == 0 ? 1 : 0;
    valueChanged();
    invalid();
    return kMouseEventHandled;
  }

  virtual CMouseEventResult onMouseCancel() override
  {
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

protected:
  CFontRef fontId = nullptr;
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
    CFontRef fontId,
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
    CFontRef fontId,
    Uhhyou::Palette &palette)
    : ButtonBase<style>(size, listener, tag, label, fontId, palette)
  {
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
    }
    Btn::isMouseEntered = false;
    Btn::invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (!buttons.isLeftButton()) return kMouseEventNotHandled;
    Btn::value = 1;
    Btn::valueChanged();
    Btn::invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
      Btn::invalid();
    }
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    if (Btn::value == 1) {
      Btn::value = 0;
      Btn::valueChanged();
    }
    Btn::isMouseEntered = false;
    Btn::invalid();
    return kMouseEventHandled;
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
    CFontRef fontId,
    Uhhyou::Palette &palette)
    : CControl(size, nullptr, -1)
    , label(label)
    , messageID(messageID)
    , controller(controller)
    , fontId(fontId)
    , pal(palette)
  {
    if (controller != nullptr) controller->addRef();
    this->fontId->remember();
  }

  ~MessageButton()
  {
    if (controller != nullptr) controller->release();
    if (fontId != nullptr) fontId->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border and background.
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(isMouseEntered ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(isMouseEntered ? 2 * borderWidth : borderWidth);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText, true);
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    isPressed = false;
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (!buttons.isLeftButton()) return kMouseEventNotHandled;

    isPressed = true;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (isPressed) {
      controller->sendTextMessage(messageID.c_str());
      isPressed = false;
    }
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    isPressed = false;
    isMouseEntered = false;
    return kMouseEventHandled;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(MessageButton, CControl);

protected:
  Steinberg::Vst::EditController *controller = nullptr;

  CFontRef fontId = nullptr;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 2.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
