// (c) 2022 Takamitsu Endo
//
// This file is part of ImmediateSpacer.
//
// ImmediateSpacer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ImmediateSpacer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ImmediateSpacer.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::gain(-145.0, 145.0, true);
DecibelScale<double>
  Scales::limiterAttackSeconds(-100.0, 20 * std::log10(maxLimiterAttackSeconds), true);
DecibelScale<double> Scales::limiterReleaseSeconds(-100.0, 60.0, true);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
