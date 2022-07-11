// (c) 2021 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
UIntScale<double> Scales::seed(1 << 23);

UIntScale<double> Scales::octave(16);
UIntScale<double> Scales::semitone(168);
UIntScale<double> Scales::milli(2000);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::pitchA4Hz(900);

DecibelScale<double> Scales::gain(-60.0, 60.0, true);
DecibelScale<double> Scales::gateRelease(-80.0, 40, false);

LinearScale<double> Scales::oscOvertone(-1.0, 1.0);
DecibelScale<double> Scales::oscAttack(-80.0, 40, false);
DecibelScale<double> Scales::oscDecay(-80.0, 40, false);
UIntScale<double> Scales::oscOctave(24);
LinearScale<double> Scales::oscFinePitch(-120.0, 120.0);
DecibelScale<double> Scales::oscSpectrumDenominatorSlope(-60.0, 40.0, true);
LinearScale<double> Scales::oscSpectrumRotationSlope(-1.0, 1.0);
UIntScale<double> Scales::oscSpectrumInterval(31);
UIntScale<double> Scales::oscSpectrumHighpass(256);
DecibelScale<double> Scales::oscSpectrumBlur(-100.0, 0.0, false);

DecibelScale<double> Scales::impulseGain(-100.0, 0.0, true);

DecibelScale<double> Scales::fdnMatrixIdentityAmount(-60.0, 60.0, false);
LinearScale<double> Scales::fdnOvertoneAdd(0.0, 64.0);
LinearScale<double> Scales::fdnOvertoneMul(0.0, 64.0);
LinearScale<double> Scales::fdnOvertoneOffset(0.0, 64.0);
DecibelScale<double> Scales::fdnInterpRate(-40.0, 40.0, false);
DecibelScale<double> Scales::fdnInterpLowpassSecond(-120.0, 40.0, true);

LinearScale<double> Scales::filterCutoffSemi(-120.0, 200.0);
LinearScale<double> Scales::filterQ(0.01, halfSqrt2);

UIntScale<double> Scales::nUnison(maximumVoice - 1);
DecibelScale<double> Scales::unisonDetune(0.0, 6.0, false);
LinearScale<double> Scales::unisonPan(0.0, 1.0);

LinearScale<double> Scales::lfoWavetable(-1.0, 1.0);
UIntScale<double> Scales::lfoInterpolation(2);
UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);
LinearScale<double> Scales::lfoToPitchAmount(-120.0, 120.0);
UIntScale<double> Scales::lfoToPitchAlignment(120);

} // namespace Synth
} // namespace Steinberg
