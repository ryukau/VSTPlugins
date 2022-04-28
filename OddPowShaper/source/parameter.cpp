// (c) 2020-2022 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::drive(1.0, 32.0, 0.5, 4.0);
LinearScale<double> Scales::boost(1.0, 32.0);
LogScale<double> Scales::outputGain(0.0, 1.0, 0.5, 0.1);
UIntScale<double> Scales::order(15);

LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

LogScale<double> Scales::limiterThreshold(0.01, 2.0, 0.5, 0.5);
LogScale<double> Scales::limiterAttack(0.0001, 0.021328, 0.1, 0.002); // Depricated.
LogScale<double> Scales::limiterRelease(0.0001, 0.2, 0.2, 0.01);

constexpr double maxClip = 1024.0;
LinearScale<double> Scales::guiInputGainScale(0.0, maxClip);

} // namespace Synth
} // namespace Steinberg
