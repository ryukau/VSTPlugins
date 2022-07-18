// (c) 2021 Takamitsu Endo
//
// This file is part of ClangSynth.
//
// ClangSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x31D71DE1, 0xC3754F49, 0xBAC9E1EE, 0x064D3A8C);
static const FUID ControllerUID(0x2497FDCD, 0xF2C24036, 0xB2A47133, 0x142A40A6);

} // namespace Synth
} // namespace Steinberg
