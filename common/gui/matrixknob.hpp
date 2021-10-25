// (c) 2021 Takamitsu Endo
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

#include "../dsp/constants.hpp"
#include "arraycontrol.hpp"
#include "style.hpp"
#include "textview.hpp"

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"

#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <cmath>
#include <functional>
#include <random>
#include <unordered_map>

namespace VSTGUI {

class MatrixKnob : public ArrayControl {
private:
  enum CommandMode : int32_t {
    modeNeutral = 0x0,
    modeRow = 0x1,
    modeColumn = 0x2,
    modeLink = 0x4,
  };

  constexpr static double arcNotchHalf = 30.0; // Radian.
  constexpr static double defaultTickLength = 0.5;
  constexpr static double arcWidth = 2.0;
  constexpr static double halfArcWidth = 1.0;

  std::vector<std::vector<double>> undoValue;
  std::vector<double> copyRow;
  std::vector<double> copyCol;
  std::unordered_map<uint32_t, uint32_t> idMap;

  const uint32_t nRow;
  const uint32_t nCol; // col is short for column.

  uint32_t focusRow = 1;
  uint32_t focusCol = 1;

  CPoint cursor{-1, -1};
  CPoint anchor{0, 0};
  bool isMouseLeftDown = false;
  bool isMouseEntered = false;
  bool isGrabbing = false;

  TextView *textView = nullptr; // nullptr means no display to status bar.
  int32_t mode = 0;

  CLineStyle lineStyle;
  Uhhyou::Palette &pal;

public:
  double sensitivity = 0.004; // MovedPixel * sensitivity = valueChanged.
  double lowSensitivity = sensitivity / 5.0;

  explicit MatrixKnob(
    Steinberg::Vst::VSTGUIEditor *editor,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue,
    uint32_t nRow,
    uint32_t nCol,
    TextView *textView,
    Uhhyou::Palette &palette)
    : ArrayControl(editor, size, id, value, defaultValue)
    , nRow(nRow)
    , nCol(nCol)
    , textView(textView)
    , lineStyle(CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound)
    , pal(palette)
  {
    assert(nRow * nCol != id.size());

    setWantsFocus(true);

    if (textView) textView->remember();

    for (size_t i = 0; i < 4; ++i) undoValue.emplace_back(defaultValue);
    copyRow.resize(nRow);
    copyCol.resize(nCol);
    for (uint32_t i = 0; i < id.size(); ++i) idMap.emplace(std::pair(id[i], i));
  }

  ~MatrixKnob()
  {
    if (textView) textView->forget();
  }

