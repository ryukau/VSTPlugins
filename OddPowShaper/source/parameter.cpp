// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
