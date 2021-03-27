// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::time(0.0001, maxDelayTime, 0.5, 1.0);
SPolyScale<double> Scales::offset(-1.0, 1.0, 0.8);
LogScale<double> Scales::lfoTimeAmount(0, 1.0, 0.5, 0.07);
LogScale<double> Scales::lfoToneAmount(0, 0.5, 0.5, 0.1);
LogScale<double> Scales::lfoFrequency(0.01, 100.0, 0.5, 1.0);
LogScale<double> Scales::lfoShape(0.01, 10.0, 0.5, 1.0);
LinearScale<double> Scales::lfoInitialPhase(0.0, 2.0 * pi);
LogScale<double> Scales::smoothness(0.0, 1.0, 0.3, 0.04);
LogScale<double> Scales::toneCutoff(90.0, maxToneFrequency, 0.5, 1000.0);
LogScale<double> Scales::toneQ(1e-5, 1.0, 0.5, 0.1);
LogScale<double> Scales::toneMix(0.0, 1.0, 0.9, 0.05);
LogScale<double> Scales::dckill(minDCKillFrequency, 120.0, 0.5, 20.0);
LogScale<double> Scales::dckillMix(0.0, 1.0, 0.9, 0.05);

} // namespace Synth
} // namespace Steinberg
