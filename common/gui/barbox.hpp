// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "arraycontrol.hpp"
#include "style.hpp"

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <string>

namespace VSTGUI {

template<typename Scale> class BarBox : public ArrayControl {
private:
  enum class BarState : uint8_t { active, lock };

  SharedPointer<CFontDesc> indexFontID;
  SharedPointer<CFontDesc> nameFontID;

  CCoord borderWidth = 1.0;

  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CPoint anchor{-1.0, -1.0};
  BarState anchorState = BarState::active;
  int indexL = 0;
  int indexR = 0;
  int indexRange = 0;
  CCoord sliderWidth = 1.0f;
  CCoord barWidth = 1.0f;

  std::string name;
  std::string indexText;
  std::vector<std::string> barIndices;
  std::vector<std::vector<double>> undoValue;
  std::vector<BarState> barState;
  std::vector<double> active, locked; // Used to store temporary values.

  Uhhyou::Palette &pal;

  Scale &scale;

public:
  double sliderZero = 0;
  int32_t indexOffset = 0;
  bool liveUpdateLineEdit = true; // Set this false when line edit is slow.
  double scrollSensitivity = 0.01;
  double altScrollSensitivity = 0.001;
  std::vector<double> snapValue;

  explicit BarBox(
    Steinberg::Vst::VSTGUIEditor *editor,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    Scale &scale,
    std::vector<double> value,
    std::vector<double> defaultValue,
    Uhhyou::Palette &palette)
    : ArrayControl(editor, size, id, value, defaultValue)
    , sliderWidth((size.right - size.left) / value.size())
    , pal(palette)
    , scale(scale)
  {
    setWantsFocus(true);

    for (size_t i = 0; i < id.size(); ++i)
      barIndices.push_back(std::to_string(i + indexOffset));

    setViewRange(0, 1);

    for (size_t i = 0; i < 4; ++i) undoValue.emplace_back(defaultValue);

    barState.resize(defaultValue.size(), BarState::active);
    active.reserve(value.size());
    locked.reserve(value.size());
  }

  CLASS_METHODS(BarBox, CView);

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();
    const auto sc = pal.guiScale();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Background.
    pContext->setFillColor(pal.boxBackground());
    pContext->drawRect(CRect(0, 0, width, height), kDrawFilled);

    // Value bar.
    float sliderZeroHeight = height * (1.0 - sliderZero);
    for (int i = indexL; i < indexR; ++i) {
      auto left = (i - indexL) * sliderWidth;
      auto right = left + sliderWidth - barWidth;
      auto top = height - value[i] * height;
      double bottom = sliderZeroHeight;
      if (top > bottom) std::swap(top, bottom);
      pContext->setFillColor(
        barState[i] == BarState::active ? pal.highlightMain() : pal.foregroundInactive());
      pContext->drawRect(CRect(left, top, right, bottom), kDrawFilled);
    }

    // Index text.
    pContext->setFrameColor(pal.highlightMain());
    pContext->setFont(indexFontID);
    pContext->setFontColor(pal.foreground());
    if (sliderWidth >= sc * 12.0) {
      for (int i = indexL; i < indexR; ++i) {
        auto left = (i - indexL) * sliderWidth;
        auto right = left + sliderWidth - barWidth;
        pContext->drawString(
          barIndices[i].c_str(), CRect(left, height - 16, right, height - 4), kCenterText,
          true);
        if (barState[i] != BarState::active)
          pContext->drawString("L", CRect(left, 0, right, 16), kCenterText, true);
      }
    }

    // Additional index text for zoom in.
    if (value.size() != size_t(indexRange)) {
      pContext->setFontColor(pal.overlay());
      std::string str = "<- #" + std::to_string(indexL);
      pContext->drawString(str.c_str(), CRect(2, 2, 120, 30), kLeftText);
    }

    // Border.
    const auto borderW = int(sc * borderWidth);
    const auto halfBorderW = int(borderW / 2);
    pContext->setLineWidth(borderW);
    pContext->setFrameColor(pal.border());
    pContext->drawRect(CRect(halfBorderW, halfBorderW, width, height), kDrawStroked);

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
           << std::to_string(scale.map(value[index]));
        indexText = os.str();
        pContext->drawString(indexText.c_str(), CRect(0, 0, width, height));

        if (barState[index] != BarState::active) {
          pContext->setFont(indexFontID);
          pContext->drawString("Locked", CRect(0, 20, width, 40));
        }
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

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    releaseFocus();

    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (event.buttonState.isRight()) {
      auto componentHandler = editor->getController()->getComponentHandler();
      if (componentHandler == nullptr) return;

      using namespace Steinberg;

      FUnknownPtr<Vst::IComponentHandler3> handler(componentHandler);
      if (handler == nullptr) return;

      setMousePosition(event.mousePosition);
      size_t index = calcIndex(mousePosition);
      if (index >= id.size()) return;

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return;

      menu->popup(event.mousePosition.x, event.mousePosition.y);
      menu->release();
      event.consumed = true;
      return;
    }

    grabFocus();

    setMousePosition(event.mousePosition);
    anchor = mousePosition;

    if (
      event.buttonState.isMiddle() && event.modifiers.has(ModifierKey::Control)
      && event.modifiers.has(ModifierKey::Shift))
    {
      anchorState = setStateFromPosition(mousePosition, BarState::lock);
    } else {
      setValueFromPosition(mousePosition, event.modifiers);
    }
    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    updateValue();
    endEdit();
    pushUndoValue();
    event.consumed = true;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    setMousePosition(event.mousePosition);
    if (event.buttonState.isLeft()) {
      if (
        event.modifiers.has(ModifierKey::Control)
        && event.modifiers.has(ModifierKey::Shift))
      {
        setValueFromPosition(mousePosition, event.modifiers);
      } else {
        setValueFromLine(anchor, mousePosition, event.modifiers);
      }
      anchor = mousePosition;
      event.consumed = true;
    } else if (event.buttonState.isMiddle()) {
      if (
        event.modifiers.has(ModifierKey::Control)
        && event.modifiers.has(ModifierKey::Shift))
      {
        setStateFromLine(anchor, mousePosition, anchorState);
      } else if (event.modifiers.has(ModifierKey::Shift)) {
        mousePosition.x = anchor.x;
        setValueFromPosition(mousePosition, false, false);
      } else {
        setValueFromLine(anchor, mousePosition, event.modifiers);
      }
      event.consumed = true;
    }
    invalid(); // Required to refresh highlighting position.
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isDirty()) {
      updateValue();
      endEdit();
      pushUndoValue();
      invalid();
    }
    event.consumed = true;
  }

  void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    if (event.deltaY == 0) return;

    grabFocus();

    size_t index = calcIndex(mousePosition);
    if (index >= value.size()) return;

    if (barState[index] != BarState::active) {
      event.consumed = true;
      return;
    }

    if (event.modifiers.has(ModifierKey::Shift)) {
      setValueAtIndex(index, value[index] + event.deltaY * altScrollSensitivity);
    } else {
      setValueAtIndex(index, value[index] + event.deltaY * scrollSensitivity);
    }
    beginEdit(index);
    updateValueAt(index);
    endEdit(index);
    invalid();
    event.consumed = true;
  }

  void onKeyboardEvent(KeyboardEvent &event) override
  {
    if (!isMouseEntered || event.type == EventType::KeyUp) return;

    size_t index = calcIndex(mousePosition);
    if (index >= value.size()) index = value.size() - 1;

    auto shift = event.modifiers.has(ModifierKey::Shift);
    if (event.character == 'a') {
      alternateSign(index);
    } else if (shift && event.character == 'd') { // Alternative default. (toggle min/max)
      toggleMinMidMax(index);
    } else if (event.character == 'd') { // reset to Default.
      resetToDefault();
    } else if (shift && event.character == 'e') {
      emphasizeHigh(index);
    } else if (event.character == 'e') {
      emphasizeLow(index);
    } else if (shift && event.character == 'f') {
      highpass(index);
    } else if (event.character == 'f') {
      averageLowpass(index);
    } else if (shift && event.character == 'i') {
      invertInRange(index);
    } else if (event.character == 'i') {
      invertFull(index);
    } else if (shift && event.character == 'l') {
      lockAll(index);
    } else if (event.character == 'l') {
      barState[index]
        = barState[index] == BarState::active ? BarState::lock : BarState::active;
    } else if (shift && event.character == 'n') {
      normalizeFull(index);
    } else if (event.character == 'n') {
      normalizeInRange(index);
    } else if (event.character == 'p') { // Permute.
      applyAlgorithm(index, [&]() {
        std::random_device device;
        std::mt19937 rng(device());
        std::shuffle(active.begin(), active.end(), rng);
      });
    } else if (shift && event.character == 'r') {
      sparseRandomize(index);
    } else if (event.character == 'r') {
      totalRandomize(index);
    } else if (shift && event.character == 'S') { // Sort ascending order.
      applyAlgorithm(index, [&]() { std::sort(active.begin(), active.end()); });
    } else if (event.character == 's') { // Sort descending order.
      applyAlgorithm(
        index, [&]() { std::sort(active.begin(), active.end(), std::greater<>()); });
    } else if (shift && event.character == 't') { // subTle randomize.
      mixRandomize(index, 0.02);
    } else if (event.character == 't') { // subTle randomize. Random walk.
      randomize(index, 0.02);
    } else if (shift && event.character == 'z') { // Redo
      redo();
      ArrayControl::editAndUpdateValue();
      invalid();
      event.consumed = true;
      return;
    } else if (event.character == 'z') { // Undo
      undo();
      ArrayControl::editAndUpdateValue();
      invalid();
      event.consumed = true;
      return;
    } else if (event.character == ',') { // Rotate back.
      applyAlgorithm(
        index, [&]() { std::rotate(active.begin(), active.begin() + 1, active.end()); });
    } else if (event.character == '.') { // Rotate forward.
      applyAlgorithm(index, [&]() {
        std::rotate(active.rbegin(), active.rbegin() + 1, active.rend());
      });
    } else if (event.character == '1') { // Decrease.
      multiplySkip(index, 1);
    } else if (event.character == '2') { // Decrease 2n.
      multiplySkip(index, 2);
    } else if (event.character == '3') { // Decrease 3n.
      multiplySkip(index, 3);
    } else if (event.character == '4') { // Decrease 4n.
      multiplySkip(index, 4);
    } else if (event.character == '5') { // Decimate and hold 2 samples.
      decimateHold(index, 2);
    } else if (event.character == '6') { // Decimate and hold 3 samples.
      decimateHold(index, 3);
    } else if (event.character == '7') { // Decimate and hold 4 samples.
      decimateHold(index, 4);
    } else if (event.character == '8') { // Decimate and hold 5 samples.
      decimateHold(index, 5);
    } else if (event.character == '9') { // Decimate and hold 6 samples.
      decimateHold(index, 6);
    } else {
      event.consumed = true;
      return;
    }
    invalid();
    editAndUpdateValue();
    pushUndoValue();
    event.consumed = true;
  }

  void setIndexFont(const SharedPointer<CFontDesc> &fontId) { indexFontID = fontId; }
  void setNameFont(const SharedPointer<CFontDesc> &fontId) { nameFontID = fontId; }
  void setName(std::string name) { this->name = name; }

  void setViewRange(CCoord left, CCoord right)
  {
    indexL = int(std::clamp<CCoord>(left, 0, 1) * value.size());
    indexR = int(std::clamp<CCoord>(right, 0, 1) * value.size());
    indexRange = indexR >= indexL ? indexR - indexL : 0;
    refreshSliderWidth(getWidth());
    invalid();
  }

