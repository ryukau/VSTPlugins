// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

UIntScale<double> Scales::seed(16777215); // 2^24 - 1
LogScale<double> Scales::randomAmount(0.0, 1.0, 0.5, 0.1);
LinearScale<double> Scales::nCymbal(0.0, 3.0);
LogScale<double> Scales::decay(0.0, 16.0, 0.5, 4.0);
LogScale<double> Scales::damping(0.0, 0.999, 0.5, 0.9);
LogScale<double> Scales::minFrequency(0.0, 1000.0, 0.5, 100.0);
LogScale<double> Scales::maxFrequency(10.0, 4000.0, 0.5, 400.0);
LinearScale<double> Scales::bandpassQ(0.0001, 0.9999);
LogScale<double> Scales::distance(0.0, 8.0, 0.5, 0.1);
UIntScale<double> Scales::stack(63);
LogScale<double> Scales::pickCombFeedback(0.0, 0.9999, 0.5, 0.7);
LogScale<double> Scales::pickCombTime(0.005, 0.4, 0.5, 0.1);
UIntScale<double> Scales::oscType(4);
LogScale<double> Scales::smoothness(0.0, 0.1, 0.2, 0.02);

LogScale<double> Scales::gain(0.0, 4.0, 0.75, 1.0);

} // namespace Synth
} // namespace Steinberg
