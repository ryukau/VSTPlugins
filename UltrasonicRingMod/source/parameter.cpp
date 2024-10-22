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

DecibelScale<double> Scales::gain(-60.0, 60.0, true);
DecibelScale<double> Scales::frequencyHz(0.0, 120.0, false);
DecibelScale<double> Scales::feedbackGain(-80.0, 80.0, true);
DecibelScale<double> Scales::modFrequencyScaling(-100.0, 0.0, true);
NegativeDecibelScale<double> Scales::modWrapMix(-60.0, 0.0, 1.0, true);

LinearScale<double> Scales::noteScaling(0.0, 4.0);
LinearScale<double> Scales::noteOffset(-192.0, 192.0);
DecibelScale<double> Scales::noteSlideTimeSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
