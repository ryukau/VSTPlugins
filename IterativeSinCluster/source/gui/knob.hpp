// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

#include <sstream>
#include <string>

namespace VSTGUI {

class Knob : public CControl {
public:
  Knob(const CRect &size, IControlListener *listener, int32_t tag)
    : CControl(size, listener, tag)
  {
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(Knob, CControl);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;

  void setSlitWidth(double width);
  void setDefaultTickLength(double length);
  void setBackgroundColor(CColor color);
  void setSlitColor(CColor color);
  void setHighlightColor(CColor color);
  void setTipColor(CColor color);

protected:
  CColor backgroundColor = CColor(255, 255, 255, 255);
  CColor highlightColor = CColor(0x33, 0xee, 0xee, 0xff);
  CColor slitColor = CColor(0xdd, 0xdd, 0xdd, 0xff);
  CColor tipColor = CColor(0, 0, 0, 255);

  const CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};

  double halfSlitWidth = 4.0;
  const double slitNotchHalf = 30.0; // Degree.
  double defaultTickLength = 0.5;

  double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  double lowSensitivity = sensitivity / 5.0;

  CPoint anchorPoint{0.0, 0.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  CPoint mapValueToSlit(double normalized, double length);
};

template<typename Scale> class TextKnob : public Knob {
public:
  TextKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    Scale &scale,
    bool isDecibel = false)
    : Knob(size, listener, tag), scale(scale), isDecibel(isDecibel)
  {
    setWantsFocus(true);
    sensitivity = 0.002f;
    lowSensitivity = sensitivity / 10.0f;
  }

  ~TextKnob()
  {
    if (fontID) fontID->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    pContext->setFrameColor(isMouseEntered ? highlightColor : foregroundColor);
    pContext->setLineWidth(borderWidth);
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawStroked);

    // Text.
    pContext->setFont(fontID);
    pContext->setFontColor(foregroundColor);

    auto displayValue = isDecibel ? 20.0 * log10(scale.map(value)) : scale.map(value);
    std::ostringstream os;
    os.precision(precision);
    os << std::fixed << displayValue;
    std::string txt(os.str());
    pContext->drawString(
      UTF8String(txt.c_str()).getPlatformString(), CRect(0, 0, width, height));

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

  void setForegroundColor(CColor color) { foregroundColor = color; }
  void setPrecision(uint32_t precision) { this->precision = precision; }
  void setFont(CFontRef fontID) { this->fontID = fontID; }

protected:
  CColor foregroundColor{0, 0, 0};
  float borderWidth = 1.0f;
  uint32_t precision = 0;

  CFontRef fontID = nullptr;
  Scale &scale;
  const bool isDecibel;
};

template<typename Scale> class NumberKnob : public Knob {
public:
  NumberKnob(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    Scale &scale,
    int32_t offset = 0)
    : Knob(size, listener, tag), scale(scale), offset(offset)
  {
  }

  ~NumberKnob()
  {
    if (fontID) fontID->forget();
  }

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
    pContext->setFillColor(backgroundColor);
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

    // Slit.
    auto radius = center.x > center.y ? center.y : center.x;
    pContext->setFrameColor(isMouseEntered ? highlightColor : slitColor);
    pContext->setLineStyle(lineStyle);
    pContext->setLineWidth(halfSlitWidth * 2.0);
    pContext->drawArc(
      CRect(
        halfSlitWidth - radius, halfSlitWidth - radius, radius - halfSlitWidth,
        radius - halfSlitWidth),
      (float)(90.0 + slitNotchHalf), (float)(90.0 - slitNotchHalf));

    // Tick for default value. Sharing color and style with slit.
    auto tipLength = halfSlitWidth - radius;
    pContext->setLineWidth(halfSlitWidth / 2.0);
    pContext->drawLine(
      mapValueToSlit(defaultValue / getRange(), tipLength * defaultTickLength),
      mapValueToSlit(defaultValue / getRange(), tipLength));

    // Text.
    pContext->setFont(fontID);
    pContext->setFontColor(tipColor);
    UTF8String text
      = std::to_string(int32_t(floor(scale.map(getValueNormalized()))) + offset).c_str();
    const auto textWidth = pContext->getStringWidth(text);
    const auto textLeft = -textWidth * 0.5;
    const auto textRight = textWidth * 0.5;
    const auto textTop = -fontID->getSize() * 0.5;
    const auto textBottom = fontID->getSize() * 0.5;
    pContext->drawString(
      text.getPlatformString(), CRect(textLeft, textTop, textRight, textBottom));

    // Tip.
    auto tip = mapValueToSlit(getValueNormalized(), tipLength);
    pContext->setFillColor(tipColor);
    pContext->drawEllipse(
      CRect(
        tip.x - halfSlitWidth, tip.y - halfSlitWidth, tip.x + halfSlitWidth,
        tip.y + halfSlitWidth),
      kDrawFilled);

    setDirty(false);
  }

  CLASS_METHODS(Knob, CControl);

  void setFont(CFontRef fontID) { this->fontID = fontID; }

protected:
  Scale &scale;
  int32_t offset;
  CFontRef fontID = nullptr;
};

} // namespace VSTGUI
