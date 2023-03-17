// (c) 2023 Takamitsu Endo
//
// This file is part of RingModSpacer.
//
// RingModSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RingModSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RingModSpacer.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x48B91321, 0xE75349B5, 0x8459491C, 0xBD923DB8);
static const FUID ControllerUID(0x49BAC896, 0x3B5A4D9B, 0xB3AB6CBD, 0x97FD933E);

} // namespace Synth
} // namespace Steinberg
