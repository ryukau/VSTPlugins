// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

constexpr double maxClip = 16.0;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::inputGain(0.0, 4.0, 0.5, 1.0);
LogScale<double> Scales::outputGain(0.0, 2.0, 0.5, 0.2);
LogScale<double> Scales::clip(0.0, maxClip, 0.5, 4.0);
UIntScale<double> Scales::orderInteger(16);

LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

LinearScale<double> Scales::guiInputGainScale(0.0, maxClip);

} // namespace Synth
} // namespace Steinberg
