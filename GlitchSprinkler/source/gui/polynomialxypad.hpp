// (c) 2024 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "../../../common/gui/plugeditor.hpp"
#include "../../../common/gui/style.hpp"
#include "../dsp/polynomial.hpp"
#include "../parameter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace VSTGUI {

class PolynomialXYPad : public CControl {
public:
  PolynomialXYPad(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor)
    : CControl(size, listener, tag), pal(palette), editor(editor), requireUpdate(true)
  {
    if (editor) editor->remember();
  }

  ~PolynomialXYPad()
  {
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    // Preprocessing.
    if (requireUpdate) {
      requireUpdate = false;
      updateCoefficients();
    }

    // Transform coordinate.
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Background.
    pContext->setFillColor(pal.boxBackground());
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilled);

    // Grid.
    pContext->setLineWidth(1);
    pContext->setFrameColor(pal.foregroundInactive());
    constexpr size_t nGrid = 12;
    for (size_t idx = 1; idx < nGrid; ++idx) {
      const auto ratio = double(idx) / double(nGrid);

      const auto x = ratio * getWidth();
      const auto y = ratio * getHeight();

      pContext->drawLine(CPoint(x, 0), CPoint(x, getHeight()));
      pContext->drawLine(CPoint(0, y), CPoint(getWidth(), y));
    }

    // Waveform.
    auto mapPolyToY = [](double v, double h) { return (v + double(0.5)) * h; };

    pContext->setLineWidth(2);
    pContext->setFrameColor(pal.foreground());
    auto *polyPath = pContext->createGraphicsPath();

    auto y0 = mapPolyToY(polynomial.evaluate(double(0)), getHeight());
    polyPath->beginSubpath(0, y0);

    for (size_t idx = 1; idx < size_t(getWidth()); ++idx) {
      auto y = mapPolyToY(polynomial.evaluate(idx / getWidth()), getHeight());
      polyPath->addLine(idx, y);
    }

    pContext->drawGraphicsPath(polyPath, CDrawContext::PathDrawMode::kPathStroked);

    // Control points.
    pContext->setLineWidth(2);
    for (int idx = 0; idx < controlPoints.size(); ++idx) {
      pContext->setFrameColor(focusedPoint == idx ? pal.highlightMain() : pal.overlay());
      const auto &pt = controlPoints[idx];
      pContext->drawEllipse(CRect(
        pt.x - controlRadiusHalfOuter, pt.y - controlRadiusHalfOuter,
        pt.x + controlRadiusHalfOuter, pt.y + controlRadiusHalfOuter));
      pContext->drawEllipse(CRect(
        pt.x - controlRadiusHalfInner, pt.y - controlRadiusHalfInner,
        pt.x + controlRadiusHalfInner, pt.y + controlRadiusHalfInner));
    }

    // Border.
    const double borderW = isMouseEntered ? 2 * borderWidth : borderWidth;
    const double halfBorderWidth = int(borderW / 2.0);
    pContext->setFrameColor(pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(
      CRect(halfBorderWidth, halfBorderWidth, getWidth(), getHeight()), kDrawStroked);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseDown = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    if (!event.buttonState.isLeft()) return;
    isMouseDown = true;

    setMousePosition(event.mousePosition);
    grabbedPoint = hitTest(mousePosition);

    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isMouseDown) {
      isMouseDown = false;
      invalid();
    }
    event.consumed = true;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    setMousePosition(event.mousePosition);
    event.consumed = true;

    if (isMouseDown) {
      if (grabbedPoint < 0) return;

      auto &point = controlPoints[grabbedPoint];
      point.x = std::clamp(mousePosition.x, CCoord(1), getWidth() - 1);
      point.y = std::clamp(mousePosition.y, CCoord(1), getHeight() - 1);

      for (size_t idx = 0; idx < controlPoints.size(); ++idx) {
        if (idx == grabbedPoint) continue;
        if (std::abs(controlPoints[idx].x - point.x) > 1e-5) continue;
        point.x += 0.1;
        break;
      }

      using ID = Steinberg::Synth::ParameterID::ID;
      setParam(ID::polynomialPointX0 + grabbedPoint, point.x / getWidth());
      setParam(ID::polynomialPointY0 + grabbedPoint, point.y / getHeight());

      requireUpdate = true;
    } else {
      auto prevFocused = focusedPoint;
      focusedPoint = hitTest(mousePosition);
      if (prevFocused == focusedPoint) return;
    }

    invalid();
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isMouseDown) {
      isMouseDown = false;
      invalid();
    }
    isMouseEntered = false;
    event.consumed = true;
  }

  void scheduleUpdate() { requireUpdate = true; }
  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  void setXAt(size_t index, double x)
  {
    x = std::clamp(x, double(0), double(1));
    controlPoints[index].x = x * getWidth();
    scheduleUpdate();
  }

  void setYAt(size_t index, double y)
  {
    y = std::clamp(y, double(0), double(1));
    controlPoints[index].y = y * getHeight();
    scheduleUpdate();
  }

  CLASS_METHODS(PolynomialXYPad, CControl);

private:
  inline void setParam(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value)
  {
    if (editor) editor->valueChanged(id, value);
  }

  void setMousePosition(CPoint &pos)
  {
    auto view = getViewSize();
    mousePosition = pos - CPoint(view.left, view.top);
  }

  int hitTest(CPoint &point)
  {
    for (size_t idx = 0; idx < controlPoints.size(); ++idx) {
      const auto dx = controlPoints[idx].x - point.x;
      const auto dy = controlPoints[idx].y - point.y;
      if (dx * dx + dy * dy > controlRadiusFull * controlRadiusFull) continue;
      return idx;
    }
    return -1;
  }

  void updateCoefficients()
  {
    for (size_t idx = 0; idx < controlPoints.size(); ++idx) {
      const auto &pt = controlPoints[idx];
      polynomial.polyX[idx + 1] = pt.x / getWidth();
      polynomial.polyY[idx + 1] = pt.y / getHeight() - double(0.5);
    }
    polynomial.updateCoefficients();
  }

  Steinberg::Vst::PlugEditor *editor = nullptr;

  Uhhyou::Palette &pal;

  static constexpr double twopi = double(2) * std::numbers::pi_v<double>;
  static constexpr CCoord controlRadiusHalfOuter = 8.0;
  static constexpr CCoord controlRadiusHalfInner = controlRadiusHalfOuter / 2;
  static constexpr CCoord controlRadiusFull = 2 * controlRadiusHalfOuter;
  CCoord borderWidth = 1.0;

  CPoint mousePosition{-1.0, -1.0};
  bool isMouseDown = false;
  bool isMouseEntered = false;

  static constexpr size_t nControlPoint = nPolyOscControl;
  std::array<CPoint, nControlPoint> controlPoints;
  int focusedPoint = -1;
  int grabbedPoint = -1;

  bool requireUpdate = false;
  SomeDSP::PolynomialCoefficientSolver<double, nControlPoint> polynomial;
};

} // namespace VSTGUI