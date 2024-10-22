// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

LinearScale<double> Scales::defaultScale(0.0, 1.0);
UIntScale<double> Scales::boolScale(1);
LogScale<double> Scales::time(0.0, 1.0, 0.5, 0.05);
LinearScale<double> Scales::feed(-1.0, 1.0);
LinearScale<double> Scales::timeOffset(-1.0, 1.0);
LinearScale<double> Scales::feedOffset(-1.0, 1.0);
LinearScale<double> Scales::multiply(0.0, 1.0);
LogScale<double> Scales::timeLfoLowpas(0.0, 1.0, 0.5, 0.2);
LinearScale<double> Scales::stereoCross(0.0, 0.5);
LogScale<double> Scales::gain(0.0, 4.0, 0.5, 1.0);
LogScale<double> Scales::smoothness(0.0, 8.0, 0.5, 1.0);

} // namespace Synth
} // namespace Steinberg
