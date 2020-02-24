// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

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
    double tabHeight,
    const CRect &size);
  ~TabView();

  void addWidget(size_t tabIndex, CView *newWidget);
  void addWidget(size_t tabIndex, std::tuple<CView *, CView *> newWidgets);
  void refreshTab();
  void setHighlightColor(CColor color) { colorFocus = color; }

  void draw(CDrawContext *pContext) override;
  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CLASS_METHODS(TabView, CView);

protected:
  bool isInTabArea(const CPoint &pos);

  CColor colorFocus{0x33, 0xaa, 0xff};
  CColor colorFore{0, 0, 0};
  CColor colorBack{0xff, 0xff, 0xff};

  float tabHeight = 30.0f;
  float tabFontSize = 14.0f;
  CFontRef tabFontID = nullptr;

  bool isMouseEntered = false;
};

TabView::TabView(
  std::vector<std::string> tabNames,
  CFontRef tabFontID,
  double tabHeight,
  const CRect &size)
  : CControl(size, nullptr, -1)
{
  tabFontID->remember();
  this->tabFontID = tabFontID;

  tabs.reserve(tabNames.size());
  const auto tabWidth = float(getWidth()) / tabNames.size();
  for (size_t idx = 0; idx < tabNames.size(); ++idx) {
    tabs.push_back(TabButton(tabNames[idx], idx * tabWidth, 0, tabWidth, tabHeight));
  }

  widgets.resize(tabs.size());
}

TabView::~TabView()
{
  if (tabFontID) tabFontID->forget();
  for (auto &tab : widgets) {
    for (auto &wdgt : tab) {
      if (wdgt != nullptr) wdgt->forget();
    }
  }
}

void TabView::addWidget(size_t tabIndex, CView *newWidget)
{
  if (newWidget == nullptr) return;
  if (tabIndex >= widgets.size()) return;
  newWidget->remember();
  widgets[tabIndex].push_back(newWidget);
}

// Parhaps using tuple is not good idea for now.
void TabView::addWidget(size_t tabIndex, std::tuple<CView *, CView *> newWidgets)
{
  addWidget(tabIndex, std::get<0>(newWidgets));
  addWidget(tabIndex, std::get<1>(newWidgets));
}

void TabView::refreshTab()
{
  for (size_t idx = 0; idx < tabs.size(); ++idx) {
    bool isVisible = idx == activeTabIndex;
    for (auto &widget : widgets[idx]) widget->setVisible(isVisible);
  }
}

void TabView::draw(CDrawContext *pContext)
{
  const auto width = getWidth();
  const auto height = getHeight();

  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  pContext->setFont(tabFontID);
  pContext->setFontColor(colorFore);
  pContext->setLineWidth(1.0f);
  for (size_t idx = 0; idx < tabs.size(); ++idx) {
    if (idx == activeTabIndex) continue;
    const auto &tab = tabs[idx];

    pContext->setFillColor(tab.isMouseEntered ? colorFocus : colorBack);
    pContext->setFrameColor(colorFore);
    pContext->drawRect(
      CRect(tab.left, tab.top, tab.right, tab.height), kDrawFilledAndStroked);

    pContext->setFillColor(colorFore);
    pContext->drawString(
      UTF8String(tab.name.c_str()), CRect(tab.left, tab.top, tab.right, tab.bottom),
      kCenterText);
  }

  // Active tab.
  pContext->setFillColor(colorBack);
  pContext->setFrameColor(colorFore);
  pContext->setLineWidth(2.0f);

  const auto &activeTab = tabs[activeTabIndex];
  CDrawContext::PointList activeTabPath = {
    CPoint(0, activeTab.height),
    CPoint(activeTab.left, activeTab.height),
    CPoint(activeTab.left, 0),
    CPoint(activeTab.right, 0),
    CPoint(activeTab.right, activeTab.height),
    CPoint(width, activeTab.height),
    CPoint(width, height),
    CPoint(0, height),
    CPoint(0, activeTab.height),
  };
  pContext->drawPolygon(activeTabPath, kDrawFilledAndStroked);

  pContext->setFillColor(colorFore);
  pContext->drawString(
    UTF8String(activeTab.name.c_str()),
    CRect(activeTab.left, activeTab.top, activeTab.right, activeTab.bottom), kCenterText);
}

CMouseEventResult TabView::onMouseEntered(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = true;
  return kMouseEventHandled;
};

CMouseEventResult TabView::onMouseExited(CPoint &where, const CButtonState &buttons)
{
  isMouseEntered = false;
  for (auto &tab : tabs) tab.isMouseEntered = false;
  invalid();
  return kMouseEventHandled;
};

CMouseEventResult TabView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if (!isInTabArea(where) || !buttons.isLeftButton()) return kMouseEventNotHandled;
  auto view = getViewSize();
  auto mouseX = where.x - view.left;
  auto mouseY = where.y - view.top;
  for (size_t idx = 0; idx < tabs.size(); ++idx) {
    if (tabs[idx].hitTest(mouseX, mouseY)) {
      activeTabIndex = idx;
      break;
    }
  }
  refreshTab();
  invalid();
  return kMouseEventHandled;
};

CMouseEventResult TabView::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  if (!isMouseEntered) return kMouseEventNotHandled;
  auto view = getViewSize();
  auto mouseX = where.x - view.left;
  auto mouseY = where.y - view.top;
  for (auto &tab : tabs) tab.isMouseEntered = tab.hitTest(mouseX, mouseY);
  invalid();
  return kMouseEventHandled;
};

bool TabView::isInTabArea(const CPoint &pos)
{
  auto view = getViewSize();
  return view.left <= pos.x && pos.x <= view.right && view.top <= pos.y
    && pos.y <= view.bottom;
}

} // namespace VSTGUI
