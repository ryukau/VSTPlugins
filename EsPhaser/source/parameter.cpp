// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::frequency(0.0, 16.0, 0.5, 2.0);
LinearScale<double> Scales::feedback(-1.0, 1.0);
LogScale<double> Scales::range(0.0, 128.0, 0.5, 32.0);
LinearScale<double> Scales::phase(0.0, twopi);
LinearScale<double> Scales::cascadeOffset(0.0, twopi);
IntScale<double> Scales::stage(4095);

LogScale<double> Scales::smoothness(0.04, 1.0, 0.5, 0.4);

} // namespace Synth
} // namespace Steinberg
