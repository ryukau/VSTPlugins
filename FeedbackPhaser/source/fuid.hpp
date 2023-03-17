// (c) 2023 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {
namespace Synth {

// https://www.guidgenerator.com/
static const FUID ProcessorUID(0x4D7FB2AB, 0xCB564068, 0xACF603D4, 0x0A2A08B6);
static const FUID ControllerUID(0x1310437F, 0xD4A84131, 0x84CA152B, 0x0E45BFE9);

} // namespace Synth
} // namespace Steinberg
