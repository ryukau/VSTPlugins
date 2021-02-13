// (c) 2021 Takamitsu Endo
//
// This file is part of MatrixShifter.
//
// MatrixShifter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixShifter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixShifter.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LinearScale<double> Scales::shiftSemi(0.0, 10.0);
LogScale<double>
  Scales::shiftDelay(0, maxShiftDelaySeconds, 0.5, 0.2 * maxShiftDelaySeconds);
DecibelScale<double> Scales::shiftGain(-30, 0, true);
LinearScale<double> Scales::shiftPhase(-0.5, 0.5);
DecibelScale<double> Scales::shiftFeedback(-18, 0, true);
LogScale<double> Scales::shiftSemiMultiplier(0.0, 1.0, 0.5, 0.2);

SemitoneScale<double> Scales::lfoHz(freqToNote(0.01), freqToNote(20.0), true);
LinearScale<double> Scales::lfoShiftOffset(-1.0, 1.0);

DecibelScale<double> Scales::gain(-24, 24, true);

LogScale<double> Scales::smoothness(0.04, 8.0, 0.5, 1.0);

} // namespace Synth
} // namespace Steinberg
