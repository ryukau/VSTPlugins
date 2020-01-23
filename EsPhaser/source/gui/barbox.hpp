// (c) 2019 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "vstgui/vstgui.h"

#include "arraycontrol.hpp"
#include "plugeditor.hpp"

#include <algorithm>
#include <random>

namespace VSTGUI {

class BarBox : public ArrayControl {
public:
  Steinberg::Vst::PlugEditor *editor = nullptr;

  BarBox(
    const CRect &size,
    Steinberg::Vst::PlugEditor *editor,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue)
    : ArrayControl(size, id, value, defaultValue)
    , editor(editor)
    , sliderWidth((size.right - size.left) / value.size())
  {
    setWantsFocus(true);
    editor->remember();
  }

  ~BarBox()
  {
    if (editor) editor->forget();
    if (indexFontID) indexFontID->forget();
    if (nameFontID) nameFontID->forget();
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(BarBox, CView);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;
  int32_t onKeyDown(VstKeyCode &key) override;

  void valueChangedAt(size_t index)
  {
    if (!getFrame() || editor == nullptr) return;
    getFrame()->beginEdit(id[index]);
    editor->valueChanged(id[index], value[index]);
    getFrame()->endEdit(id[index]);
  }

  void valueChanged()
  {
    if (id.size() != value.size()) return;
    for (size_t i = 0; i < id.size(); ++i) valueChangedAt(i);
  }

  void setIndexFont(CFontRef fontID) { indexFontID = fontID; }
  void setNameFont(CFontRef fontID) { nameFontID = fontID; }
  void setHighlightColor(CColor color) { highlightColor = color; }
  void setValueColor(CColor color) { valueColor = color; }
  void setBorderColor(CColor color) { borderColor = color; }
  void setDefaultBorderWidth(float width) { defaultBorderWidth = width; }
  void setHighlightBorderWidth(float width) { highlightBorderWidth = width; }
  void setName(UTF8String name) { this->name = name; }

protected:
  void averageLowpass();
  void highpass();
  void randomize(double mix);
  void sparseRandomize();
  void permute();
  void invert(bool preserveMin);
  void normalize(bool preserveMin) noexcept;
  void multiplySkip(size_t offset, size_t interval) noexcept;
  void emphasizeLow();
  void emphasizeHigh();

  void setValueFromPosition(CPoint &position, const CButtonState &buttons);
  void setValueFromLine(CPoint p0, CPoint p1);
  void setMousePosition(CPoint &where);

  CColor highlightColor{0x00, 0xff, 0x00, 0x33};
  CColor splitterColor{0xff, 0xff, 0xff};
  CColor valueColor{0xdd, 0xdd, 0xdd};
  CColor borderColor{0, 0, 0};
  CColor nameColor{0, 0, 0, 0x33};

  CFontRef indexFontID = nullptr;
  CFontRef nameFontID = nullptr;

  CCoord defaultBorderWidth = 2.0;
  CCoord highlightBorderWidth = 4.0;

  bool isMouseLeftDown = false;
  bool isMouseMiddleDown = false;
  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CPoint anchor{-1.0, -1.0};
  CCoord sliderWidth;

  UTF8String name;
};

void BarBox::draw(CDrawContext *pContext)
{
  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  // Value bar and Text.
  pContext->setFillColor(valueColor);
  pContext->setFont(indexFontID);
  pContext->setFontColor(borderColor);
  for (size_t i = 0; i < value.size(); ++i) {
    auto sliderHeight = value[i] * height;
    auto left = round(i * sliderWidth);
    auto right = round(left + sliderWidth);
    auto top = round(height - sliderHeight);
    pContext->drawRect(CRect(left, top, right, height), kDrawFilled);
    pContext->drawString(
      UTF8String(std::to_string(i + 1).c_str()),
      CRect(left, height - 16, right, height - 4)); // TODO
  }

  // Splitter.
  pContext->setLineWidth(defaultBorderWidth);
  pContext->setFrameColor(splitterColor);
  for (size_t i = 0; i < value.size(); ++i) {
    auto x = i * sliderWidth;
    pContext->drawLine(CPoint(x, 0), CPoint(x, height));
  }

  // Border.
  pContext->setFrameColor(borderColor);
  pContext->drawRect(CRect(0, 0, width, height), kDrawStroked);

  // Highlight.
  if (isMouseEntered) {
    size_t index = size_t(value.size() * mousePosition.x / width);
    if (index < value.size()) {
      pContext->setFillColor(highlightColor);
      auto left = index * sliderWidth;
      pContext->drawRect(CRect(left, 0, left + sliderWidth, height), kDrawFilled);
    }
  }

  // Title.
  pContext->setFont(nameFontID);
  pContext->setFontColor(nameColor);
  pContext->drawString(name, CRect(0, 0, width, height));

  setDirty(false);
}

CMouseEventResult BarBox::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseDown(CPoint &where, const CButtonState &buttons)
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

CMouseEventResult BarBox::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if (buttons.isLeftButton()) isMouseLeftDown = false;
  if (buttons.isMiddleButton()) isMouseMiddleDown = false;
  return kMouseEventHandled;
}

CMouseEventResult BarBox::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  setMousePosition(where);
  invalid(); // Required to refresh highlighting position.
  if (buttons.isLeftButton()) {
    setValueFromPosition(mousePosition, buttons);
    return kMouseEventHandled;
  } else if (buttons.isMiddleButton()) {
    setValueFromLine(anchor, mousePosition);
    return kMouseEventHandled;
  }
  return kMouseEventNotHandled;
}

CMouseEventResult BarBox::onMouseCancel()
{
  if (isDirty()) {
    valueChanged();
    invalid();
  }
  return kMouseEventHandled;
}

