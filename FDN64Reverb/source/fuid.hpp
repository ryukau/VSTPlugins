// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0xC880D61A, 0x8F9A443D, 0x9612BA79, 0xE69A87C3);
static const FUID ControllerUID(0x0D768248, 0xCE7E403A, 0xA7B927B9, 0x0F1F1B15);

} // namespace Synth
} // namespace Steinberg
