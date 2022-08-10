// (c) 2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x7AFF243D, 0xA6854F10, 0x8378D9C0, 0xCBF71B07);
static const FUID ControllerUID(0x6732A813, 0xA8404764, 0xBD33FB98, 0x6254F6EA);

} // namespace Synth
} // namespace Steinberg
