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

#include <algorithm>
#include <random>

namespace VSTGUI {

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
    std::vector<double> defaultValue)
    : ArrayControl(editor, size, id, value, defaultValue)
    , sliderWidth((size.right - size.left) / value.size())
  {
    setWantsFocus(true);

    for (size_t i = 0; i < id.size(); ++i)
      barIndices.push_back(std::to_string(i + indexOffset));
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

    // Value bar.
    pContext->setFillColor(valueColor);
    pContext->setLineWidth(1.0f);

    float sliderZeroHeight = height * (1.0f - sliderZero);
    for (size_t i = 0; i < value.size(); ++i) {
      auto left = i * sliderWidth;
      auto right = left + (sliderWidth >= 4.0 ? sliderWidth - defaultBorderWidth : 1.0);

      auto top = height - value[i] * height;
      double bottom = sliderZeroHeight;
      if (top > bottom) std::swap(top, bottom);
      pContext->drawRect(CRect(left, top, right, bottom), kDrawFilled);
    }

    // Splitter.
    if (sliderWidth >= 4.0) {
      pContext->setLineWidth(defaultBorderWidth);
      pContext->setFrameColor(splitterColor);
      for (size_t i = 0; i < value.size(); ++i) {
        auto x = i * sliderWidth;
        pContext->drawLine(CPoint(x, 0), CPoint(x, height));
      }
    }

    // Bar index.
    pContext->setFrameColor(valueColor);
    pContext->setFont(indexFontID);
    pContext->setFontColor(borderColor);
    if (sliderWidth >= 10.0) {
      for (size_t i = 0; i < barIndices.size(); ++i) {
        auto left = i * sliderWidth;
        auto right = left + (sliderWidth >= 4.0 ? sliderWidth - defaultBorderWidth : 1.0);
        pContext->drawString(
          barIndices[i].c_str(), CRect(left, height - 16, right, height - 4),
          kCenterText);
      }
    }

    // Border.
    pContext->setFrameColor(borderColor);
    pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);

    // Highlight.
    if (isMouseEntered || isMouseLeftDown) {
      size_t index = size_t(value.size() * mousePosition.x / width);
      if (index < value.size()) {
        pContext->setFillColor(highlightColor);
        auto left = index * sliderWidth;
        pContext->drawRect(CRect(left, 0, left + sliderWidth, height), kDrawFilled);

        // Index text.
        pContext->setFont(nameFontID);
        pContext->setFontColor(nameColor);
        std::ostringstream os;
        os << "#" << std::to_string(index + indexOffset) << ": "
           << std::to_string(value[index]);
        indexText = os.str();
        pContext->drawString(indexText.c_str(), CRect(0, 0, width, height));
      }
    } else {
      // Title.
      pContext->setFont(nameFontID);
      pContext->setFontColor(nameColor);
      pContext->drawString(name.c_str(), CRect(0, 0, width, height));
    }

    // Zero line.
    auto zeroLineHeight = height - sliderZero * height;
    pContext->setFrameColor(nameColor);
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
      size_t index = size_t(mousePosition.x / sliderWidth);

      Vst::IContextMenu *menu = handler->createContextMenu(editor, &id[index]);
      if (menu == nullptr) return kMouseEventNotHandled;

      menu->popup(where.x, where.y);
      menu->release();
      return kMouseEventHandled;
    }

    setMousePosition(where);
    if (buttons.isLeftButton()) {
      isMouseLeftDown = true;
    } else if (buttons.isMiddleButton()) {
      isMouseMiddleDown = true;
      anchor = mousePosition;
    }
    setValueFromPosition(mousePosition, buttons);
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override
  {
    if (buttons.isLeftButton()) isMouseLeftDown = false;
    if (buttons.isMiddleButton()) isMouseMiddleDown = false;
    return kMouseEventHandled;
  }

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override
  {
    setMousePosition(where);
    invalid(); // Required to refresh highlighting position.
    if (buttons.isLeftButton()) {
      setValueFromPosition(mousePosition, buttons);
      return kMouseEventHandled;
    } else if (buttons.isMiddleButton()) {
      setValueFromLine(anchor, mousePosition);
      if (liveUpdateLineEdit) updateValue();
      return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
  }

  CMouseEventResult onMouseCancel() override
  {
    if (isDirty()) {
      updateValue();
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
    if (key.character == 'd' && shift) { // Alternative default. (toggle min/max)
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
      invert(index, false);
    } else if (key.character == 'i') {
      invert(index, true);
    } else if (key.character == 'n' && shift) {
      normalize(index, false);
    } else if (key.character == 'n') {
      normalize(index, true);
    } else if (key.character == 'p') { // Permute.
      permute(index);
    } else if (key.character == 'r' && shift) {
      sparseRandomize(index);
    } else if (key.character == 'r') {
      randomize(index, 1.0);
    } else if (key.character == 's' && shift) { // Sort ascending order.
      std::sort(value.begin() + index, value.end());
    } else if (key.character == 's') { // Sort descending order.
      std::sort(value.begin() + index, value.end(), std::greater<>());
    } else if (key.character == 't') { // subTle randomize.
      randomize(index, 0.02);
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
    return 1;
  }

  void setIndexFont(CFontRef fontID) { indexFontID = fontID; }
  void setNameFont(CFontRef fontID) { nameFontID = fontID; }
  void setHighlightColor(CColor color) { highlightColor = color; }
  void setValueColor(CColor color) { valueColor = color; }
  void setBorderColor(CColor color) { borderColor = color; }
  void setDefaultBorderWidth(float width) { defaultBorderWidth = width; }
  void setName(std::string name) { this->name = name; }

protected:
  void averageLowpass(size_t start)
  {
    const int32_t range = 1;

    std::vector<double> result(value);
    for (size_t i = start; i < value.size(); ++i) {
      result[i] = 0.0;
      for (int32_t j = -range; j <= range; ++j) {
        size_t index = i + j; // Note that index is unsigned.
        if (index >= value.size()) continue;
        result[i] += value[index];
      }
      setValueAt(i, result[i] / double(2 * range + 1));
    }
  }

  void highpass(size_t start)
  {
    // value[i] = sum((-0.5, 1.0, -0.5) * value[(i - 1, i, i + 1)])
    // Value of index outside of array is assumed to be same as closest element.
    std::vector<double> result(value);
    size_t last = value.size() - 1;
    for (size_t i = start; i < value.size(); ++i) {
      result[i] = 0.0;
      result[i] -= (i >= 1) ? value[i - 1] : value[i];
      result[i] -= (i < last) ? value[i + 1] : value[i];
      result[i] = value[i] + 0.5f * result[i];
      setValueAt(i, result[i]);
    }
  }

  void randomize(size_t start, double mix)
  {
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i)
      value[i] += mix * (dist(rng) - value[i]);
  }

  void sparseRandomize(size_t start)
  {
    std::random_device device;
    std::mt19937_64 rng(device());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (size_t i = start; i < value.size(); ++i)
      if (dist(rng) < 0.1) value[i] = dist(rng);
  }

  void permute(size_t start)
  {
    std::random_device device;
    std::mt19937 rng(device());
    std::shuffle(value.begin() + start, value.end(), rng);
  }

  void invert(size_t start, bool preserveMin)
  {
    auto min = preserveMin ? *(std::min_element(value.begin(), value.end())) : 0.0;
    for (size_t i = start; i < value.size(); ++i) setValueAt(i, 1.0f - value[i] + min);
  }

  void normalize(size_t start, bool preserveMin) noexcept
  {
    auto begin = start < value.size() ? value.begin() + start : value.begin();
    auto min = preserveMin ? 0.0 : *(std::min_element(begin, value.end()));
    auto mul = 1.0 / (*(std::max_element(begin, value.end())) - min);
    if (!std::isfinite(mul)) return;
    for (size_t i = start; i < value.size(); ++i) setValueAt(i, (value[i] - min) * mul);
  }

  void multiplySkip(size_t start, size_t interval) noexcept
  {
    for (size_t i = start; i < value.size(); i += interval) value[i] *= 0.9;
  }

  void emphasizeLow(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i)
      setValueAt(i, value[i] / pow(i + 1, 0.0625));
  }

  void emphasizeHigh(size_t start)
  {
    for (size_t i = start; i < value.size(); ++i)
      setValueAt(i, value[i] * (0.9 + 0.1 * double(i + 1) / value.size()));
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

  void setValueFromLine(CPoint p0, CPoint p1)
  {
    if (p0.x > p1.x) std::swap(p0, p1);

    size_t left = size_t(p0.x / sliderWidth);
    size_t right = size_t(p1.x / sliderWidth);
    if (left >= value.size() || right >= value.size()) return;

    const float p0y = p0.y;
    const float p1y = p1.y;

    if (left == right) {
      // p0 and p1 are in a same bar.
      setValueAt(left, 1.0f - (p0y + p1y) * 0.5f / getHeight());
      updateValueAt(left);
      invalid();
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

    invalid();
  }

  void setMousePosition(CPoint &where)
  {
    auto view = getViewSize();
    mousePosition = where - CPoint(view.left, view.top);
  }

  inline size_t calcIndex(CPoint position) { return size_t(position.x / sliderWidth); }

  CColor highlightColor{0x00, 0xff, 0x00, 0x33};
  CColor splitterColor{0xff, 0xff, 0xff};
  CColor valueColor{0xdd, 0xdd, 0xdd};
  CColor borderColor{0, 0, 0};
  CColor nameColor{0, 0, 0, 0x88};

  CFontRef indexFontID = nullptr;
  CFontRef nameFontID = nullptr;

  CCoord defaultBorderWidth = 2.0;

  bool isMouseLeftDown = false;
  bool isMouseMiddleDown = false;
  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CPoint anchor{-1.0, -1.0};
  CCoord sliderWidth;

  std::string name;
  std::string indexText;
  std::vector<std::string> barIndices;
};

} // namespace VSTGUI
