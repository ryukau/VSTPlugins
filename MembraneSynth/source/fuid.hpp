// (c) 2022 Takamitsu Endo
//
// This file is part of MembraneSynth.
//
// MembraneSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MembraneSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MembraneSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x6A672DC1, 0xE57A4D6F, 0xA81A49F0, 0x9916B435);
static const FUID ControllerUID(0x845CABF6, 0xE97F465F, 0xB936C09B, 0xC09B82F0);

} // namespace Synth
} // namespace Steinberg
