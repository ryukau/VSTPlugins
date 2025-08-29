// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
    const SharedPointer<CFontDesc> &tabFontID,
    Uhhyou::Palette &palette,
    float tabHeight,
    const CRect &size)
    : CControl(size, nullptr, -1)
    , tabHeight(tabHeight)
    , tabFontID(tabFontID)
    , pal(palette)
  {
    tabs.reserve(tabNames.size());
    const auto tabWidth = float(getWidth()) / tabNames.size();
    for (size_t idx = 0; idx < tabNames.size(); ++idx) {
      tabs.push_back(TabButton(tabNames[idx], idx * tabWidth, 0, tabWidth, tabHeight));
    }

    widgets.resize(tabs.size());
  }

  virtual ~TabView()
  {
    for (auto &tab : widgets) {
      for (auto &wdgt : tab) {
        if (wdgt != nullptr) wdgt->forget();
      }
    }
  }

  void addWidget(size_t tabIndex, CView *newWidget)
  {
    if (newWidget == nullptr) return;
    if (tabIndex >= widgets.size()) return;
    newWidget->remember();
    widgets[tabIndex].push_back(newWidget);
  }

  void addWidget(size_t tabIndex, std::tuple<CView *, CView *> newWidgets)
  {
    addWidget(tabIndex, std::get<0>(newWidgets));
    addWidget(tabIndex, std::get<1>(newWidgets));
  }

  void refreshTab()
  {
    for (size_t idx = 0; idx < tabs.size(); ++idx) {
      bool isVisible = idx == activeTabIndex;
      for (auto &widget : widgets[idx]) widget->setVisible(isVisible);
    }
  }

  virtual void setVisible(bool state) override
  {
    CView::setVisible(state);
    for (auto &widget : widgets[activeTabIndex]) widget->setVisible(state);
  }

  void draw(CDrawContext *pContext) override
  {
    const auto width = getWidth();
    const auto height = getHeight();
    const auto sc = pal.guiScale();
    auto borderW = int(2 * sc);
    borderW += borderW % 2;
    const auto halfBorderW = int(borderW / 2);

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Inactive tab.
    pContext->setFont(tabFontID);
    pContext->setFontColor(pal.foregroundInactive());
    pContext->setFrameColor(pal.border());
    pContext->setLineWidth(borderW);
    for (size_t idx = 0; idx < tabs.size(); ++idx) {
      if (idx == activeTabIndex) continue;
      const auto &tab = tabs[idx];

      pContext->setFillColor(
        tab.isMouseEntered ? pal.overlayHighlight() : pal.boxBackground());

      const auto left = tab.left + (idx == 0 ? halfBorderW : 0);
      const auto right = tab.right + (idx != tabs.size() - 1 ? halfBorderW : 0);
      pContext->drawRect(
        CRect(left, tab.top + halfBorderW, right, tab.height + halfBorderW),
        kDrawFilledAndStroked);

      pContext->drawString(
        tab.name.c_str(), CRect(tab.left, tab.top, tab.right, tab.bottom), kCenterText);
    }

    // Active tab.
    pContext->setFontColor(pal.foreground());
    pContext->setFillColor(pal.background());
    pContext->setFrameColor(pal.border());
    pContext->setLineWidth(borderW);

    const auto &activeTab = tabs[activeTabIndex];
    const auto &hbW = halfBorderW;
    const auto tabB = activeTab.height - halfBorderW;
    const auto tabL = activeTab.left + (activeTabIndex == 0 ? hbW : 0);
    const auto tabR = activeTab.right - (activeTabIndex == tabs.size() - 1 ? hbW : 0);
    CDrawContext::PointList activeTabPath = {
      CPoint(hbW, tabB),
      CPoint(tabL, tabB),
      CPoint(tabL, hbW),
      CPoint(tabR, hbW),
      CPoint(tabR, tabB),
      CPoint(width - hbW, tabB),
      CPoint(width - hbW, height - hbW),
      CPoint(hbW, height - hbW),
      CPoint(hbW, tabB),
    };
    pContext->drawPolygon(activeTabPath, kDrawFilledAndStroked);

    pContext->setFillColor(pal.foreground());
    pContext->drawString(
      activeTab.name.c_str(),
      CRect(activeTab.left, activeTab.top, activeTab.right, activeTab.bottom),
      kCenterText);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    isMouseEntered = false;
    for (auto &tab : tabs) tab.isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    if (!isInTabArea(event.mousePosition) || !event.buttonState.isLeft()) return;
    auto view = getViewSize();
    auto mouseX = event.mousePosition.x - view.left;
    auto mouseY = event.mousePosition.y - view.top;
    for (size_t idx = 0; idx < tabs.size(); ++idx) {
      if (tabs[idx].hitTest(float(mouseX), float(mouseY))) {
        activeTabIndex = idx;
        break;
      }
    }
    refreshTab();
    invalid();
    event.consumed = true;
  }

  void onMouseMoveEvent(MouseMoveEvent &event) override
  {
    if (!isMouseEntered) return;
    auto view = getViewSize();
    auto mouseX = event.mousePosition.x - view.left;
    auto mouseY = event.mousePosition.y - view.top;
    for (auto &tab : tabs) tab.isMouseEntered = tab.hitTest(float(mouseX), float(mouseY));
    invalid();
    event.consumed = true;
  }

  void onMouseWheelEvent(MouseWheelEvent &event) override
  {
    if (event.deltaY == 0.0f || !isInTabArea(event.mousePosition)) return;
    if (event.deltaY > 0.0f) {
      activeTabIndex -= 1;
      if (activeTabIndex >= tabs.size()) activeTabIndex += tabs.size();
    } else {
      activeTabIndex += 1;
      if (activeTabIndex >= tabs.size()) activeTabIndex -= tabs.size();
    }
    refreshTab();
    invalid();
    event.consumed = true;
  }

  CLASS_METHODS(TabView, CView);

protected:
  bool isInTabArea(const CPoint &pos)
  {
    auto view = getViewSize();
    return view.left <= pos.x && pos.x <= view.right && view.top <= pos.y
      && pos.y <= view.top + tabHeight;
  }

  float tabHeight = 30.0f;
  SharedPointer<CFontDesc> tabFontID;
  Uhhyou::Palette &pal;

  bool isMouseEntered = false;
  CPoint mousePosition;
};

} // namespace VSTGUI
