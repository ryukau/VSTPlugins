// (c) 2023 Takamitsu Endo
//
// This file is part of DoubleLoopCymbal.
//
// DoubleLoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DoubleLoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DoubleLoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"

#include <cmath>
#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

template<typename T> inline T ampToDB(T amp) { return T(20) * std::log10(amp); }

constexpr auto eps = std::numeric_limits<float>::epsilon();

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);
UIntScale<double> Scales::seed(1 << 23);

DecibelScale<double> Scales::gain(-100.0, 60.0, true);

LinearScale<double> Scales::semitone(-60.0, 60.0);
DecibelScale<double> Scales::noteSlideTimeSecond(-100.0, 40.0, true);

DecibelScale<double> Scales::noiseDecaySeconds(-100, 40, false);
DecibelScale<double> Scales::halfClosedGain(-100.0, 0.0, true);
DecibelScale<double> Scales::halfClosedDensityHz(0.0, 80.0, true);
DecibelScale<double> Scales::delayTimeSecond(-100, -30, false);
DecibelScale<double> Scales::delayTimeModAmount(-40, 60, true);

UIntScale<double> Scales::allpassDelayCount(nAllpass - 1);

DecibelScale<double> Scales::cutoffFrequencyHz(0, 100, false);
DecibelScale<double> Scales::shelvingGain(-60, 0, true);

LinearScale<double> Scales::velocityRangeDecibel(-100.0, 0.0);

} // namespace Synth
} // namespace Steinberg
