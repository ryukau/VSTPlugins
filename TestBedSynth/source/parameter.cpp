// (c) 2021 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

LinearScale<double> Scales::waveform(0.0, 3.0);

LinearScale<double> Scales::oscPitchSemitone(-60.0, 60.0);
DecibelScale<double> Scales::hardSync(-20, 20, false);

DecibelScale<double> Scales::spectralSpread(-40, 40, false);
DecibelScale<double> Scales::phaseSlope(0, 60, true);
DecibelScale<double> Scales::spectralFilterCutoff(-67, 0, true);

LinearScale<double> Scales::modSpectralFilterCutoff(-8.0, 8.0);

BipolarDecibelScale<double> Scales::sumToPm(-80, 40.0);
LinearScale<double> Scales::sumToFm(-16.0, 16.0);

DecibelScale<double> Scales::envelopeSecond(-60.0, 40, true);
DecibelScale<double> Scales::envelopeSustainAmplitude(-40.0, 0, true);

DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);

DecibelScale<double> Scales::gain(-60, 40, true);
DecibelScale<double> Scales::cutoffHz(-20, 100, false);

UIntScale<double> Scales::octave(16);
UIntScale<double> Scales::semitone(168);
UIntScale<double> Scales::milli(2000);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::pitchA4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);
UIntScale<double> Scales::oversampling(1);
UIntScale<double> Scales::nVoice(maxVoice - 1);

} // namespace Synth
} // namespace Steinberg
