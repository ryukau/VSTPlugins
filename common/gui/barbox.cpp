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

#include "barbox.hpp"

namespace VSTGUI {

void BarBox::draw(CDrawContext *pContext)
{
  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  // Value bar and Text.
  pContext->setFillColor(valueColor);
  pContext->setFrameColor(valueColor);
  pContext->setLineWidth(1.0f);
  pContext->setFont(indexFontID);
  pContext->setFontColor(borderColor);
  for (size_t i = 0; i < value.size(); ++i) {
    auto sliderHeight = value[i] * height;
    auto left = i * sliderWidth;
    auto right = left + (sliderWidth >= 4.0 ? sliderWidth - defaultBorderWidth : 1.0);
    auto top = height - sliderHeight;
    pContext->drawRect(CRect(left, top, right, height), kDrawFilled);
    if (drawCenterLine) {
      pContext->drawString(
        UTF8String(std::to_string(i + 1).c_str()),
        CRect(left, height - 16, right, height - 4), kCenterText);
    }
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
      os << "#" << std::to_string(index + 1) << ": " << std::to_string(value[index]);
      std::string indexText(os.str());
      pContext->drawString(UTF8String(indexText.c_str()), CRect(0, 0, width, height));
    }
  } else {
    // Title.
    pContext->setFont(nameFontID);
    pContext->setFontColor(nameColor);
    pContext->drawString(UTF8String(name.c_str()), CRect(0, 0, width, height));
  }

  // Center line.
  if (drawCenterLine) {
    pContext->setFrameColor(nameColor);
    pContext->setLineWidth(1.0f);
    pContext->drawLine(CPoint(0, height / 2), CPoint(width, height / 2));
  }

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
    auto componentHandler = controller->getComponentHandler();
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
    updateValue();
    invalid();
  }
  return kMouseEventHandled;
}

bool BarBox::onWheel(
  const CPoint &where,
  const CMouseWheelAxis &axis,
  const float &distance,
  const CButtonState &buttons)
{
  if (isEditing() || axis != kMouseWheelAxisY || distance == 0.0f) return false;

  size_t index = calcIndex(mousePosition);
  if (index >= value.size()) return false;

  setValueAt(index, value[index] + distance * 0.01f);
  updateValueAt(index);
  invalid();
  return true;
}

int32_t BarBox::onKeyDown(VstKeyCode &key)
{
  size_t index = calcIndex(mousePosition);
  if (key.character == 'd') { // reset to Default.
    value = defaultValue;
  } else if (key.character == 'D') { // Alternative default. (toggle min/max)
    std::fill(
      value.begin() + index, value.end(),
      value[index] == 0 ? 0.5 : value[index] == 0.5 ? 1.0 : 0.0);
  } else if (key.character == 'e') {
    emphasizeLow(index);
  } else if (key.character == 'E') {
    emphasizeHigh(index);
  } else if (key.character == 'f') {
    averageLowpass(index);
  } else if (key.character == 'F') {
    highpass(index);
  } else if (key.character == 'i') {
    invert(index, true);
  } else if (key.character == 'I') {
    invert(index, false);
  } else if (key.character == 'n') {
    normalize(index, true);
  } else if (key.character == 'N') {
    normalize(index, false);
  } else if (key.character == 'p') { // Permute.
    permute(index);
  } else if (key.character == 'r') {
    randomize(index, 1.0);
  } else if (key.character == 'R') {
    sparseRandomize(index);
  } else if (key.character == 's') { // Sort descending order.
    std::sort(value.begin() + index, value.end(), std::greater<>());
  } else if (key.character == 'S') { // Sort ascending order.
    std::sort(value.begin() + index, value.end());
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

void BarBox::averageLowpass(size_t start)
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

void BarBox::highpass(size_t start)
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

void BarBox::randomize(size_t start, double mix)
{
  std::random_device dev;
  std::mt19937_64 rng(dev());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  for (size_t i = start; i < value.size(); ++i) value[i] += mix * (dist(rng) - value[i]);
}

void BarBox::sparseRandomize(size_t start)
{
  std::random_device device;
  std::mt19937_64 rng(device());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  for (size_t i = start; i < value.size(); ++i)
    if (dist(rng) < 0.1) value[i] = dist(rng);
}

void BarBox::permute(size_t start)
{
  std::random_device device;
  std::mt19937 rng(device());
  std::shuffle(value.begin() + start, value.end(), rng);
}

void BarBox::invert(size_t start, bool preserveMin)
{
  auto min = preserveMin ? *(std::min_element(value.begin(), value.end())) : 0.0;
  for (size_t i = start; i < value.size(); ++i) setValueAt(i, 1.0f - value[i] + min);
}

void BarBox::normalize(size_t start, bool preserveMin) noexcept
{
  auto min = preserveMin ? 0.0 : *(std::min_element(value.begin(), value.end()));
  auto mul = 1.0 / (*(std::max_element(value.begin(), value.end())) - min);
  if (!std::isfinite(mul)) return;
  for (size_t i = start; i < value.size(); ++i) setValueAt(i, (value[i] - min) * mul);
}

void BarBox::multiplySkip(size_t start, size_t interval) noexcept
{
  for (size_t i = start; i < value.size(); i += interval) value[i] *= 0.9;
}

void BarBox::emphasizeLow(size_t start)
{
  for (size_t i = start; i < value.size(); ++i)
    setValueAt(i, value[i] / pow(i + 1, 0.0625));
}

void BarBox::emphasizeHigh(size_t start)
{
  for (size_t i = start; i < value.size(); ++i)
    setValueAt(i, value[i] * (0.9 + 0.1 * double(i + 1) / value.size()));
}

void BarBox::setValueFromPosition(CPoint &position, const CButtonState &buttons)
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

  updateValue();
  invalid();
}

void BarBox::setMousePosition(CPoint &where)
{
  auto view = getViewSize();
  mousePosition = where - CPoint(view.left, view.top);
}

inline size_t BarBox::calcIndex(CPoint position)
{
  return size_t(position.x / sliderWidth);
}

} // namespace VSTGUI
