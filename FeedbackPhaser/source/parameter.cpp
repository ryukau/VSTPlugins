// (c) 2023 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::outputGain(-145.0, 20.0, true);
LinearScale<double> Scales::feedback(-0.99, 0.99);
DecibelScale<double> Scales::feedbackClip(-145.0, 145.0, false);

UIntScale<double> Scales::stage(maxAllpass - 1);
LinearScale<double> Scales::allpassSpread(0.0, 2.0);
DecibelScale<double> Scales::cutoffHz(-20.0, 100.0, false);

UIntScale<double> Scales::modType(2);
DecibelScale<double> Scales::modulation(-60.0, 80.0, true);
LinearScale<double> Scales::modAsymmetry(-1.0, 1.0);

LinearScale<double> Scales::notePitchCenter(0.0, 136.0);
LinearScale<double> Scales::notePitchToAllpassCutoff(-2.0, 2.0);
DecibelScale<double> Scales::notePitchEnvelopeSecond(-120.0, 40.0, true);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);
UIntScale<double> Scales::oversampling(2);

} // namespace Synth
} // namespace Steinberg
