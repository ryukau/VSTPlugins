// (c) 2022 Takamitsu Endo
//
// This file is part of BasicLimiterAutoMake.
//
// BasicLimiterAutoMake is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiterAutoMake is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiterAutoMake.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0xAB6D22F6, 0x7C2A4ADA, 0xBCFDAF95, 0x8D3CFA57);
static const FUID ControllerUID(0xD07A48A9, 0x433948C8, 0x8512CC0B, 0x5BE4CE6E);

} // namespace Synth
} // namespace Steinberg
