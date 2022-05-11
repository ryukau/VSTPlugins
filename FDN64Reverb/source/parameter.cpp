// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LogScale<double> Scales::delayTime(0.0, 1.0, 0.5, 0.05);
SemitoneScale<double> Scales::lowpassCutoffHz(33.0, 136.0, false);
SemitoneScale<double> Scales::highpassCutoffHz(-37.0, 57.0, true);
UIntScale<double>
  Scales::matrixType(FeedbackMatrixType::FeedbackMatrixType_ENUM_LENGTH - 1);
DecibelScale<double> Scales::gateThreshold(-140.0, 0.0, true);
DecibelScale<double> Scales::dry(-60.0, 24.0, true);
DecibelScale<double> Scales::wet(-60.0, 60.0, true);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);
LogScale<double> Scales::delayTimeInterpRate(0.0, 4.0, 0.5, 0.25);
UIntScale<double> Scales::seed(1 << 23);
LogScale<double> Scales::splitRotationHz(0.0, 10.0, 0.5, 0.2);
LinearScale<double> Scales::splitSkew(0.0, 6.0);

} // namespace Synth
} // namespace Steinberg
