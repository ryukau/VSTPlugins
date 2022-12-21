// (c) 2022 Takamitsu Endo
//
// This file is part of OrdinaryPhaser.
//
// OrdinaryPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OrdinaryPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OrdinaryPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0xB80CFC48, 0x698A4016, 0x852D7818, 0x310D7DA3);
static const FUID ControllerUID(0x50F05D56, 0xF85D4602, 0xBC69FA0E, 0x3EF60997);

} // namespace Synth
} // namespace Steinberg
