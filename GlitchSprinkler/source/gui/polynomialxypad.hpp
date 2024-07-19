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
#include <random>
#include <string>

namespace VSTGUI {

class PolynomialXYPad : public ArrayControl {
private:
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

  Uhhyou::Palette &pal;

  SharedPointer<Label> polyXLabel;
  SharedPointer<Label> polyYLabel;
  SharedPointer<CControl> polyXControl;
  SharedPointer<CControl> polyYControl;

  enum class AxisLock { none, x, y };

  AxisLock getLockState(MouseEvent &event)
  {
    if (event.buttonState.isMiddle()) {
      return event.modifiers.has(ModifierKey::Shift) ? AxisLock::x : AxisLock::y;
    }
    return AxisLock::none;
  }

public:
  PolynomialXYPad(
    Steinberg::Vst::VSTGUIEditor *editor,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue,
    Uhhyou::Palette &palette,
    SharedPointer<Label> polyXLabel,
    SharedPointer<Label> polyYLabel,
    SharedPointer<CControl> polyXControl,
    SharedPointer<CControl> polyYControl)
    : ArrayControl(editor, size, id, value, defaultValue)
    , pal(palette)
    , requireUpdate(true)
    , polyXLabel(polyXLabel)
    , polyYLabel(polyYLabel)
    , polyXControl(polyXControl)
    , polyYControl(polyYControl)
  {
    setWantsFocus(true);

    updateControlPoints();
  }

  virtual void setValueAt(Steinberg::Vst::ParamID id_, double normalized) override
  {
    ArrayControl::setValueAt(id_, normalized);

    using ID = Steinberg::Synth::ParameterID::ID;

    auto iter = idMap.find(id_);
    if (iter == idMap.end()) return;
    auto index = iter->second;

    if (index >= 0 && index < nControlPoint) {
      setXAt(index, normalized);
    } else {
      setYAt(index % nControlPoint, normalized);
    }
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
    releaseFocus();
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    if (!event.buttonState.isLeft() && !event.buttonState.isMiddle()) return;
    isMouseDown = true;
    grabFocus();

    setMousePosition(event.mousePosition);
    grabbedPoint = hitTest(mousePosition);

    if (grabbedPoint >= 0) {
      linkControl(grabbedPoint);
    }

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

      auto lock = getLockState(event);

      using ID = Steinberg::Synth::ParameterID::ID;
      auto &point = controlPoints[grabbedPoint];
      if (lock != AxisLock::x) {
        point.x = std::clamp(mousePosition.x, CCoord(1), getWidth() - 1);

        for (size_t idx = 0; idx < controlPoints.size(); ++idx) {
          if (idx == grabbedPoint) continue;
          if (std::abs(controlPoints[idx].x - point.x) > double(1e-5)) continue;
          point.x += 0.1;
          break;
        }

        editAndUpdateValueAt(ID::polynomialPointX0 + grabbedPoint, point.x / getWidth());
      }
      if (lock != AxisLock::y) {
        point.y = std::clamp(mousePosition.y, CCoord(1), getHeight() - 1);
        editAndUpdateValueAt(ID::polynomialPointY0 + grabbedPoint, point.y / getHeight());
      }
      linkControl(grabbedPoint);

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

  template<typename xFunc, typename yFunc> void setControlPoints(xFunc fx, yFunc fy)
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    for (size_t idx = 0; idx < nControlPoint; ++idx) {
      const auto ratio = double(idx + 1) / double(nControlPoint + 1);
      setValueAt(ID::polynomialPointX0 + idx, fx(ratio));
      setValueAt(ID::polynomialPointY0 + idx, fy(ratio));
    }

    invalid();
    editAndUpdateValue();
  }

  double triangle(double t)
  {
    t += double(0.75);
    t -= std::floor(t);
    return std::abs(double(2) * t - double(1));
  }

  void onKeyboardEvent(KeyboardEvent &event) override
  {
    if (!isMouseEntered || event.type == EventType::KeyUp) return;

    constexpr auto pi = std::numbers::pi_v<double>;
    constexpr auto twopi = double(2) * pi;

    if (event.character == '1') { // Preset: sine.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          return double(0.5) * (std::sin(twopi * ratio) + double(1));
        });
    } else if (event.character == '2') { // Preset: fmA.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          return double(0.5)
            * (std::sin(twopi * ratio + double(4) * std::sin(twopi * ratio)) + double(1));
        });
    } else if (event.character == '3') { // Preset: fmB.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          return double(0.5)
            * (std::sin(twopi * ratio + std::sin(twopi * ratio)) + double(1));
        });
    } else if (event.character == '4') { // Preset: sawtooth.
      setControlPoints(
        [&](double ratio) { return ratio; }, [&](double ratio) { return ratio; });
    } else if (event.character == '5') { // Preset: triangle.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) { return triangle(ratio); });
    } else if (event.character == '6') { // Preset: trapezoid.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          return std::clamp(
            double(1.5) * triangle(ratio) - double(0.25), double(0), double(1));
        });
    } else if (event.character == '7') { // Preset: alternate.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          double i = ratio * double(nControlPoint) + double(0.5);
          return std::fmod(i, double(2)) < double(1) ? double(0.4) : double(0.6);
        });
    } else if (event.character == '8') { // Preset: pulse.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) { return ratio < double(0.5) ? double(0.25) : double(0.75); });
    } else if (event.character == '9') { // Preset: saturated sine.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          return double(0.5) + double(0.5) * std::tanh(std::sin(twopi * ratio));
        });
    } else if (event.character == '0') { // Preset: saturated sine.
      setControlPoints(
        [&](double ratio) { return ratio; }, [&](double ratio) { return double(0.5); });
    } else if (event.character == 'r') { // Randomize.
      setControlPoints(
        [&](double ratio) { return ratio; },
        [&](double ratio) {
          std::random_device dev;
          std::uniform_real_distribution<double> dist{double(0), double(1)};
          return dist(dev);
        });
    }

    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  void linkControlFromId(Steinberg::Vst::ParamID id)
  {
    auto iter = idMap.find(id);
    if (iter == idMap.end()) return;
    linkControl(iter->second);
  }

  CLASS_METHODS(PolynomialXYPad, CControl);

private:
  void setXAt(size_t index, double x)
  {
    x = std::clamp(x, double(0), double(1));
    controlPoints[index].x = x * getWidth();
    requireUpdate = true;
  }

  void setYAt(size_t index, double y)
  {
    y = std::clamp(y, double(0), double(1));
    controlPoints[index].y = y * getHeight();
    requireUpdate = true;
  }

  void updateControlPoints()
  {
    for (size_t idx = 0; idx < nControlPoint; ++idx) {
      setXAt(idx, value[idx]);
      setYAt(idx, value[idx + nControlPoint]);
    }
  }

  void linkControl(size_t index)
  {
    const auto indexX = index % nPolyOscControl;
    if (polyXControl != nullptr) {
      polyXControl->setTag(id[indexX]);
      polyXControl->setValueNormalized(value[indexX]);
      polyXControl->invalid();
    }

    if (polyYControl != nullptr) {
      const auto indexY = indexX + nPolyOscControl;
      polyYControl->setTag(id[indexY]);
      polyYControl->setValueNormalized(value[indexY]);
      polyYControl->invalid();
    }

    if (polyXLabel != nullptr) {
      polyXLabel->setText("X" + std::to_string(indexX));
      polyXLabel->invalid();
    }

    if (polyYLabel != nullptr) {
      polyYLabel->setText("Y" + std::to_string(indexX));
      polyYLabel->invalid();
    }
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
};

} // namespace VSTGUI
