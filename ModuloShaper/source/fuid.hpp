// (c) 2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x5ACBA767, 0x837B4C02, 0x9DE8E89B, 0xD232C5B9);
static const FUID ControllerUID(0x912ED4B6, 0x5F264FA8, 0xA5BCCAD4, 0x299D4BEF);

} // namespace Synth
} // namespace Steinberg
