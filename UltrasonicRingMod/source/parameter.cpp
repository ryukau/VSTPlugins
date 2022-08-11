// (c) 2022 Takamitsu Endo
//
// This file is part of UltrasonicRingMod.
//
// UltrasonicRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltrasonicRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltrasonicRingMod.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

DecibelScale<double> Scales::gain(-60.0, 60.0, true);
DecibelScale<double> Scales::frequencyHz(0.0, 120.0, false);
DecibelScale<double> Scales::feedbackGain(-80.0, 80.0, true);
DecibelScale<double> Scales::modFrequencyScaling(-100.0, 0.0, true);
NegativeDecibelScale<double> Scales::modWrapMix(-60.0, 0.0, 1.0, true);

LinearScale<double> Scales::noteScaling(0.0, 4.0);
LinearScale<double> Scales::noteOffset(-192.0, 192.0);
DecibelScale<double> Scales::noteSlideTimeSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