  CLASS_METHODS(MatrixKnob, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Background.
    pContext->setFillColor(pal.boxBackground());
    pContext->drawRect(CRect(0, 0, width, height), kDrawFilled);

    // Arc.
    const auto cellHeight = getHeight() / float(nRow);
    const auto cellWidth = getWidth() / float(nCol);
    const auto radius
      = (cellHeight > cellWidth ? cellWidth : cellHeight) / 2.0f - arcWidth;
    float centerX = 0;
    float centerY = 0;
    uint32_t focusIndex = focusRow * nCol + focusCol;
    pContext->setLineStyle(lineStyle);
    pContext->setLineWidth(2.0);
    for (uint32_t row = 0; row < nRow; ++row) {
      for (uint32_t col = 0; col < nCol; ++col) {
        uint32_t idx = getIndex(row, col);
        translatePoint(row, col, centerX, centerY);

        pContext->setFrameColor(
          idx == focusIndex ? pal.highlightMain() : pal.unfocused());
        const auto arcLeft = centerX - radius;
        const auto arcTop = centerY - radius;
        pContext->drawArc(
          CRect(arcLeft, arcTop, arcLeft + 2.0 * radius, arcTop + 2.0 * radius),
          float(90.0 + arcNotchHalf), float(90.0 - arcNotchHalf), kDrawStroked);

        // Tick for default value. Sharing color and style with Arc.
        const auto p1 = mapValueToArc(defaultValue[idx], -radius * defaultTickLength);
        const auto p2 = mapValueToArc(defaultValue[idx], -radius);
        pContext->drawLine(
          CPoint(p1.x + centerX, p1.y + centerY), CPoint(p2.x + centerX, p2.y + centerY));

        // Line from center to tip.
        pContext->setFrameColor(
          value[idx] == defaultValue[idx] ? pal.unfocused() : pal.foreground());
        const auto tip = mapValueToArc(value[idx], -radius);
        pContext->drawLine(
          CPoint(centerX, centerY), CPoint(tip.x + centerX, tip.y + centerY));

        // strokeColor(value[idx] == defaultValue[idx] ? pal.unfocused() :
        // pal.foreground()); beginPath(); moveTo(centerX, centerY); const auto tip =
        // mapValueToArc(value[idx], -radius); lineTo(tip.x + centerX, tip.y + centerY);
        // stroke();
      }
    }

    // Overlay.
    if (isGrabbing || isMouseEntered) {
      float rx = int32_t(cursor.x * nCol / width) * width / nCol;
      float ry = int32_t(cursor.y * nRow / height) * height / nRow;
      float rw = float(width) / nCol;
      float rh = float(height) / nRow;

      pContext->setFillColor(
        mode & modeRow ? pal.overlayHighlight() : pal.overlayFaint());
      pContext->drawRect(CRect(0, ry, width, ry + rh), kDrawFilled);

      pContext->setFillColor(
        mode & modeColumn ? pal.overlayHighlight() : pal.overlayFaint());
      pContext->drawRect(CRect(rx, 0, rx + rw, height), kDrawFilled);
    }

    setDirty(false);
  }

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
    if (buttons.isRightButton()) {
      auto componentHandler = editor->getController()->getComponentHandler();
      if (componentHandler == nullptr) return kMouseEventNotHandled;

      using namespace Steinberg;

      FUnknownPtr<Vst::IComponentHandler3> handler(componentHandler);
      if (handler == nullptr) return kMouseEventNotHandled;

      setMousePosition(where);
      setFocus(cursor);
      size_t index = getIndex(focusRow, focusCol);
      if (index >= id.size()) return kMouseEventNotHandled;

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return kMouseEventNotHandled;

      menu->popup(where.x, where.y);
      menu->release();
      return kMouseEventHandled;
    }

    if (buttons.isLeftButton()) {
      isMouseLeftDown = true;
      setFocus(cursor);
      if (buttons.isControlSet()) {
        uint32_t idx = getIndex(focusRow, focusCol);
        value[idx] = defaultValue[idx];
        updateValueAt(idx);
      } else {
        isGrabbing = true;
        setMousePosition(where);
        anchor = where;
      }
    }
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    isMouseLeftDown = false;
    isGrabbing = false;
    updateValueWithUndo();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    if (isMouseLeftDown) {
      if (isGrabbing) {
        auto sensi = (buttons.isShiftSet()) ? lowSensitivity : sensitivity;
        setValueFromDelta((anchor.y - where.y) * sensi);

        anchor = where;

        invalid();
        return kMouseEventHandled;
      } else if (buttons.isControlSet()) {
        uint32_t idx = getIndex(focusRow, focusCol);
        value[idx] = defaultValue[idx];
        updateValueAt(idx);
      }
    }

    if (isMouseEntered) {
      setMousePosition(where);
      setFocus(cursor);
    } else {
      focusCol = nCol;
      focusRow = nRow;
    }
    invalid();
    return kMouseEventNotHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    isMouseLeftDown = false;
    isMouseEntered = false;
    isGrabbing = false;
    if (isDirty()) {
      updateValueWithUndo();
      invalid();
    }
    return kMouseEventHandled;
  }

  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override
  {
    if (isEditing() || axis != kMouseWheelAxisY || distance == 0.0) return false;

    auto sensi = (buttons.isShiftSet()) ? lowSensitivity : 8.0 * sensitivity;
    setValueFromDelta(distance * sensi);
    updateValueWithUndo();
    invalid();
    return true;
  }

