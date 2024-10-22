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
LogScale<double> Scales::timeMultiply(0.0, 1.0, 0.5, 0.1);
LogScale<double> Scales::timeOffsetRange(0.0, 1.0, 0.5, 0.2);
UIntScale<double> Scales::seed(16777215); // 2^24 - 1
LinearScale<double> Scales::stereoCross(-1.0, 1.0);
LogScale<double> Scales::gain(0.0, 4.0, 0.5, 1.0);
LogScale<double> Scales::smoothness(0.0, 8.0, 0.5, 1.0);

} // namespace Synth
} // namespace Steinberg