int32_t BarBox::onKeyDown(VstKeyCode &key)
{
  if (key.character == 'a') { // Sort ascending order.
    std::sort(value.begin(), value.end());
  } else if (key.character == 'd') { // Sort descending order.
    std::sort(value.begin(), value.end(), std::greater<>());
  } else if (key.character == 'f') {
    averageLowpass();
  } else if (key.character == 'F') {
    highpass();
  } else if (key.character == 'h') {
    emphasizeHigh();
  } else if (key.character == 'i') {
    invert(true);
  } else if (key.character == 'I') {
    invert(false);
  } else if (key.character == 'l') {
    emphasizeLow();
  } else if (key.character == 'n') {
    normalize(true);
  } else if (key.character == 'N') {
    normalize(false);
  } else if (key.character == 'p') { // Permute.
    permute();
  } else if (key.character == 'r') {
    randomize(1.0);
  } else if (key.character == 'R') {
    sparseRandomize();
  } else if (key.character == 's') { // Subtle randomize.
    randomize(0.02);
  } else if (key.character == ',') { // Rotate back.
    std::rotate(value.begin(), value.begin() + 1, value.end());
  } else if (key.character == '.') { // Rotate forward.
    std::rotate(value.rbegin(), value.rbegin() + 1, value.rend());
  } else if (key.character == '1') { // Decrease odd.
    multiplySkip(0, 2);
  } else if (key.character == '2') { // Decrease even.
    multiplySkip(1, 2);
  } else if (key.character == '3') { // Decrease 3n.
    multiplySkip(2, 3);
  } else if (key.character == '4') { // Decrease 4n.
    multiplySkip(3, 4);
  } else if (key.character == '5') { // Decrease 5n.
    multiplySkip(4, 5);
  } else if (key.character == '6') { // Decrease 6n.
    multiplySkip(5, 6);
  } else if (key.character == '7') { // Decrease 7n.
    multiplySkip(6, 7);
  } else if (key.character == '8') { // Decrease 8n.
    multiplySkip(7, 8);
  } else if (key.character == '9') { // Decrease 9n.
    multiplySkip(8, 9);
  } else {
    return -1;
  }
  invalid();
  valueChanged();
  return 1;
}

void BarBox::averageLowpass()
{
  const int32_t range = 1;

  std::vector<double> result(value);
  for (size_t i = 0; i < value.size(); ++i) {
    result[i] = 0.0;
    for (int32_t j = -range; j <= range; ++j) {
      size_t index = i + j; // Note that index is unsigned.
      if (index >= value.size()) continue;
      result[i] += value[index];
    }
    setValueAt(i, result[i] / double(2 * range + 1));
  }
}

void BarBox::highpass()
{
  // value[i] = sum((-0.5, 1.0, -0.5) * value[(i - 1, i, i + 1)])
  // Value of index outside of array is assumed to be same as closest element.
  std::vector<double> result(value);
  size_t last = value.size() - 1;
  for (size_t i = 0; i < value.size(); ++i) {
    result[i] = 0.0;
    result[i] -= (i >= 1) ? value[i - 1] : value[i];
    result[i] -= (i < last) ? value[i + 1] : value[i];
    result[i] = value[i] + 0.5f * result[i];
    setValueAt(i, result[i]);
  }
}

void BarBox::randomize(double mix)
{
  std::random_device dev;
  std::mt19937_64 rng(dev());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  for (auto &val : value) val += mix * (dist(rng) - val);
}

void BarBox::sparseRandomize()
{
  std::random_device device;
  std::mt19937_64 rng(device());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  for (auto &val : value) {
    if (dist(rng) < 0.1f) val = dist(rng);
  }
}

void BarBox::permute()
{
  std::random_device device;
  std::mt19937 rng(device());
  std::shuffle(value.begin(), value.end(), rng);
}

void BarBox::invert(bool preserveMin)
{
  auto min = preserveMin ? *(std::min_element(value.begin(), value.end())) : 0.0;
  for (size_t i = 0; i < value.size(); ++i) setValueAt(i, 1.0f - value[i] + min);
}

void BarBox::normalize(bool preserveMin) noexcept
{
  auto min = preserveMin ? 0.0 : *(std::min_element(value.begin(), value.end()));
  auto mul = 1.0 / (*(std::max_element(value.begin(), value.end())) - min);
  if (!std::isfinite(mul)) return;
  for (size_t i = 0; i < value.size(); ++i) setValueAt(i, (value[i] - min) * mul);
}

void BarBox::multiplySkip(size_t offset, size_t interval) noexcept
{
  for (size_t i = offset; i < value.size(); i += interval) value[i] *= 0.9;
}

void BarBox::emphasizeLow()
{
  for (size_t i = 0; i < value.size(); ++i) setValueAt(i, value[i] / pow(i + 1, 0.0625));
}

void BarBox::emphasizeHigh()
{
  for (size_t i = 0; i < value.size(); ++i)
    setValueAt(i, value[i] * (0.9 + 0.1 * double(i + 1) / value.size()));
}

void BarBox::setValueFromPosition(CPoint &position, const CButtonState &buttons)
{
  size_t index = size_t(position.x / sliderWidth);
  if (index >= value.size()) return;
  if (buttons & kControl)
    setValueAt(index, defaultValue[index]);
  else
    setValueAt(index, 1.0 - double(position.y) / getHeight());
  valueChangedAt(index);
  invalid();
}

void BarBox::setValueFromLine(CPoint p0, CPoint p1)
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
    valueChangedAt(left);
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

  valueChanged();
  invalid();
}

void BarBox::setMousePosition(CPoint &where)
{
  auto view = getViewSize();
  mousePosition = where - CPoint(view.left, view.top);
}

} // namespace VSTGUI