  void updateValueWithUndo()
  {
    ArrayControl::updateValue();

    std::rotate(undoValue.begin(), undoValue.begin() + 1, undoValue.end());
    undoValue.back() = value;
  }

  void setValueFromDelta(double delta)
  {
    bool linked = mode & modeLink;
    bool colMode = mode & modeColumn;
    bool rowMode = mode & modeRow;

    uint32_t focusIndex = getIndex(focusRow, focusCol);
    value[focusIndex] = std::clamp(value[focusIndex] + delta, 0.0, 1.0);

    if (!linked || (linked && !colMode && !rowMode)) {
      updateValueAt(focusIndex);
      return;
    }

    if (colMode) {
      for (uint32_t row = 0; row < nRow; ++row) {
        auto idx = row * nCol + focusCol;
        if (idx == focusIndex) continue;
        if (idx >= value.size()) break;
        value[idx] = std::clamp(value[idx] + delta, 0.0, 1.0);
      }
    }
    if (rowMode) {
      for (uint32_t col = 0; col < nCol; ++col) {
        auto idx = focusRow * nCol + col;
        if (idx == focusIndex) continue;
        if (idx >= value.size()) break;
        value[idx] = std::clamp(value[idx] + delta, 0.0, 1.0);
      }
    }
    updateValueWithUndo();
  }

  virtual void setValueFromId(int id, double normalized)
  {
    auto idPair = idMap.find(id);
    if (idPair == idMap.end()) return;
    auto index = idPair->second;
    if (index < value.size()) {
      auto tmp = std::clamp(normalized, 0.0, 1.0);
      if (std::fabs(tmp - defaultValue[index]) < FLT_EPSILON) tmp = defaultValue[index];
      value[index] = tmp;
    }
  }

  int32_t onKeyDown(VstKeyCode &key) override
  {
    if (!isMouseEntered) return 1;

    bool shift = key.modifier & MODIFIER_SHIFT;
    if (key.character == 'c') { // Copy.
      if (mode == modeNeutral) {
        updateTextView("c: Copy failed. Enable column(1)/row(2) mode to copy.");
        invalid();
        return -1;
      }
      copy();
      updateTextView("c: Copy: Done.");
    } else if (key.character == 'd') {
      resetToDefault();
      updateTextView("d: Reset to default: Done.");
    } else if (key.character == 'l') {
      mode ^= modeLink;
      updateTextView("l: Toggle link mode.");
    } else if (key.character == 'q') {
      mode ^= modeColumn;
      updateTextView("q: Toggle column mode.");
    } else if (key.character == 'r') {
      totalRandomize();
      updateTextView("r: Randomize: Done.");
    } else if (key.character == 't') { // subTle randomize. Random walk.
      randomize(0.02);
      updateTextView("t: Subtle Randomize: Done.");
    } else if (key.character == 'v') { // Paste.
      if (mode == modeNeutral) {
        updateTextView("v: Paste failed. Enable column(1)/row(2) mode to paste.");
        invalid();
        return -1;
      }
      paste();
      updateTextView("v: Paste: Done.");
    } else if (key.character == 'w') {
      mode ^= modeRow;
      updateTextView("w: Toggle row mode.");
    } else if (key.character == 'z' && shift) { // Redo
      redo();
      ArrayControl::updateValue();
      updateTextView("Redo: Done.");
      invalid();
      return 1;
    } else if (key.character == 'z') { // Undo
      undo();
      ArrayControl::updateValue();
      updateTextView("Undo: Done.");
      invalid();
      return 1;
    } else {
      std::string str("No bind on key ");
      updateTextView(str + std::to_string(key.character));
      return -1;
    }
    updateValueWithUndo();
    invalid();
    return 1;
  }

private:
  void updateTextView(std::string text)
  {
    if (textView == nullptr) return;
    std::string modeStr;
    if (mode & modeColumn) modeStr += "Column, ";
    if (mode & modeRow) modeStr += "Row, ";
    if (mode & modeLink) modeStr += "Link, ";
    textView->setText(text + "\n" + modeStr);
    textView->invalid();
  }

