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
  double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  double lowSensitivity = sensitivity / 5.0;

  KnobBase(
    const CRect &size, IControlListener *listener, int32_t tag, Uhhyou::Palette &palette)
    : CControl(size, listener, tag), pal(palette)
  {
  }

  CLASS_METHODS(KnobBase, CControl);

  void draw(CDrawContext *pContext) override {}

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = true;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    isMouseEntered = false;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (!buttons.isLeftButton()) return kMouseEventNotHandled;

    beginEdit();
    if (checkDefaultValue(buttons)) {
      endEdit();
      return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    isMouseDown = true;
    anchorPoint = where;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (isMouseDown) endEdit();
    isMouseDown = false;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    if (!isMouseDown) return kMouseEventNotHandled;

    auto sensi = (buttons & kShift) ? lowSensitivity : sensitivity;
    value += (float)((anchorPoint.y - where.y) * sensi);
    bounceValue();

    if (value != getOldValue()) valueChanged();
    if (isDirty()) invalid();

    anchorPoint = where;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseCancel() override
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
    return kMouseEventHandled;
  }

  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override
  {
    if (isEditing() || axis != kMouseWheelAxisY || distance == 0.0f) return false;

    beginEdit();
    value += distance * float(sensitivity) * 0.5f;
    bounceValue();
    valueChanged();
    endEdit();
    invalid();
    return true;
  }

  void setSlitWidth(double width) { halfArcWidth = width / 2.0; }
  void setDefaultTickLength(double length) { defaultTickLength = length; }

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
      mapValueToSlit(defaultValue / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(defaultValue / getRange(), tipLength));

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
    CFontRef fontId,
    Uhhyou::Palette &palette,
    Scale &scale,
    bool isDecibel = false)
    : KnobBase(size, listener, tag, palette)
    , fontId(fontId)
    , scale(scale)
    , isDecibel(isDecibel)
  {
    this->fontId->remember();

    setWantsFocus(true);
    sensitivity = 0.002f;
    lowSensitivity = sensitivity / 10.0f;
  }

  ~TextKnob()
  {
    if (fontId) fontId->forget();
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

    auto displayValue = isDecibel ? 20.0 * log10(scale.map(value)) : scale.map(value);
    if (precision == 0) displayValue = floor(displayValue);
    std::ostringstream os;
    os.precision(precision);
    os << std::fixed << displayValue + offset;
    textStr = os.str();
    pContext->drawString(textStr.c_str(), CRect(0, 0, width, height));

    setDirty(false);
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) {
      beginEdit();
      if (checkDefaultValue(buttons)) {
        endEdit();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
      }
      isMouseDown = true;
      anchorPoint = where;
      return kMouseEventHandled;
    } else if (buttons.isMiddleButton()) {
      value = value > getMin() ? getMin() : getMax();
      bounceValue();
      if (value != getOldValue()) valueChanged();
      if (isDirty()) invalid();
    }
    return kMouseEventNotHandled;
  }

  int32_t onKeyDown(VstKeyCode &keyCode) override
  {
    if (keyCode.character != 't') return -1;
    value = value > getMin() ? getMin() : getMax();
    bounceValue();
    valueChanged();
    invalid();
    return 1;
  }

  void setPrecision(uint32_t precision) { this->precision = precision; }

protected:
  float borderWidth = 1.0f;
  uint32_t precision = 0;

  CFontRef fontId = nullptr;
  Scale &scale;
  const bool isDecibel;

private:
  std::string textStr;
};

template<typename Scale, Uhhyou::Style style = Uhhyou::Style::common>
class NumberKnob : public KnobBase {
public:
  NumberKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    CFontRef fontId,
    Uhhyou::Palette &palette,
    Scale &scale,
    int32_t offset = 0)
    : KnobBase(size, listener, tag, palette), fontId(fontId), scale(scale), offset(offset)
  {
    this->fontId->remember();
  }

  ~NumberKnob()
  {
    if (fontId) fontId->forget();
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
      mapValueToSlit(defaultValue / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(defaultValue / getRange(), tipLength));

    // Text.
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    numberStr
      = std::to_string(int32_t(floor(scale.map(getValueNormalized()))) + offset).c_str();
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
  CFontRef fontId = nullptr;
  Scale &scale;
  int32_t offset;

private:
  std::string numberStr;
};

} // namespace VSTGUI
