// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::gain(-145.0, 145.0, true);
DecibelScale<double>
  Scales::limiterAttackSeconds(-100.0, 20 * std::log10(maxLimiterAttackSeconds), true);
DecibelScale<double> Scales::limiterReleaseSeconds(-100.0, 60.0, true);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
