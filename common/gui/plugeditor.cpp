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

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "plugeditor.hpp"
#include "x11runloop.hpp"

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller) { setRect(viewRect); }

PlugEditor::~PlugEditor()
{
  for (auto &ctrl : controlMap)
    if (ctrl.second) ctrl.second->forget();

  for (auto &ctrl : arrayControlInstances)
    if (ctrl.second) ctrl.second->forget();
}

bool PlugEditor::open(void *parent, const PlatformType &platformType)
{
  if (frame) return false;

  setIdleRate(1000 / 60);

  frame = new CFrame(
    CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
  if (frame == nullptr) return false;
  frame->setBackgroundColor(palette.background());
  frame->registerMouseObserver(this);

  IPlatformFrameConfig *config = nullptr;
#if LINUX
  X11::FrameConfig x11config;
  x11config.runLoop = VSTGUI::owned(new RunLoop(plugFrame));
  config = &x11config;
#endif
  frame->open(parent, platformType, config);

  return prepareUI();
}

void PlugEditor::close()
{
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

void PlugEditor::valueChanged(ParamID id, ParamValue normalized)
{
  controller->setParamNormalized(id, normalized);
  controller->performEdit(id, normalized);
}

void PlugEditor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto vCtrl = controlMap.find(id);
  if (vCtrl != controlMap.end()) {
    vCtrl->second->setValueNormalized(normalized);
    vCtrl->second->invalid();
    return;
  }

  auto aCtrl = arrayControlMap.find(id);
  if (aCtrl != arrayControlMap.end()) {
    aCtrl->second->setValueAt(id - aCtrl->second->id.front(), normalized);
    aCtrl->second->invalid();
    return;
  }
}

CMouseEventResult
PlugEditor::onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isRightButton()) return kMouseEventNotHandled;

  auto componentHandler = controller->getComponentHandler();
  if (componentHandler == nullptr) return kMouseEventNotHandled;

  FUnknownPtr<IComponentHandler3> handler(componentHandler);
  if (handler == nullptr) return kMouseEventNotHandled;

  auto control = dynamic_cast<CControl *>(frame->getViewAt(where));
  if (control == nullptr) return kMouseEventNotHandled;

  // Context menu will not popup when the control has negative tag.
  ParamID id = control->getTag();
  if (id < 1 || id >= LONG_MAX) return kMouseEventNotHandled;

  IContextMenu *menu = handler->createContextMenu(this, &id);
  if (menu == nullptr) return kMouseEventNotHandled;

  menu->popup(where.x, where.y);
  menu->release();
  return kMouseEventHandled;
}

CMouseEventResult
PlugEditor::onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  return kMouseEventNotHandled;
}

void PlugEditor::addToControlMap(Vst::ParamID id, CControl *control)
{
  auto iter = controlMap.find(id);
  if (iter != controlMap.end()) iter->second->forget();
  control->remember();
  controlMap.insert({id, control});
}

} // namespace Vst
} // namespace Steinberg
