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

#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

namespace VSTGUI {

struct ArrayControl : public CControl {
public:
  ArrayControl(
    Steinberg::Vst::EditController *controller,
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue)
    : CControl(size, nullptr, -1)
    , controller(controller)
    , id(id)
    , value(value)
    , defaultValue(defaultValue)
  {
    if (controller != nullptr) controller->addRef();
  }

  ~ArrayControl()
  {
    if (controller != nullptr) controller->release();
  }

  void setValueAt(size_t index, double normalized)
  {
    if (index < value.size())
      value[index] = normalized < 0.0 ? 0.0 : normalized > 1.0 ? 1.0 : normalized;
  }

  void updateValue()
  {
    if (id.size() != value.size()) return;
    for (size_t i = 0; i < id.size(); ++i) updateValueAt(i);
  }

  void updateValueAt(size_t index)
  {
    controller->setParamNormalized(id[index], value[index]);
    controller->performEdit(id[index], value[index]);
  }

  Steinberg::Vst::EditController *controller = nullptr;
  std::vector<Steinberg::Vst::ParamID> id;
  std::vector<double> value;
  std::vector<double> defaultValue;
};

} // namespace VSTGUI
