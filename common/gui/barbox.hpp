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

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "vstgui/vstgui.h"

#include "../dsp/scale.hpp"
#include "arraycontrol.hpp"
#include "style.hpp"

#include <algorithm>
#include <random>

#include <iostream>

namespace VSTGUI {

template<typename Scrollable> class ScrollBar : public CControl {
public:
  ScrollBar(
    const CRect &size,
    IControlListener *listener,
    Scrollable parent,
    Uhhyou::Palette &palette)
    : CControl(size, listener), parent(parent), pal(palette)
  {
    parent->remember();
  }

  ~ScrollBar() { parent->forget(); }

  CLASS_METHODS(ScrollBar, CControl);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    pContext->setLineWidth(2.0);
    pContext->setFrameColor(pal.border());

    // Bar.
    CCoord rightHandleL = rightPos * width - handleWidth;
    CCoord left = leftPos * width;
    CCoord barL = left + handleWidth;
    pContext->setFillColor(
      pointed == Part::bar ? pal.highlightButton() : pal.highlightMain());
    pContext->drawRect(CRect(barL, 0, rightHandleL, height), kDrawFilledAndStroked);

    // Left handle.
    pContext->setFillColor(
      pointed == Part::leftHandle ? pal.highlightButton() : pal.unfocused());
    pContext->drawRect(CRect(left, 0, left + handleWidth, height), kDrawFilledAndStroked);

    // Right handle.
    pContext->setFillColor(
      pointed == Part::rightHandle ? pal.highlightButton() : pal.unfocused());
    pContext->drawRect(
      CRect(rightHandleL, 0, rightHandleL + handleWidth, height), kDrawFilledAndStroked);
  }

  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override
  {
    grabbed = pointed = Part::background;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) {
      CPoint point = translateMousePosition(where);

      grabbed = pointed = hitTest(point);

      if (grabbed == Part::leftHandle)
        grabOffset = int(leftPos * getWidth() - point.x);
      else if (grabbed == Part::rightHandle)
        grabOffset = int(rightPos * getWidth() - point.x);
      else if (grabbed == Part::bar)
        grabOffset = int(leftPos * getWidth() - point.x);

    } else if (buttons.isRightButton()) {
      leftPos = 0;
      rightPos = 1;
      parent->setViewRange(leftPos, rightPos);
    }
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    grabbed = Part::background;
    invalid();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    CPoint point = translateMousePosition(where);

    auto posX = std::clamp<int>(point.x + grabOffset, 0, getWidth()) / float(getWidth());
    switch (grabbed) {
      case Part::bar: {
        auto barWidth = rightPos - leftPos;
        leftPos = posX;
        rightPos = leftPos + barWidth;
        if (leftPos < 0.0f) {
          rightPos = barWidth;
          leftPos = 0.0f;
        } else if (rightPos > 1.0f) {
          leftPos = 1.0f - barWidth;
          rightPos = 1.0f;
        }

        // Just in case.
        leftPos = std::clamp<CCoord>(leftPos, 0, 1);
        rightPos = std::clamp<CCoord>(rightPos, 0, 1);
      } break;

      case Part::leftHandle: {
        setLeftPos(posX);
      } break;

      case Part::rightHandle: {
        setRightPos(posX);
      } break;

      default:
        pointed = hitTest(point);
        invalid();
        return kMouseEventHandled;
    }

    parent->setViewRange(leftPos, rightPos);
    invalid();

    return kMouseEventHandled;
  }

  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override
  {
    CPoint point = translateMousePosition(where);

    const CCoord mouseX = float(point.x) / getWidth();
    const CCoord delta = distance;

    float amountL, amountR;
    if (delta > 0) {
      amountL = 0.5 * zoomSensi;
      amountR = 0.5 * zoomSensi;
    } else {
      const CCoord bias = (mouseX - leftPos) / (rightPos - leftPos);
      amountL = zoomSensi * std::clamp<CCoord>(bias, 0.0, 1.0);
      amountR = zoomSensi * std::clamp<CCoord>((1.0 - bias), 0.0, 1.0);
      if (bias < 0.0 || 1.0 < bias) std::swap(amountL, amountR);
    }
    setLeftPos(leftPos - amountL * delta);
    setRightPos(rightPos + amountR * delta);

    parent->setViewRange(leftPos, rightPos);
    invalid();

    return true;
  }

  void setHandleWidth(float width) { handleWidth = std::max(width, 0.0f); }

