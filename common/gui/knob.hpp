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

#include "vstgui/vstgui.h"

#include "style.hpp"

#include <sstream>
#include <string>

namespace VSTGUI {

class KnobBase : public CControl {
public:
  bool liveUpdate = true;     // When false, only update value on mouse up event.
  double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  double lowSensitivity = sensitivity / 5.0;
  double wheelSensitivity = sensitivity / 10.0;

  KnobBase(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : CControl(size, listener, tag), pal(palette)
  {
  }

  CLASS_METHODS(KnobBase, CControl);

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

    auto sensi = event.modifiers.has(ModifierKey::Shift) ? lowSensitivity : sensitivity;
    value += (float)((anchorPoint.y - event.mousePosition.y) * sensi);
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
    value += event.deltaY * float(wheelSensitivity);
    bounceValue();
    valueChanged();
    endEdit();
    invalid();
    event.consumed = true;
  }

  void setSlitWidth(double width) { halfArcWidth = width / 2.0; }
  void setDefaultTickLength(double length) { defaultTickLength = length; }

  void setSensitivity(double sensi, double lowSensi, double wheelSensi)
  {
    sensitivity = sensi;
    lowSensitivity = lowSensi;
    wheelSensitivity = wheelSensi;
  }

protected:
  CPoint mapValueToSlit(double normalized, double length)
  {
    auto radian
      = (2.0 * normalized - 1.0) * Constants::pi * (180.0 - arcNotchHalf) / 180.0;
    return CPoint(-sin(radian) * length, cos(radian) * length);
  }

  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  double halfArcWidth = 4.0;
  const double arcNotchHalf = 30.0; // Degree.
  double defaultTickLength = 0.5;

  CPoint anchorPoint{0.0, 0.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  Uhhyou::Palette &pal;
};

template<Uhhyou::Style style = Uhhyou::Style::common> class Knob : public KnobBase {
public:
  Knob(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : KnobBase(size, listener, tag, palette)
  {
  }

  CLASS_METHODS(Knob, CControl);

  void draw(CDrawContext *pContext) override
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
    pContext->drawArc(
      CRect(
        halfArcWidth - radius, halfArcWidth - radius, radius - halfArcWidth,
        radius - halfArcWidth),
      (float)(90.0 + arcNotchHalf), (float)(90.0 - arcNotchHalf));

    // Tick for default value. Sharing color and style with slit.
    auto tipLength = halfArcWidth - radius;
    pContext->setLineWidth(halfArcWidth / 2.0);
    pContext->drawLine(
      mapValueToSlit(getDefaultValue() / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(getDefaultValue() / getRange(), tipLength));

    // Line from center to tip.
    auto tip = mapValueToSlit(getValueNormalized(), tipLength);
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
};

template<typename Scale, Uhhyou::Style style = Uhhyou::Style::common>
class TextKnob : public KnobBase {
public:
  int32_t offset = 0;

  TextKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Scale &scale,
    bool isDecibel = false)
    : KnobBase(size, listener, tag, palette)
    , fontId(fontId)
    , scale(scale)
    , isDecibel(isDecibel)
  {
    setWantsFocus(true);
    sensitivity = 0.002f;
    lowSensitivity = sensitivity / 10.0f;
  }

  CLASS_METHODS(TextKnob, CControl);

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

    auto displayValue
      = isDecibel ? 20.0 * std::log10(scale.map(value)) : scale.map(value);
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
  uint32_t precision = 0;

  SharedPointer<CFontDesc> fontId;
  Scale &scale;
  const bool isDecibel;

  std::string textStr;
};

template<typename Scale, Uhhyou::Style style = Uhhyou::Style::common>
class NumberKnob : public KnobBase {
public:
  NumberKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Scale &scale,
    int32_t offset = 0)
    : KnobBase(size, listener, tag, palette), fontId(fontId), scale(scale), offset(offset)
  {
  }

  CLASS_METHODS(NumberKnob, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();
    const auto center = CPoint(width / 2.0, height / 2.0);

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft() + center));

    // Background.
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
    pContext->drawArc(
      CRect(
        halfArcWidth - radius, halfArcWidth - radius, radius - halfArcWidth,
        radius - halfArcWidth),
      (float)(90.0 + arcNotchHalf), (float)(90.0 - arcNotchHalf));

    // Tick for default value. Sharing color and style with slit.
    auto tipLength = halfArcWidth - radius;
    pContext->setLineWidth(halfArcWidth / 2.0);
    pContext->drawLine(
      mapValueToSlit(getDefaultValue() / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(getDefaultValue() / getRange(), tipLength));

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    numberStr
      = std::to_string(int32_t(std::floor(scale.map(getValueNormalized()))) + offset)
          .c_str();
    const auto textWidth = pContext->getStringWidth(numberStr.c_str());
    const auto textLeft = -textWidth * 0.5;
    const auto textRight = textWidth * 0.5;
    const auto textTop = -fontId->getSize() * 0.5;
    const auto textBottom = fontId->getSize() * 0.5;
    pContext->drawString(
      numberStr.c_str(), CRect(textLeft, textTop, textRight, textBottom));

    // Tip.
    auto tip = mapValueToSlit(getValueNormalized(), tipLength);
    pContext->setFillColor(pal.foreground());
    pContext->drawEllipse(
      CRect(
        tip.x - halfArcWidth, tip.y - halfArcWidth, tip.x + halfArcWidth,
        tip.y + halfArcWidth),
      kDrawFilled);

    setDirty(false);
  }

protected:
  SharedPointer<CFontDesc> fontId;
  Scale &scale;
  int32_t offset;

private:
  std::string numberStr;
};

} // namespace VSTGUI
