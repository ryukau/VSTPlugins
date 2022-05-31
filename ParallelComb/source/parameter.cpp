// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::delayTime(0.0, maxDelayTime, 0.5, 0.1 * maxDelayTime);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);

// log2(0.1 Hz / 440) * 12 + 69 = -76.23945370094427.
// log2(24000 Hz / 440) * 12 + 69 = 138.232644862303.
SemitoneScale<double>
  Scales::feedbackHighpassCutoffHz(-76.23945370094427, 138.232644862303, true);

LogScale<double> Scales::feedbackLimiterRelease(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::delayTimeInterpRate(0.0, 4.0, 0.5, 0.25);
DecibelScale<double> Scales::delayTimeInterpLowpassSeconds(-80.0, 40.0, true);

LinearScale<double> Scales::stereoLean(-1.0, 1.0);

DecibelScale<double> Scales::dry(-60.0, 24.0, true);
DecibelScale<double> Scales::wet(-60.0, 60.0, true);

} // namespace Synth
} // namespace Steinberg
