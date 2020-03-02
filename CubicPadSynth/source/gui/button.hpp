// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include <sstream>
#include <string>

namespace VSTGUI {

class KickButton : public CControl {
public:
  std::string label;

  KickButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    CFontRef fontID)
    : CControl(size, listener, tag), label(label), fontID(fontID)
  {
    this->fontID->remember();
  }

  ~KickButton()
  {
    if (fontID != nullptr) fontID->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    pContext->setFillColor(isPressed ? colorFocus : colorBack);
    pContext->setFrameColor(isMouseEntered && !isPressed ? colorFocus : colorFore);
    pContext->setLineWidth(isMouseEntered ? 2 * borderWidth : borderWidth);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontID);
    pContext->setFontColor(colorFore);
    pContext->drawString(
      UTF8String(label.c_str()), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setHighlightColor(CColor color) { colorFocus = color; }
  void setForegroundColor(CColor color) { colorFore = color; }
  void setBackgroundColor(CColor color) { colorBack = color; }
  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(KickButton, CControl);

protected:
  Steinberg::Vst::EditController *controller = nullptr;

  CColor colorFore{0, 0, 0};
  CColor colorBack{0xff, 0xff, 0xff};
  CColor colorFocus{0x33, 0xee, 0xee};

  CFontRef fontID = nullptr;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

CMouseEventResult KickButton::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isPressed = false;
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isLeftButton()) return kMouseEventNotHandled;
  isPressed = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isPressed) {
    value = 1;
    valueChanged();
    value = 0;

    isPressed = false;
    invalid();
  }
  return kMouseEventHandled;
}

CMouseEventResult KickButton::onMouseCancel()
{
  value = 0;
  isPressed = false;
  isMouseEntered = false;
  return kMouseEventHandled;
}

class MessageButton : public CControl {
public:
  std::string label;
  std::string messageID;

  MessageButton(
    Steinberg::Vst::EditController *controller,
    const CRect &size,
    std::string label,
    std::string messageID,
    CFontRef fontID)
    : CControl(size, nullptr, -1)
    , label(label)
    , messageID(messageID)
    , controller(controller)
    , fontID(fontID)
  {
    controller->addRef();
    this->fontID->remember();
  }

  ~MessageButton()
  {
    if (controller != nullptr) controller->release();
    if (fontID != nullptr) fontID->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    pContext->setFillColor(isPressed ? colorFocus : colorBack);
    pContext->setFrameColor(isMouseEntered && !isPressed ? colorFocus : colorFore);
    pContext->setLineWidth(isMouseEntered ? 2 * borderWidth : borderWidth);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontID);
    pContext->setFontColor(colorFore);
    pContext->drawString(
      UTF8String(label.c_str()), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setHighlightColor(CColor color) { colorFocus = color; }
  void setForegroundColor(CColor color) { colorFore = color; }
  void setBackgroundColor(CColor color) { colorBack = color; }
  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(MessageButton, CControl);

protected:
  Steinberg::Vst::EditController *controller = nullptr;

  CColor colorFore{0, 0, 0};
  CColor colorBack{0xff, 0xff, 0xff};
  CColor colorFocus{0x33, 0xee, 0xee};

  CFontRef fontID = nullptr;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

CMouseEventResult
MessageButton::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isPressed = false;
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isLeftButton()) return kMouseEventNotHandled;

  isPressed = true;
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (isPressed) {
    controller->sendTextMessage(messageID.c_str());
    isPressed = false;
  }
  return kMouseEventHandled;
}

CMouseEventResult MessageButton::onMouseCancel()
{
  isPressed = false;
  isMouseEntered = false;
  return kMouseEventHandled;
}

} // namespace VSTGUI
