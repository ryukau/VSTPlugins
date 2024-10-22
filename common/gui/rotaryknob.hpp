// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <sstream>
#include <string>

namespace VSTGUI {

// Incremental encoder.
class RotaryKnobBase : public CControl {
public:
  bool liveUpdate = true;     // When false, only update value on mouse up event.
  double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  double lowSensitivity = sensitivity / 5.0;

  RotaryKnobBase(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : CControl(size, listener, tag), pal(palette)
  {
  }

  CLASS_METHODS(RotaryKnobBase, CControl);

  virtual void draw(CDrawContext *pContext) override {}

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
    beginEdit();
    isMouseDown = true;
    anchorPoint = event.mousePosition;
    event.consumed = true;
  }

  virtual void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (!liveUpdate) valueChanged();
    if (isMouseDown) endEdit();
    isMouseDown = false;
    event.consumed = true;
  }

  virtual void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    if (!isMouseDown) return;

    auto sensi = event.modifiers.is(ModifierKey::Shift) ? lowSensitivity : sensitivity;
    value += (float)((anchorPoint.y - event.mousePosition.y) * sensi);
    value = value > 1.0 || value < 0.0 ? value - floor(value) : value;
    bounceValue();

    if (liveUpdate && value != getOldValue()) valueChanged();
    if (isDirty()) invalid();

    anchorPoint = event.mousePosition;
    event.consumed = true;
  }

  virtual void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isMouseDown) {
      if (isDirty()) {
        valueChanged();
        invalid();
      }
      endEdit();
    }
    isMouseDown = false;
    isMouseEntered = false;
    event.consumed = true;
  }

  virtual void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    if (isEditing() || event.deltaY == 0) return;
    beginEdit();
    value += event.deltaY * float(sensitivity) * 0.5f;
    value -= floor(value);
    bounceValue();
    valueChanged();
    endEdit();
    invalid();
    event.consumed = true;
  }

protected:
  CPoint anchorPoint{0.0, 0.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  Uhhyou::Palette &pal;
};

// Incremental encoder.
template<Uhhyou::Style style = Uhhyou::Style::common>
class RotaryKnob : public RotaryKnobBase {
public:
  RotaryKnob(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : RotaryKnobBase(size, listener, tag, palette)
  {
  }

  CLASS_METHODS(RotaryKnob, CControl);

  virtual void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();
    const auto center = CPoint(width / 2.0, height / 2.0);

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft() + center));

    // Background.
    const double borderWidth = 2.0;
    pContext->setLineWidth(borderWidth);
    pContext->setFillColor(pal.background());
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

    // Arc.
    auto radius = center.x > center.y ? center.y : center.x;
    if constexpr (style == Uhhyou::Style::accent) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightAccent() : pal.unfocused());
    } else if (style == Uhhyou::Style::warning) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightWarning() : pal.unfocused());
    } else {
      pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.unfocused());
    }
    pContext->setLineStyle(lineStyle);
    pContext->setLineWidth(halfArcWidth * 2.0);
    pContext->drawEllipse(
      CRect(
        halfArcWidth - center.x, halfArcWidth - center.y, center.x - halfArcWidth,
        center.y - halfArcWidth),
      kDrawStroked);

    // Tick for default value. Sharing color and style with arc.
    auto tipLength = halfArcWidth - radius;
    pContext->setLineWidth(halfArcWidth / 2.0);
    pContext->drawLine(
      mapValueToArc(getDefaultValue() / getRange(), tipLength * defaultTickLength),
      mapValueToArc(getDefaultValue() / getRange(), tipLength));

    // Line from center to tip.
    auto tip = mapValueToArc(getValueNormalized(), tipLength);
    pContext->setFrameColor(pal.foreground());
    pContext->drawLine(CPoint(0.0, 0.0), tip);

    // Tip.
    pContext->setFillColor(pal.foreground());
    pContext->drawEllipse(
      CRect(
        tip.x - halfArcWidth, tip.y - halfArcWidth, tip.x + halfArcWidth,
        tip.y + halfArcWidth),
      kDrawFilled);

    setDirty(false);
  }

  void setArcWidth(double width) { halfArcWidth = width / 2.0; }
  void setDefaultTickLength(double length) { defaultTickLength = length; }

protected:
  CPoint mapValueToArc(double normalized, double length)
  {
    auto radian = 2.0 * normalized * Constants::pi;
    return CPoint(-sin(radian) * length, cos(radian) * length);
  }

  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  double halfArcWidth = 4.0;
  double defaultTickLength = 0.5;
};

template<typename Scale, Uhhyou::Style style = Uhhyou::Style::common>
class RotaryTextKnob : public RotaryKnobBase {
public:
  int32_t offset = 0;

  RotaryTextKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Scale &scale)
    : RotaryKnobBase(size, listener, tag, palette), fontId(fontId), scale(scale)
  {
    setWantsFocus(true);
    sensitivity = 0.002f;
    lowSensitivity = sensitivity / 10.0f;
  }

  CLASS_METHODS(RotaryTextKnob, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    if constexpr (style == Uhhyou::Style::accent) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightAccent() : pal.border());
    } else if (style == Uhhyou::Style::warning) {
      pContext->setFrameColor(isMouseEntered ? pal.highlightWarning() : pal.border());
    } else {
      pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
    }
    pContext->setFillColor(pal.boxBackground());
    pContext->setLineWidth(borderWidth);
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilledAndStroked);

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());

    auto displayValue = scale.map(value);
    if (precision == 0) displayValue = std::floor(displayValue);
    std::ostringstream os;
    os.precision(precision);
    os << std::fixed << displayValue + offset;
    textStr = os.str();
    pContext->drawString(textStr.c_str(), CRect(0, 0, width, height));

    setDirty(false);
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (event.buttonState.isLeft()) {
      beginEdit();
      isMouseDown = true;
      anchorPoint = event.mousePosition;
      event.consumed = true;
    } else if (event.buttonState.isMiddle()) {
      auto mid = getDefaultValue();
      value = value >= getMax() ? getMin() : value < mid ? mid : getMax();
      bounceValue();
      if (value != getOldValue()) valueChanged();
      if (isDirty()) invalid();
      event.consumed = true;
    }
  }

  void setPrecision(uint32_t precision) { this->precision = precision; }

protected:
  float borderWidth = 1.0f;
  uint32_t precision = 5;

  SharedPointer<CFontDesc> fontId;
  Scale &scale;

  std::string textStr;
};

} // namespace VSTGUI
