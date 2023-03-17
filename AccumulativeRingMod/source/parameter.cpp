// (c) 2023 Takamitsu Endo
//
// This file is part of AccumulativeRingMod.
//
// AccumulativeRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AccumulativeRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with AccumulativeRingMod.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::outputGain(-60.0, 20.0, true);
DecibelScale<double> Scales::stereoPhaseLinkHz(-40.0, 100.0, true);
DecibelScale<double> Scales::modulation(-80.0, 40.0, true);
DecibelScale<double> Scales::cutoffHz(-20.0, 100.0, false);
DecibelScale<double> Scales::gateThreshold(-140.0, 0.0, true);
DecibelScale<double> Scales::envelopeSecond(-100.0, 40.0, true);

UIntScale<double> Scales::oversampling(2);
DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
