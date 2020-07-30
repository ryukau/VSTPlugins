// (c) 2020 Takamitsu Endo
//
// This file is part of CollidingCombSynth.
//
// CollidingCombSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CollidingCombSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CollidingCombSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::gain(0.0, 1.0, 0.5, 0.2);
LogScale<double> Scales::boost(0.0, 8.0, 0.5, 1.0);

LogScale<double> Scales::exciterGain(0.001, 2.0, 0.5, 0.5);
LogScale<double> Scales::exciterAttack(0.001, 0.2, 0.5, 0.1);
LogScale<double> Scales::exciterDecay(0.001, 0.5, 0.5, 0.1);
LogScale<double> Scales::exciterLowpassCutoff(1.0, 20000.0, 0.5, 100.0);

LinearScale<double> Scales::combTime(0.0001, 0.002);
LogScale<double> Scales::frequency(12.0, 1000.0, 0.5, 100.0);
LogScale<double> Scales::lowpassCutoff(20.0, 20000.0, 0.5, 1000.0);
LogScale<double> Scales::highpassCutoff(20.0, 20000.0, 0.5, 400.0);

LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 4.0);
LogScale<double> Scales::envelopeS(0.0, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<double> Scales::distance(0.001, 1.0, 0.5, 0.2);
IntScale<double> Scales::seed(16777215); // 2^24 - 1

LogScale<double> Scales::randomFrequency(0.0, 1.0, 0.5, 0.05);

LogScale<double> Scales::compressorTime(0.001, 8.0, 0.5, 1.0);
LogScale<double> Scales::compressorThreshold(0.01, 2.0, 0.5, 0.5);

IntScale<double> Scales::nVoice(15);
IntScale<double> Scales::nUnison(7);
LogScale<double> Scales::unisonDetune(0.0, 0.3, 0.2, 0.001);

IntScale<double> Scales::octave(16);
IntScale<double> Scales::semitone(168);
IntScale<double> Scales::milli(2000);
IntScale<double> Scales::equalTemperament(119);
IntScale<double> Scales::pitchA4Hz(900);

} // namespace Synth
} // namespace Steinberg
