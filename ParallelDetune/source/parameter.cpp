// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelDetune.
//
// ParallelDetune is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelDetune is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelDetune.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::gain(-60.0, 20.0, true);
DecibelScale<double> Scales::tremoloMix(-60.0, 0.0, true);
DecibelScale<double> Scales::delayTimeSeconds(-80.0, 20 * std::log10(maxDelayTime), true);
DecibelScale<double> Scales::delayTimeMultiplier(
  -12.041199826559248, 12.041199826559248, false); // Range is [1/4, 4].
LinearScale<double> Scales::shiftOctave(-4.0, 4.0);
LinearScale<double> Scales::shiftSemitone(-12.0, 12.0);
LinearScale<double> Scales::shiftFineTuningCent(-10.0, 10.0);
DecibelScale<double> Scales::cutoffHz(0.0, 100.0, false);
DecibelScale<double> Scales::normalizedCutoff(-60, -6.021, false);
LinearScale<double> Scales::lfoOctaveAmount(-1.0, 1.0);

UIntScale<double> Scales::lfoInterpolation(2);
UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
