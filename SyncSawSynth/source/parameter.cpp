// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::oscGain(0.0, 1.0, 0.75, 0.5);
LinearScale<double> Scales::semi(-24.0, 24.0);
LinearScale<double> Scales::cent(-100.0, 100.0);
LinearScale<double> Scales::sync(0.01, 16.0);
IntScale<double> Scales::osc1SyncType(3);
IntScale<double> Scales::osc1PTROrder(16);

LogScale<double> Scales::fmToSync(0.0, 1.0, 0.5, 0.1);
LogScale<double> Scales::fmToFreq(0.0, 1.0, 0.5, 0.1);

LogScale<double> Scales::gain(0.0, 4.0, 0.75, 1.0);

LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 1.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 1.0);
LogScale<double> Scales::envelopeS(0.0, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<double> Scales::filterCutoff(20.0, 20000.0, 0.5, 800.0);
LogScale<double> Scales::filterResonance(0.001, 1.0, 0.5, 0.3);
LogScale<double> Scales::filterFeedback(0.0, 1.0, 0.5, 0.3);
LogScale<double> Scales::filterSaturation(0.01, 8.0, 0.3, 0.5);
IntScale<double> Scales::filterType(4);
IntScale<double> Scales::filterShaper(3);
LinearScale<double> Scales::filterCutoffAmount(-1.0, 1.0);
LinearScale<double> Scales::filterKeyMod(-1.0, 1.0);

LogScale<double> Scales::modEnvelopeA(0.0, 4.0, 0.5, 0.5);
LogScale<double> Scales::modEnvelopeCurve(1.0, 96.0, 0.5, 8.0);
LogScale<double> Scales::modLFOFrequency(0.01, 20.0, 0.5, 1.0);
LogScale<double> Scales::modToFreq(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::modToSync(0.0, 16.0, 0.5, 1.0);

IntScale<double> Scales::nVoice(5);

} // namespace Synth
} // namespace Steinberg
