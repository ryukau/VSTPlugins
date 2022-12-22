// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x2F26E910, 0xCB5446A4, 0x8EEC8E38, 0xAB36F890);
static const FUID ControllerUID(0x22B01464, 0xCC0D41BB, 0x94FF6B66, 0xAD46A77D);

} // namespace Synth
} // namespace Steinberg
