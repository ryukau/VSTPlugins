// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

LinearScale<double> Scales::defaultScale(0.0, 1.0);
IntScale<double> Scales::boolScale(1);
LogScale<double> Scales::time(0.0, 1.0, 0.5, 0.05);
LinearScale<double> Scales::feed(-1.0, 1.0);
LinearScale<double> Scales::timeOffset(-1.0, 1.0);
LinearScale<double> Scales::feedOffset(-1.0, 1.0);
LinearScale<double> Scales::multiply(0.0, 1.0);
LogScale<double> Scales::timeLfoLowpas(0.0, 1.0, 0.5, 0.2);
LinearScale<double> Scales::stereoCross(0.0, 1.0);
LogScale<double> Scales::gain(0.0, 4.0, 0.5, 1.0);
LogScale<double> Scales::smoothness(0.0, 1.0, 0.5, 0.25);

} // namespace Synth
} // namespace Steinberg
