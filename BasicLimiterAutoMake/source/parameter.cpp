// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
DecibelScale<double> Scales::limiterThreshold(-30.0, 30.0, false);
DecibelScale<double> Scales::limiterGate(-100.0, 0.0, true);
LogScale<double> Scales::limiterAttack(0.0001, maxAttackSeconds, 0.5, 0.002);
LogScale<double> Scales::limiterRelease(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::limiterSustain(0.0, maxAttackSeconds, 0.1, 0.002);
LinearScale<double> Scales::overshoot(1.0, 32.0);
UIntScale<double> Scales::channelType(1);

} // namespace Synth
} // namespace Steinberg
