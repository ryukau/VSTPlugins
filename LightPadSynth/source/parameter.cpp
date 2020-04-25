// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

DecibelScale<double> Scales::overtoneGain(-60.0, 0.0, true);
LinearScale<double> Scales::overtoneWidth(0.0, 100.0);
LogScale<double> Scales::overtonePitch(0.0625, 16.0, 0.5, 1.0);
LinearScale<double> Scales::overtonePhase(0.0, twopi);

LogScale<double> Scales::tableBaseFrequency(0.1, 100.0, 0.5, 2.5);
IntScale<double> Scales::tableBufferSize(11); // Max 1024 * 2^11
LogScale<double> Scales::overtoneGainPower(0.2, 10.0, 0.5, 1.0);
LogScale<double> Scales::overtoneWidthMultiply(0.05, 12.0, 0.5, 1.0);
LinearScale<double> Scales::overtonePitchMultiply(0.0001, 16.0);
LinearScale<double> Scales::overtonePitchModulo(0.0, 136.0);
LogScale<double> Scales::spectrumExpand(0.03125, 32.0, 0.5, 1.0);
IntScale<double> Scales::profileComb(255);
LogScale<double> Scales::profileShape(0.0, 4.0, 0.5, 1.0);
IntScale<double> Scales::seed(16777215); // 2^24 - 1

LogScale<double> Scales::gain(0.0, 2.0, 0.5, 0.5);
LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 4.0);
LogScale<double> Scales::envelopeS(0.0001, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<double> Scales::filterCutoff(0.0, 22000.0, 0.5, 400.0);
LinearScale<double> Scales::filterResonance(0.0, 0.999);

LinearScale<double> Scales::delayFeedback(-1.0, 1.0);
IntScale<double> Scales::delayDetuneSemi(240);

IntScale<double> Scales::oscOctave(16);
IntScale<double> Scales::oscSemi(168);
IntScale<double> Scales::oscMilli(2000);
IntScale<double> Scales::equalTemperament(119);
IntScale<double> Scales::pitchA4Hz(900);

LinearScale<double> Scales::lfoWavetable(-1.0, 1.0);
IntScale<double> Scales::lfoWavetableType(2);
IntScale<double> Scales::lfoTempoNumerator(255);
IntScale<double> Scales::lfoTempoDenominator(255);
LogScale<double> Scales::lfoFrequencyMultiplier(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::lfoDelayAmount(0.0, 1.0, 0.5, 0.2);

IntScale<double> Scales::nUnison(15);
LogScale<double> Scales::unisonDetune(0.0, 1.0, 0.5, 0.05);
IntScale<double> Scales::unisonPanType(9);

IntScale<double> Scales::nVoice(7);
LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

} // namespace Synth
} // namespace Steinberg