  void undo()
  {
    std::rotate(undoValue.rbegin(), undoValue.rbegin() + 1, undoValue.rend());
    value = undoValue.back();
  }

  void redo()
  {
    std::rotate(undoValue.begin(), undoValue.begin() + 1, undoValue.end());
    value = undoValue.back();
  }

  void resetToDefault()
  {
    for (size_t i = 0; i < value.size(); ++i) value[i] = defaultValue[i];
  }

  void copy()
  {
    if (mode & modeColumn && copyCol.size() >= nCol) {
      for (uint32_t row = 0; row < nRow; ++row) {
        auto index = row * nCol + focusCol;
        if (index >= value.size()) break;
        copyRow[row] = value[index];
      }
    }
    if (mode & modeRow && copyRow.size() >= nRow) {
      for (uint32_t col = 0; col < nCol; ++col) {
        auto index = focusRow * nCol + col;
        if (index >= value.size()) break;
        copyCol[col] = value[index];
      }
    }
  }

  void paste()
  {
    if (mode & modeColumn && copyCol.size() >= nCol) {
      for (uint32_t row = 0; row < nRow; ++row) {
        auto index = row * nCol + focusCol;
        if (index >= value.size()) break;
        value[index] = copyRow[row];
      }
    }
    if (mode & modeRow && copyRow.size() >= nRow) {
      for (uint32_t col = 0; col < nCol; ++col) {
        auto index = focusRow * nCol + col;
        if (index >= value.size()) break;
        value[index] = copyCol[col];
      }
    }
  }

  void applyRandomize(std::function<double(double)> func)
  {
    if (mode == modeNeutral) {
      for (size_t i = 0; i < value.size(); ++i) value[i] = func(value[i]);
      return;
    }
    if (mode & modeColumn && copyCol.size() >= nCol) {
      for (uint32_t row = 0; row < nRow; ++row) {
        auto index = row * nCol + focusCol;
        if (index >= value.size()) break;
        value[index] = func(value[index]);
      }
    }
    if (mode & modeRow && copyRow.size() >= nRow) {
      for (uint32_t col = 0; col < nCol; ++col) {
        auto index = focusRow * nCol + col;
        if (index >= value.size()) break;
        value[index] = func(value[index]);
      }
    }
  }

  void totalRandomize()
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    applyRandomize([&](double /* val */) { return dist(rng); });
  }

  void randomize(double amount)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    amount /= 2;
    applyRandomize([&](double val) {
      std::uniform_real_distribution<double> dist(val - amount, val + amount);
      return std::clamp(dist(rng), 0.0, 1.0);
    });
  }

  CPoint mapValueToArc(double normalized, double length)
  {
    auto radian = (2.0 * normalized - 1.0) * (SomeDSP::pi * (1.0 - arcNotchHalf / 360.0));
    return CPoint(-sin(radian) * length, cos(radian) * length);
  }

  void translatePoint(uint32_t row, uint32_t col, float &x, float &y)
  {
    x = (col + 0.5f) * getWidth() / float(nCol);
    y = (row + 0.5f) * getHeight() / float(nRow);
  }

  void setFocus(const CPoint &pos)
  {
    focusCol = pos.x * nCol / getWidth();
    focusRow = pos.y * nRow / getHeight();
  }

  void setMousePosition(CPoint &where)
  {
    auto view = getViewSize();
    cursor = where - CPoint(view.left, view.top);
  }

  uint32_t getIndex(uint32_t row, uint32_t col) { return row * nCol + col; }
};

} // namespace VSTGUI
