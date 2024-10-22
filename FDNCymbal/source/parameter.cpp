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
LogScale<double> Scales::fdnTime(0.0001, 0.5, 0.5, 0.1);
LogScale<double> Scales::fdnFeedback(0.0, 4.0, 0.75, 1.0);
LogScale<double> Scales::fdnCascadeMix(0.0, 1.0, 0.5, 0.2);
LogScale<double> Scales::allpassTime(0.0, 0.005, 0.5, 0.001);
LogScale<double> Scales::allpassFeedback(0.0, 0.9999, 0.5, 0.9);
LogScale<double> Scales::allpassHighpassCutoff(1.0, 40.0, 0.5, 10.0);
LogScale<double> Scales::tremoloFrequency(0.1, 20, 0.5, 4.0);
LogScale<double> Scales::tremoloDelayTime(0.00003, 0.001, 0.5, 0.0001);
LogScale<double> Scales::stickDecay(0.01, 4.0, 0.5, 0.1);
LogScale<double> Scales::stickToneMix(0.0, 0.02, 0.5, 0.001);

LogScale<double> Scales::smoothness(0.0, 0.5, 0.2, 0.02);

LogScale<double> Scales::gain(0.0, 4.0, 0.75, 0.5);

} // namespace Synth
} // namespace Steinberg
