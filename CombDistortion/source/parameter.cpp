// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

constexpr double eps = std::numeric_limits<double>::epsilon();

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::outputGain(-60.0, 20.0, true);
LinearScale<double> Scales::feedback(-1.0 + eps, 1.0 - eps);
DecibelScale<double> Scales::highpassHz(0.0, 80.0, true);
DecibelScale<double> Scales::lowpassHz(0.0, 100.0, true);
DecibelScale<double> Scales::delayTimeSeconds(-100.0, 0.0, true);
DecibelScale<double> Scales::gain(-60.0, 60.0, true);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);
UIntScale<double> Scales::oversampling(2);
LinearScale<double> Scales::notePitchOrigin(0.0, 136.0);

} // namespace Synth
} // namespace Steinberg
