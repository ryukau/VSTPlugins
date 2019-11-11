// (c) 2019 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LinearScale<double> Scales::semi(-24.0, 24.0);
LinearScale<double> Scales::cent(-100.0, 100.0);
LinearScale<double> Scales::overtone(1.0, 32.0);
LinearScale<double> Scales::pitchDrift(0.0, 3.0);
LogScale<double> Scales::oscSlope(1.0, 32.0, 0.5, 8.0);
LogScale<double> Scales::oscFeedback(0.0, 1.0, 0.5, 0.1);
LogScale<double> Scales::phaseModulation(0.0, 1.0, 0.5, 0.2);

LogScale<double> Scales::envelopeA(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::envelopeD(0.01, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeS(0.0, 1.0, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.0012, 16.0, 0.5, 2.0);

LogScale<double> Scales::filterCutoff(20.0, 20000.0, 0.5, 1000.0);
LogScale<double> Scales::filterResonance(0.0, 1.0, 0.5, 0.3);
LogScale<double> Scales::filterSaturation(0.01, 8.0, 0.3, 0.5);
IntScale<double> Scales::filterOrder(7);
SPolyScale<double> Scales::filterEnvToCutoff(-1.0, 1.0, 0.3);
LinearScale<double> Scales::filterKeyToCutoff(0.0, 2.0);
LinearScale<double> Scales::oscMixToFilterCutoff(0.0, 0.5);
LinearScale<double> Scales::filterEnvToOctave(-9.0, 9.0);

LogScale<double> Scales::modEnvelopeA(0.0, 4.0, 0.5, 0.5);
LogScale<double> Scales::modEnvelopeCurve(1.0, 96.0, 0.5, 8.0);
LogScale<double> Scales::modEnvToShifter(0.0, 4000.0, 0.5, 500.0);

LinearScale<double> Scales::shifterSemi(0.0, 24.0);
LinearScale<double> Scales::shifterCent(0.0, 100.0);

IntScale<double> Scales::lfoType(3);
LogScale<double> Scales::lfoFrequency(0.1, 20.0, 0.5, 4.0);
LogScale<double> Scales::lfoSlope(0.0, 32.0, 0.5, 8.0);

LinearScale<double> Scales::octave(-4.0, 4.0);

IntScale<double> Scales::pitchSlideType(2);
LogScale<double> Scales::pitchSlide(0.001, 0.5, 0.5, 0.15);
LinearScale<double> Scales::pitchSlideOffset(0.0, 2.0);

LogScale<double> Scales::smoothness(0.001, 0.5, 0.5, 0.2);

LogScale<double> Scales::gain(0.0, 4.0, 0.5, 0.75);

} // namespace Synth
} // namespace Steinberg
