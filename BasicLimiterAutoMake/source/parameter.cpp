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

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
DecibelScale<double> Scales::limiterThreshold(-30.0, 30.0, false);
DecibelScale<double> Scales::limiterGate(-100.0, 0.0, true);
LogScale<double> Scales::limiterAttack(0.0001, maxAttackSeconds, 0.5, 0.002);
LogScale<double> Scales::limiterRelease(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::limiterSustain(0.0, maxAttackSeconds, 0.1, 0.002);
LinearScale<double> Scales::overshoot(1.0, 32.0);
UIntScale<double> Scales::channelType(1);

} // namespace Synth
} // namespace Steinberg
