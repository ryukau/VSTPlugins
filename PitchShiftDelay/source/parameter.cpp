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

LinearScale<double> Scales::lfoWavetable(-1.0, 1.0);
UIntScale<double> Scales::lfoInterpolation(2);
UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);

DecibelScale<double> Scales::pitch(-60.0, 60.0, true);
DecibelScale<double> Scales::delayTime(-100.0, 20.0, true);
LinearScale<double> Scales::stereoLean(-1.0, 1.0);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);
SemitoneScale<double> Scales::highpassCutoffHz(-37.0, 117.0, true);

DecibelScale<double> Scales::dry(-40.0, 0.0, true);
DecibelScale<double> Scales::wet(-40.0, 40.0, true);
DecibelScale<double> Scales::smoothingTime(-40.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
