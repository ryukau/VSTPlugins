// (c) 2022 Takamitsu Endo
//
// This file is part of UltrasonicRingMod.
//
// UltrasonicRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltrasonicRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltrasonicRingMod.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x67F7662C, 0x470F47AD, 0xAF877B5C, 0x06B39ED5);
static const FUID ControllerUID(0xD4F49D46, 0x90C9457F, 0xB5F41485, 0xACD4803F);

} // namespace Synth
} // namespace Steinberg
