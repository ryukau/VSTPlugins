// (c) 2023 Takamitsu Endo
//
// This file is part of GenericDrum.
//
// GenericDrum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenericDrum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenericDrum.  If not, see <https://www.gnu.org/licenses/>.

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
DecibelScale<double> Scales::safetyHighpassHz(ampToDB(0.1), ampToDB(100.0), false);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);
DecibelScale<double> Scales::noteSlideTimeSecond(-100.0, 40.0, true);

DecibelScale<double> Scales::noiseDecaySeconds(-40, ampToDB(0.5), false);

DecibelScale<double> Scales::wireFrequencyHz(0, ampToDB(1000), false);
DecibelScale<double> Scales::wireDecaySeconds(-40, 40, false);

DecibelScale<double> Scales::crossFeedbackGain(-12, 0, false);
DecibelScale<double> Scales::feedbackDecaySeconds(-40, 20, false);

LinearScale<double> Scales::pitchRandomCent(0, 1200);
DecibelScale<double> Scales::envelopeSeconds(-60, 40, false);
DecibelScale<double> Scales::envelopeModAmount(-20, 20, true);

UIntScale<double> Scales::pitchType(8);
DecibelScale<double> Scales::delayTimeHz(ampToDB(2), ampToDB(10000), false);
DecibelScale<double> Scales::delayTimeModAmount(-20, 100, true);
LinearScale<double> Scales::bandpassCutRatio(-8, 8);
DecibelScale<double> Scales::bandpassQ(-40, 40, false);

DecibelScale<double> Scales::collisionDistance(-80, 40, true);

} // namespace Synth
} // namespace Steinberg
