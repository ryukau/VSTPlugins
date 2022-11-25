// (c) 2022 Takamitsu Endo
//
// This file is part of MaybeSnare.
//
// MaybeSnare is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MaybeSnare is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MaybeSnare.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x8CEAB2D9, 0x91E34194, 0x80C077FF, 0xD3840FC4);
static const FUID ControllerUID(0x8E2148A6, 0xC2C64057, 0x9982A314, 0x858895AA);

} // namespace Synth
} // namespace Steinberg
