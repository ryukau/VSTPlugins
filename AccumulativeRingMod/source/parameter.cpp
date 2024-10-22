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

DecibelScale<double> Scales::outputGain(-60.0, 20.0, true);
DecibelScale<double> Scales::stereoPhaseLinkHz(-40.0, 100.0, true);
DecibelScale<double> Scales::modulation(-80.0, 40.0, true);
DecibelScale<double> Scales::cutoffHz(-20.0, 100.0, false);
DecibelScale<double> Scales::gateThreshold(-140.0, 0.0, true);
DecibelScale<double> Scales::envelopeSecond(-100.0, 40.0, true);

UIntScale<double> Scales::oversampling(2);
DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
