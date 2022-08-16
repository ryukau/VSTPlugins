// (c) 2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

constexpr auto eps = std::numeric_limits<float>::epsilon();

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::outputGain(-60.0, 20.0, true);
DecibelScale<double> Scales::envelopeAttackSecond(-100.0, 20.0, true);
DecibelScale<double> Scales::envelopeSecond(-60.0, 20.0, false);
DecibelScale<double> Scales::gainSustainAmplitude(-40.0, 0.0, true);

UIntScale<double> Scales::octave(octaveOffset + 4);
LinearScale<double> Scales::fineTuneCent(-4800.0, 4800.0);
LinearScale<double> Scales::waveShape(eps, 1.0 - eps);
DecibelScale<double> Scales::phaseMod(-140.0, 0.0, true);

DecibelScale<double> Scales::lowpassCutoffHz(0.0, 120.0, false);
DecibelScale<double> Scales::lowpassQ(-40.0, 40.0, false);
DecibelScale<double> Scales::lowpassCutoffEnvelopeAmount(-80.0, 80.0, false);

UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::a4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::noteSlideTimeSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