private:
  void setMousePosition(CPoint &pos)
  {
    auto view = getViewSize();
    mousePosition = pos - CPoint(view.left, view.top);
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

  double snap(double val)
  {
    if (snapValue.size() <= 0) return val;

    size_t idx = 0;
    for (; idx < snapValue.size(); ++idx) {
      if (snapValue[idx] < val) continue;
      break;
    }
    return idx < snapValue.size() ? snapValue[idx] : 1.0;
  }

  BarState setStateFromPosition(CPoint &position, BarState state)
  {
    size_t index = calcIndex(position);
    if (index >= value.size()) return BarState::active;

    barState[index] = barState[index] != state ? state : BarState::active;
    return barState[index];
  }

  void setStateFromLine(CPoint &p0, CPoint &p1, BarState state)
  {
    if (p0.x > p1.x) std::swap(p0, p1);

    int last = int(value.size()) - 1;
    if (last < 0) last = 0; // std::clamp is undefined if low is greater than high.

    int left = int(calcIndex(p0));
    int right = int(calcIndex(p1));

    if ((left < 0 && right < 0) || (left > last && right > last)) return;

    left = std::clamp(left, 0, last);
    right = std::clamp(right, 0, last);

    for (int idx = left; idx >= 0 && idx <= right; ++idx) barState[idx] = state;

    invalid();
  }

  void setValueFromPosition(CPoint &position, const Modifiers &modifiers)
  {
    setValueFromPosition(
      position, modifiers.has(ModifierKey::Control), modifiers.has(ModifierKey::Shift));
  }

  void setValueFromPosition(CPoint &position, const bool ctrl, const bool shift)
  {
    size_t index = calcIndex(position);
    if (index >= value.size()) return;
    if (barState[index] != BarState::active) return;

    if (ctrl && !shift)
      setValueAtIndex(index, defaultValue[index]);
    else if (!ctrl && shift)
      setValueAtIndex(index, snap(1.0 - position.y / getHeight()));
    else
      setValueAtIndex(index, 1.0 - position.y / getHeight());

    updateValueAt(index);
    invalid();
  }

  void setValueAtIndex(size_t index, double normalized)
  {
    if (barState[index] != BarState::active) return;
    beginEdit(index);

    if (index >= value.size()) return;
    value[index] = normalized < 0.0 ? 0.0 : normalized > 1.0 ? 1.0 : normalized;
  }

  void setValueFromLine(CPoint p0, CPoint p1, const Modifiers &modifiers)
  {
    if (p0.x > p1.x) std::swap(p0, p1);

    size_t left = calcIndex(p0);
    size_t right = calcIndex(p1);
    if (left >= value.size() || right >= value.size()) return;

    const float p0y = p0.y;
    const float p1y = p1.y;

    if (left == right) { // p0 and p1 are in a same bar.
      if (barState[left] != BarState::active) return;

      if (modifiers.has(ModifierKey::Control))
        setValueAtIndex(left, defaultValue[left]);
      else if (modifiers.has(ModifierKey::Shift))
        setValueAtIndex(left, snap(1.0f - anchor.y / getHeight()));
      else
        setValueAtIndex(left, 1.0f - anchor.y / getHeight());

      updateValueAt(left);
      invalid();
      return;
    } else if (modifiers.has(ModifierKey::Control)) {
      for (size_t idx = left; idx >= 0 && idx <= right; ++idx) {
        if (barState[left] != BarState::active) return;
        setValueAtIndex(idx, defaultValue[idx]);
      }
      if (liveUpdateLineEdit) updateValue();
      return;
    }

    const bool isSnapping = modifiers.has(ModifierKey::Shift);

    if (barState[left] == BarState::active) {
      auto val = 1.0f - p0y / getHeight();
      setValueAtIndex(left, isSnapping ? snap(val) : val);
    }
    if (barState[right] == BarState::active) {
      auto val = 1.0f - p1y / getHeight();
      setValueAtIndex(right, isSnapping ? snap(val) : val);
    }

    // In between.
    const float xL = sliderWidth * (left + 1);
    const float xR = sliderWidth * right;

    if (fabs(xR - xL) >= 1e-5) { // Avoid 0 division on slope.
      p0.x = xL;
      p1.x = xR;
    }

    const float p0x = p0.x;
    const float p1x = p1.x;
    const float slope = (p1y - p0y) / (p1x - p0x);
    const float yInc = slope * sliderWidth;
    float y = slope * (sliderWidth * (left + 1) - p0x) + p0y;
    for (size_t idx = left + 1; idx < right; ++idx) {
      auto val = 1.0f - (y + 0.5f * yInc) / getHeight();
      setValueAtIndex(idx, isSnapping ? snap(val) : val);
      y += yInc;
    }

    if (liveUpdateLineEdit) updateValue();
    invalid();
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

  template<typename Func> void applyAlgorithm(size_t start, Func func)
  {
    active.resize(0);
    locked.resize(0);

    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] == BarState::active)
        active.push_back(value[i]);
      else
        locked.push_back(value[i]);
    }

    func();

    size_t activeIndex = 0;
    size_t lockedIndex = 0;
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] == BarState::active) {
        value[i] = active[activeIndex];
        ++activeIndex;
      } else {
        value[i] = locked[lockedIndex];
        ++lockedIndex;
      }
    }
  }

  void resetToDefault()
  {
    for (size_t i = 0; i < value.size(); ++i) {
      if (barState[i] == BarState::active) value[i] = defaultValue[i];
    }
  }

  void toggleMinMidMax(size_t start)
  {
    double filler = 0;
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      filler = value[i] == 0 ? 0.5 : value[i] == 0.5 ? 1.0 : 0.0;
      start = i;
      break;
    }

    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] == BarState::active) value[i] = filler;
    }
  }

  void lockAll(size_t index)
  {
    std::fill(
      barState.begin(), barState.end(),
      barState[index] == BarState::active ? BarState::lock : BarState::active);
  }

  void alternateSign(size_t start)
  {
    for (size_t i = start; i < value.size(); i += 2) {
      if (barState[i] != BarState::active) continue;
      setValueAtIndex(i, 2 * sliderZero - value[i]);
    }
  }

  void averageLowpass(size_t start)
  {
    const int32_t range = 1;

    std::vector<double> result(value);
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      result[i] = 0.0;
      for (int32_t j = -range; j <= range; ++j) {
        size_t index = i + j; // Note that index is unsigned.
        if (index >= value.size()) continue;
        result[i] += value[index] - sliderZero;
      }
      setValueAtIndex(i, sliderZero + result[i] / double(2 * range + 1));
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
      if (barState[i] != BarState::active) continue;
      auto val = value[i] - sliderZero;
      result[i] = 0.0;
      result[i] -= (i >= 1) ? value[i - 1] - sliderZero : val;
      result[i] -= (i < last) ? value[i + 1] - sliderZero : val;
      result[i] = val + 0.5f * result[i];
      setValueAtIndex(i, sliderZero + result[i]);
    }
  }

  void totalRandomize(size_t start)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      value[i] = dist(rng);
    }
  }

  void randomize(size_t start, double amount)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    amount /= 2;
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      std::uniform_real_distribution<double> dist(value[i] - amount, value[i] + amount);
      setValueAtIndex(i, dist(rng));
    }
  }

  void mixRandomize(size_t start, double mix)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(sliderZero - 0.5, sliderZero + 0.5);
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      setValueAtIndex(i, value[i] + mix * (dist(rng) - value[i]));
    }
  }

  void sparseRandomize(size_t start)
  {
    std::random_device device;
    std::mt19937_64 rng(device());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      if (dist(rng) < 0.1f) value[i] = dist(rng);
    }
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
      if (barState[i] != BarState::active) continue;
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

  void invertInRange(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      double val
        = value[i] >= sliderZero ? 1.0 - value[i] + sliderZero : sliderZero - value[i];
      setValueAtIndex(i, val);
    }
  }

  void invertFull(size_t start)
  {
    auto pk = getValuePeak(start, false);
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;

      if (value[i] < sliderZero) {
        auto x = 1.0 + value[i] - (value[i] / sliderZero);
        setValueAtIndex(i, std::clamp(x, sliderZero, 1.0));
      } else {
        auto x = sliderZero - sliderZero * (value[i] - sliderZero) / (1.0 - sliderZero);
        setValueAtIndex(i, std::clamp(x, 0.0, sliderZero));
      }

      // setValueAtIndex(i, 1.0 - value[i]);
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
      if (barState[i] != BarState::active) continue;
      if (value[i] == sliderZero) continue;
      double val = value[i] < sliderZero
        ? std::min<double>(
            (value[i] - sliderZero + pk.minNeg) * mulNeg + fixNeg, sliderZero)
        : std::max<double>(
            (value[i] - sliderZero - pk.minPos) * mulPos + fixPos, sliderZero);
      setValueAtIndex(i, val);
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
      if (barState[i] != BarState::active) continue;
      if (value[i] == sliderZero) continue;
      auto val = value[i] < sliderZero
        ? (value[i] - sliderZero + pk.minNeg) * mulNeg + sliderZero - pk.minNeg
        : (value[i] - sliderZero - pk.minPos) * mulPos + sliderZero + pk.minPos;
      setValueAtIndex(i, val);
    }
  }

  void multiplySkip(size_t start, size_t interval) noexcept
  {
    for (size_t i = start; i < value.size(); i += interval) {
      if (barState[i] != BarState::active) continue;
      setValueAtIndex(i, (value[i] - sliderZero) * 0.9 + sliderZero);
    }
  }

  void decimateHold(size_t start, size_t interval)
  {
    size_t counter = 0;
    double hold = 0;
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;

      if (counter == 0) hold = value[i];
      counter = (counter + 1) % interval;
      setValueAtIndex(i, hold);
    }
  }

  void emphasizeLow(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      setValueAtIndex(i, (value[i] - sliderZero) / std::pow(i + 1, 0.0625) + sliderZero);
    }
  }

  void emphasizeHigh(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i) {
      if (barState[i] != BarState::active) continue;
      auto &&emphasis = 0.9 + 0.1 * double(i + 1) / value.size();
      setValueAtIndex(i, (value[i] - sliderZero) * emphasis + sliderZero);
    }
  }
};

} // namespace VSTGUI
