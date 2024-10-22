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

LogScale<double> Scales::delayTime(0.0, maxDelayTime, 0.5, 0.1 * maxDelayTime);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);

// log2(0.1 Hz / 440) * 12 + 69 = -76.23945370094427.
// log2(24000 Hz / 440) * 12 + 69 = 138.232644862303.
SemitoneScale<double>
  Scales::feedbackHighpassCutoffHz(-76.23945370094427, 138.232644862303, true);

LogScale<double> Scales::feedbackLimiterRelease(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::delayTimeInterpRate(0.0, 4.0, 0.5, 0.25);
DecibelScale<double> Scales::delayTimeInterpLowpassSeconds(-80.0, 40.0, true);
LinearScale<double> Scales::stereoLean(-1.0, 1.0);
DecibelScale<double> Scales::feedbackToDelayTime(-80.0, 40.0, true);

DecibelScale<double> Scales::gateThreshold(-140.0, 0.0, true);
DecibelScale<double> Scales::gateRelease(-80.0, 20, false);

DecibelScale<double> Scales::dry(-60.0, 24.0, true);
DecibelScale<double> Scales::wet(-60.0, 60.0, true);

} // namespace Synth
} // namespace Steinberg