protected:
  enum class Part : uint8_t { background = 0, bar, leftHandle, rightHandle };

  inline CPoint translateMousePosition(const CPoint &where)
  {
    auto view = getViewSize();
    return where - CPoint(view.left, view.top);
  }

  inline void setLeftPos(CCoord x)
  {
    CCoord rightMost = std::max(rightPos - 3 * handleWidth / getWidth(), 0.0);
    leftPos = std::clamp<CCoord>(x, 0, rightMost);
  }

  inline void setRightPos(CCoord x)
  {
    CCoord leftMost = std::min(leftPos + 3 * handleWidth / getWidth(), 1.0);
    rightPos = std::clamp<CCoord>(x, leftMost, 1);
  }

  Part hitTest(const CPoint &point)
  {
    if (point.y < 0 || point.y > int(getHeight())) return Part::background;

    auto left = leftPos * getWidth();
    auto right = rightPos * getWidth();
    auto width = handleWidth;
    auto px = point.x;

    auto leftHandleR = left + width;
    if (px >= left && px <= leftHandleR) return Part::leftHandle;

    auto rightHandleL = right - width;
    if (px >= rightHandleL && px <= right) return Part::rightHandle;

    if (px > leftHandleR && px < rightHandleL) return Part::bar;

    return Part::background;
  }

  CCoord zoomSensi = 0.05;
  CCoord handleWidth = 10;
  CCoord leftPos = 0;
  CCoord rightPos = 1;
  int grabOffset = 0;

  Part pointed = Part::background;
  Part grabbed = Part::background;

  Scrollable parent;
  Uhhyou::Palette &pal;
};

