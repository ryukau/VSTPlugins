// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "pluginterfaces/vst/vsttypes.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <cmath>

#include "parameter.hpp"

namespace Steinberg {
namespace SevenDelay {

using namespace SomeDSP;

BoolScale<Vst::ParamValue> Scales::boolScale{};
LinearScale<Vst::ParamValue> Scales::defaultScale(0.0, 1.0);

LogScale<Vst::ParamValue> Scales::time(0.0001, maxDelayTime, 0.5, 1.0);
SPolyScale<Vst::ParamValue> Scales::offset(-1.0, 1.0, 0.8);
LogScale<Vst::ParamValue> Scales::lfoTimeAmount(0, 1.0, 0.5, 0.07);
LogScale<Vst::ParamValue> Scales::lfoToneAmount(0, 0.5, 0.5, 0.1);
LogScale<Vst::ParamValue> Scales::lfoFrequency(0.01, 100.0, 0.5, 1.0);
LogScale<Vst::ParamValue> Scales::lfoShape(0.01, 10.0, 0.5, 1.0);
LinearScale<Vst::ParamValue> Scales::lfoInitialPhase(0.0, 2.0 * M_PI);
LogScale<Vst::ParamValue> Scales::smoothness(0.0, 1.0, 0.3, 0.04);
LogScale<Vst::ParamValue> Scales::toneCutoff(90.0, maxToneFrequency, 0.5, 1000.0);
LogScale<Vst::ParamValue> Scales::toneQ(1e-5, 1.0, 0.5, 0.1);
LogScale<Vst::ParamValue> Scales::toneMix(0.0, 1.0, 0.9, 0.05);
LogScale<Vst::ParamValue> Scales::dckill(minDCKillFrequency, 120.0, 0.5, 20.0);
LogScale<Vst::ParamValue> Scales::dckillMix(0.0, 1.0, 0.9, 0.05);

} // namespace SevenDelay
} // namespace Steinberg
