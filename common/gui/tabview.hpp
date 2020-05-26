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

#include <string>
#include <tuple>
#include <vector>

#include <iostream>

namespace VSTGUI {

struct TabButton {
  std::string name;
  float left = 0;
  float top = 0;
  float right = 0;
  float bottom = 0;

  float width = 0;
  float height = 0;

  bool isMouseEntered = false;

  TabButton(std::string name, float left, float top, float width, float height)
    : name(name), left(left), top(top), width(width), height(height)
  {
    right = left + width;
    bottom = top + height;
  }

  bool hitTest(float x, float y)
  {
    return left <= x && x <= right && top <= y && y <= bottom;
  }
};

class TabView : public CControl {
public:
  std::vector<std::vector<CView *>> widgets;
  std::vector<TabButton> tabs;
  size_t activeTabIndex = 0;

  TabView(
    std::vector<std::string> tabNames,
    CFontRef tabFontID,
    Uhhyou::Palette &palette,
    float tabHeight,
    const CRect &size);
  ~TabView();

  void addWidget(size_t tabIndex, CView *newWidget);
  void addWidget(size_t tabIndex, std::tuple<CView *, CView *> newWidgets);
  void refreshTab();

  void draw(CDrawContext *pContext) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override;

  CLASS_METHODS(TabView, CView);

protected:
  bool isInTabArea(const CPoint &pos);

  float tabHeight = 30.0f;
  float tabFontSize = 14.0f;
  CFontRef tabFontID = nullptr;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
  CPoint mousePosition;
};

} // namespace VSTGUI
