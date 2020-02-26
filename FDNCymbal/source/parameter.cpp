// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

IntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

IntScale<double> Scales::seed(16777215); // 2^24 - 1
LogScale<double> Scales::fdnTime(0.0001, 0.5, 0.5, 0.1);
LogScale<double> Scales::fdnFeedback(0.0, 4.0, 0.75, 1.0);
LogScale<double> Scales::fdnCascadeMix(0.0, 1.0, 0.5, 0.2);
LogScale<double> Scales::allpassTime(0.0, 0.005, 0.5, 0.001);
LogScale<double> Scales::allpassFeedback(0.0, 0.9999, 0.5, 0.9);
LogScale<double> Scales::allpassHighpassCutoff(1.0, 40.0, 0.5, 10.0);
LogScale<double> Scales::tremoloFrequency(0.1, 20, 0.5, 4.0);
LogScale<double> Scales::tremoloDelayTime(0.00003, 0.001, 0.5, 0.0001);
LogScale<double> Scales::stickDecay(0.01, 4.0, 0.5, 0.1);
LogScale<double> Scales::stickToneMix(0.0, 0.02, 0.5, 0.001);

LogScale<double> Scales::smoothness(0.0, 0.5, 0.2, 0.02);

LogScale<double> Scales::gain(0.0, 4.0, 0.75, 0.5);

} // namespace Synth
} // namespace Steinberg
