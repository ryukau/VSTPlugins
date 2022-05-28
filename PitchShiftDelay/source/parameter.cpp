// (c) 2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
DecibelScale<double> Scales::pitch(-60.0, 60.0, true);
LogScale<double> Scales::delayTime(0.0, 1.0, 0.5, 0.05);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);
DecibelScale<double> Scales::dry(-40.0, 0.0, true);
DecibelScale<double> Scales::wet(-40.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