template<typename Scale> class BarBox : public ArrayControl {
public:
  float sliderZero = 0.0f;
  int32_t indexOffset = 0;
  bool liveUpdateLineEdit = true; // Set this false when line edit is slow.

  BarBox(
    Steinberg::Vst::VSTGUIEditor *editor,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue,
    Uhhyou::Palette &palette)
    : ArrayControl(editor, size, id, value, defaultValue)
    , sliderWidth((size.right - size.left) / value.size())
    , pal(palette)
  {
    setWantsFocus(true);

    for (size_t i = 0; i < id.size(); ++i)
      barIndices.push_back(std::to_string(i + indexOffset));

    setViewRange(0, 1);

    for (size_t i = 0; i < 4; ++i) undoValue.emplace_back(defaultValue);
  }

  ~BarBox()
  {
    if (indexFontID) indexFontID->forget();
    if (nameFontID) nameFontID->forget();
  }

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

    // Value bar.
    pContext->setFillColor(pal.highlightMain());

    float sliderZeroHeight = height * (1.0 - sliderZero);
    for (int i = indexL; i < indexR; ++i) {
      auto left = (i - indexL) * sliderWidth;
      auto right = left + sliderWidth - barWidth;
      auto top = height - value[i] * height;
      double bottom = sliderZeroHeight;
      if (top > bottom) std::swap(top, bottom);
      pContext->drawRect(CRect(left, top, right, bottom), kDrawFilled);
    }

    // Index text.
    pContext->setFrameColor(pal.highlightMain());
    pContext->setFont(indexFontID);
    pContext->setFontColor(pal.foreground());
    if (sliderWidth >= 8.0) {
      for (int i = indexL; i < indexR; ++i) {
        auto left = (i - indexL) * sliderWidth;
        auto right = left + sliderWidth - barWidth;
        pContext->drawString(
          barIndices[i].c_str(), CRect(left, height - 16, right, height - 4),
          kCenterText);
      }
    }

    // Additional index text for zoom in.
    if (value.size() != size_t(indexRange)) {
      pContext->setFontColor(pal.overlay());
      std::string str = "<- #" + std::to_string(indexL);
      pContext->drawString(str.c_str(), CRect(2, 2, 120, 30), kLeftText);
    }

    // Border.
    pContext->setLineWidth(borderWidth);
    pContext->setFrameColor(pal.border());
    pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);

    // Highlight.
    if (isMouseEntered) {
      size_t index = size_t(indexL + indexRange * mousePosition.x / width);
      if (index < value.size()) {
        pContext->setFillColor(pal.overlayHighlight());
        auto left = (index - indexL) * sliderWidth;
        pContext->drawRect(CRect(left, 0, left + sliderWidth, height), kDrawFilled);

        // Index text.
        pContext->setFont(nameFontID);
        pContext->setFontColor(pal.overlay());
        std::ostringstream os;
        os << "#" << std::to_string(index + indexOffset) << ": "
           << std::to_string(value[index]);
        indexText = os.str();
        pContext->drawString(indexText.c_str(), CRect(0, 0, width, height));
      }
    } else {
      // Title.
      pContext->setFont(nameFontID);
      pContext->setFontColor(pal.overlay());
      pContext->drawString(name.c_str(), CRect(0, 0, width, height));
    }

    // Zero line.
    auto zeroLineHeight = height - sliderZero * height;
    pContext->setFrameColor(pal.overlay());
    pContext->setLineWidth(1.0f);
    pContext->drawLine(CPoint(0, zeroLineHeight), CPoint(width, zeroLineHeight));

    setDirty(false);
  }

  CLASS_METHODS(BarBox, CView);

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
      size_t index = calcIndex(mousePosition);
      if (index >= id.size()) return kMouseEventNotHandled;

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return kMouseEventNotHandled;

      menu->popup(where.x, where.y);
      menu->release();
      return kMouseEventHandled;
    }

    setMousePosition(where);

    anchor = mousePosition;

    setValueFromPosition(mousePosition, buttons);
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    updateValue();
    pushUndoValue();
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    setMousePosition(where);
    invalid(); // Required to refresh highlighting position.
    if (buttons.isLeftButton()) {
      if (buttons.isShiftSet())
        setValueFromPosition(mousePosition, buttons);
      else
        setValueFromLine(anchor, mousePosition, buttons.getModifierState());
      anchor = mousePosition;
      return kMouseEventHandled;
    } else if (buttons.isMiddleButton()) {
      setValueFromLine(anchor, mousePosition, buttons.getModifierState());
      return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    if (isDirty()) {
      updateValue();
      pushUndoValue();
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
    if (isEditing() || axis != kMouseWheelAxisY || distance == 0.0f) return false;

    size_t index = calcIndex(mousePosition);
    if (index >= value.size()) return false;

    setValueAt(index, value[index] + distance * 0.01f);
    updateValueAt(index);
    invalid();
    return true;
  }

  int32_t onKeyDown(VstKeyCode &key) override
  {
    if (!isMouseEntered) return 1;

    size_t index = calcIndex(mousePosition);
    bool shift = key.modifier == MODIFIER_SHIFT;
    if (key.character == 'a') {
      alternateSign(index);
    } else if (key.character == 'd' && shift) { // Alternative default. (toggle min/max)
      std::fill(
        value.begin() + index, value.end(),
        value[index] == 0 ? 0.5 : value[index] == 0.5 ? 1.0 : 0.0);
    } else if (key.character == 'd') { // reset to Default.
      value = defaultValue;
    } else if (key.character == 'e' && shift) {
      emphasizeHigh(index);
    } else if (key.character == 'e') {
      emphasizeLow(index);
    } else if (key.character == 'f' && shift) {
      highpass(index);
    } else if (key.character == 'f') {
      averageLowpass(index);
    } else if (key.character == 'i' && shift) {
      invertFull(index);
    } else if (key.character == 'i') {
      invertInRange(index);
    } else if (key.character == 'n' && shift) {
      normalizeFull(index);
    } else if (key.character == 'n') {
      normalizeInRange(index);
    } else if (key.character == 'p') { // Permute.
      permute(index);
    } else if (key.character == 'r' && shift) {
      sparseRandomize(index);
    } else if (key.character == 'r') {
      totalRandomize(index);
    } else if (key.character == 's' && shift) { // Sort ascending order.
      std::sort(value.begin() + index, value.end());
    } else if (key.character == 's') { // Sort descending order.
      std::sort(value.begin() + index, value.end(), std::greater<>());
    } else if (key.character == 't' && shift) { // subTle randomize.
      mixRandomize(index, 0.02);
    } else if (key.character == 't') { // subTle randomize. Random walk.
      randomize(index, 0.02);
    } else if (key.character == 'z' && shift) { // Redo
      redo();
      ArrayControl::updateValue();
      invalid();
      return 1;
    } else if (key.character == 'z') { // Undo
      undo();
      ArrayControl::updateValue();
      invalid();
      return 1;
    } else if (key.character == ',') { // Rotate back.
      if (index == value.size() - 1) index = 0;
      std::rotate(value.begin() + index, value.begin() + index + 1, value.end());
    } else if (key.character == '.') { // Rotate forward.
      size_t rIndex = index == 0 ? 0 : value.size() - 1 - index;
      std::rotate(value.rbegin() + rIndex, value.rbegin() + rIndex + 1, value.rend());
    } else if (key.character == '1') { // Decrease.
      multiplySkip(index, 1);
    } else if (key.character == '2') { // Decrease 2n.
      multiplySkip(index, 2);
    } else if (key.character == '3') { // Decrease 3n.
      multiplySkip(index, 3);
    } else if (key.character == '4') { // Decrease 4n.
      multiplySkip(index, 4);
    } else if (key.character == '5') { // Decrease 5n.
      multiplySkip(index, 5);
    } else if (key.character == '6') { // Decrease 6n.
      multiplySkip(index, 6);
    } else if (key.character == '7') { // Decrease 7n.
      multiplySkip(index, 7);
    } else if (key.character == '8') { // Decrease 8n.
      multiplySkip(index, 8);
    } else if (key.character == '9') { // Decrease 9n.
      multiplySkip(index, 9);
    } else {
      return -1;
    }
    invalid();
    updateValue();
    pushUndoValue();
    return 1;
  }

  void pushUndoValue()
  {
    std::rotate(undoValue.begin(), undoValue.begin() + 1, undoValue.end());
    undoValue.back() = value;
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

  void setIndexFont(CFontRef fontId) { indexFontID = fontId; }
  void setNameFont(CFontRef fontId) { nameFontID = fontId; }
  void setName(std::string name) { this->name = name; }

  void setViewRange(CCoord left, CCoord right)
  {
    indexL = int(std::clamp<CCoord>(left, 0, 1) * value.size());
    indexR = int(std::clamp<CCoord>(right, 0, 1) * value.size());
    indexRange = indexR >= indexL ? indexR - indexL : 0;
    refreshSliderWidth(getWidth());
    invalid();
  }

protected:
  void alternateSign(size_t start)
  {
    for (size_t i = start; i < value.size(); i += 2)
      setValueAt(i, 2 * sliderZero - value[i]);
  }

  void averageLowpass(size_t start)
  {
    const int32_t range = 1;

    std::vector<double> result(value);
    for (size_t i = start; i < value.size(); ++i) {
      result[i] = 0.0;
      for (int32_t j = -range; j <= range; ++j) {
        size_t index = i + j; // Note that index is unsigned.
        if (index >= value.size()) continue;
        result[i] += value[index] - sliderZero;
      }
      setValueAt(i, sliderZero + result[i] / double(2 * range + 1));
    }
  }

  /**
  Highpass equation is:
    `value[i] = sum((-0.5, 1.0, -0.5) * value[(i - 1, i, i + 1)])`
  Value of index outside of array is assumed to be same as closest element.
  */
  void highpass(size_t start)
  {
    std::vector<double> result(value);
    size_t last = value.size() - 1;
    for (size_t i = start; i < value.size(); ++i) {
      auto val = value[i] - sliderZero;
      result[i] = 0.0;
      result[i] -= (i >= 1) ? value[i - 1] - sliderZero : val;
      result[i] -= (i < last) ? value[i + 1] - sliderZero : val;
      result[i] = val + 0.5f * result[i];
      setValueAt(i, sliderZero + result[i]);
    }
  }

  void totalRandomize(size_t start)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i) value[i] = dist(rng);
  }

  void randomize(size_t start, double amount)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    amount /= 2;
    for (size_t i = start; i < value.size(); ++i) {
      std::uniform_real_distribution<double> dist(value[i] - amount, value[i] + amount);
      setValueAt(i, dist(rng));
    }
  }

  void mixRandomize(size_t start, double mix)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(sliderZero - 0.5, sliderZero + 0.5);
    for (size_t i = start; i < value.size(); ++i)
      setValueAt(i, value[i] + mix * (dist(rng) - value[i]));
  }

  void sparseRandomize(size_t start)
  {
    std::random_device device;
    std::mt19937_64 rng(device());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i)
      if (dist(rng) < 0.1f) value[i] = dist(rng);
  }

  void permute(size_t start)
  {
    std::random_device device;
    std::mt19937 rng(device());
    std::shuffle(value.begin() + start, value.end(), rng);
  }

  struct ValuePeak {
    double minNeg = 2;
    double minPos = 2;
    double maxNeg = -1;
    double maxPos = -1;
  };

  ValuePeak getValuePeak(size_t start, bool skipZero)
  {
    ValuePeak pk;
    for (size_t i = start; i < value.size(); ++i) {
      double val = fabs(value[i] - sliderZero);
      if (value[i] == sliderZero) {
        if (skipZero) continue;
        pk.minNeg = 0;
        pk.minPos = 0;
      } else if (value[i] < sliderZero) {
        if (val > pk.maxNeg)
          pk.maxNeg = val;
        else if (val < pk.minNeg)
          pk.minNeg = val;
      } else {
        if (val > pk.maxPos)
          pk.maxPos = val;
        else if (val < pk.minPos)
          pk.minPos = val;
      }
    }
    if (pk.minNeg > 1.0) pk.minNeg = 0.0;
    if (pk.minPos > 1.0) pk.minPos = 0.0;
    if (pk.maxNeg < 0.0) pk.maxNeg = 0.0;
    if (pk.maxPos < 0.0) pk.maxPos = 0.0;
    return pk;
  }

  void invertFull(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i) {
      double val
        = value[i] >= sliderZero ? 1.0 - value[i] + sliderZero : sliderZero - value[i];
      setValueAt(i, val);
    }
  }

  void invertInRange(size_t start)
  {
    auto pk = getValuePeak(start, false);
    for (size_t i = start; i < value.size(); ++i) {
      double val = value[i] < sliderZero
        ? std::clamp<double>(
          2.0 * sliderZero - pk.maxNeg - value[i] - pk.minNeg, sliderZero - pk.maxNeg,
          sliderZero)
        : std::clamp<double>(
          2.0 * sliderZero + pk.maxPos - value[i] + pk.minPos, sliderZero,
          pk.maxPos + sliderZero);
      setValueAt(i, val);
    }
  }

  void normalizeFull(size_t start)
  {
    auto pk = getValuePeak(start, true);

    double diffNeg = pk.maxNeg - pk.minNeg;
    double diffPos = pk.maxPos - pk.minPos;

    double mulNeg = sliderZero / diffNeg;
    double mulPos = (1.0 - sliderZero) / diffPos;

    double fixNeg = sliderZero;
    double fixPos = sliderZero;

    if (diffNeg == 0.0) {
      mulNeg = 0.0;
      if (pk.maxNeg != 0.0) fixNeg = 0.0;
    }
    if (diffPos == 0.0) {
      mulPos = 0.0;
      if (pk.maxPos != 0.0) fixPos = 1.0;
    }

    for (size_t i = start; i < value.size(); ++i) {
      if (value[i] == sliderZero) continue;
      double val = value[i] < sliderZero
        ? std::min<double>(
          (value[i] - sliderZero + pk.minNeg) * mulNeg + fixNeg, sliderZero)
        : std::max<double>(
          (value[i] - sliderZero - pk.minPos) * mulPos + fixPos, sliderZero);
      setValueAt(i, val);
    }
  }

  void normalizeInRange(size_t start) noexcept
  {
    auto pk = getValuePeak(start, true);

    double diffNeg = pk.maxNeg - pk.minNeg;
    double diffPos = pk.maxPos - pk.minPos;

    double mulNeg = (sliderZero - pk.minNeg) / diffNeg;
    double mulPos = (1.0 - sliderZero - pk.minPos) / diffPos;

    if (diffNeg == 0.0) {
      mulNeg = 0.0;
      pk.minNeg = pk.maxNeg == 0.0 ? 0.0 : 1.0;
    }
    if (diffPos == 0.0) {
      mulPos = 0.0;
      pk.minPos = pk.maxPos == 0.0 ? 0.0 : 1.0;
    }

    for (size_t i = start; i < value.size(); ++i) {
      if (value[i] == sliderZero) continue;
      auto val = value[i] < sliderZero
        ? (value[i] - sliderZero + pk.minNeg) * mulNeg + sliderZero - pk.minNeg
        : (value[i] - sliderZero - pk.minPos) * mulPos + sliderZero + pk.minPos;
      setValueAt(i, val);
    }
  }

  void multiplySkip(size_t start, size_t interval) noexcept
  {
    for (size_t i = start; i < value.size(); i += interval) {
      setValueAt(i, (value[i] - sliderZero) * 0.9 + sliderZero);
    }
  }

  void emphasizeLow(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i)
      setValueAt(i, (value[i] - sliderZero) / pow(i + 1, 0.0625) + sliderZero);
  }

  void emphasizeHigh(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i)
      setValueAt(
        i,
        (value[i] - sliderZero) * (0.9 + 0.1 * double(i + 1) / value.size())
          + sliderZero);
  }

  void setValueFromPosition(CPoint &position, const CButtonState &buttons)
  {
    size_t index = calcIndex(position);
    if (index >= value.size()) return;
    if (buttons & kControl)
      setValueAt(index, defaultValue[index]);
    else
      setValueAt(index, 1.0 - double(position.y) / getHeight());
    updateValueAt(index);
    invalid();
  }

  void setValueFromLine(CPoint p0, CPoint p1, int32_t modifier)
  {
    if (p0.x > p1.x) std::swap(p0, p1);

    size_t left = calcIndex(p0);
    size_t right = calcIndex(p1);
    if (left >= value.size() || right >= value.size()) return;

    const float p0y = p0.y;
    const float p1y = p1.y;

    if (left == right) { // p0 and p1 are in a same bar.
      if (modifier & kControl)
        setValueAt(left, defaultValue[left]);
      else
        setValueAt(left, 1.0f - (p0y + p1y) * 0.5f / getHeight());
      updateValueAt(left);
      invalid();
      return;
    } else if (modifier & kControl) {
      for (size_t idx = left; idx >= 0 && idx <= right; ++idx)
        setValueAt(idx, defaultValue[idx]);
      if (liveUpdateLineEdit) updateValue();
      return;
    }

    const float xL = sliderWidth * (left + 1);
    const float xR = sliderWidth * right;

    if (fabs(xR - xL) >= 1e-5) {
      p0.x = xL;
      p1.x = xR;
    }

    setValueAt(left, 1.0f - p0y / getHeight());
    setValueAt(right, 1.0f - p1y / getHeight());

    // In between.
    const float p0x = p0.x;
    const float p1x = p1.x;
    const float slope = (p1y - p0y) / (p1x - p0x);
    const float yInc = slope * sliderWidth;
    float y = slope * (sliderWidth * (left + 1) - p0x) + p0y;
    for (size_t idx = left + 1; idx < right; ++idx) {
      setValueAt(idx, 1.0f - (y + 0.5f * yInc) / getHeight());
      y += yInc;
    }

    if (liveUpdateLineEdit) updateValue();
    invalid();
  }

  void setMousePosition(CPoint &where)
  {
    auto view = getViewSize();
    mousePosition = where - CPoint(view.left, view.top);
  }

  inline size_t calcIndex(CPoint &position)
  {
    return size_t(indexL + position.x / sliderWidth);
  }

  void refreshSliderWidth(CCoord width)
  {
    sliderWidth = indexRange >= 1 ? width / indexRange : width;
    barWidth = sliderWidth <= 4.0f ? 1.0f : 2.0f;
  }

  CFontRef indexFontID = nullptr;
  CFontRef nameFontID = nullptr;

  CCoord borderWidth = 1.0;

  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CPoint anchor{-1.0, -1.0};
  int indexL = 0;
  int indexR = 0;
  int indexRange = 0;
  CCoord sliderWidth = 1.0f;
  CCoord barWidth = 1.0f;

  std::string name;
  std::string indexText;
  std::vector<std::string> barIndices;
  std::vector<std::vector<double>> undoValue;

  Uhhyou::Palette &pal;
}; // namespace VSTGUI

} // namespace VSTGUI
