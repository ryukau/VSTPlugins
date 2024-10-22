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

DecibelScale<double> Scales::gain(-60.0, 20.0, true);
DecibelScale<double> Scales::delayTimeSeconds(-80.0, 20 * std::log10(maxDelayTime), true);
LinearScale<double> Scales::shiftPitch(-4.0, 4.0);
DecibelScale<double> Scales::shiftHz(-40.0, 100.0, true);
DecibelScale<double> Scales::cutoffHz(0.0, 100.0, false);
LinearScale<double> Scales::lfoToPrimary(-4.0, 4.0);

UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);
DecibelScale<double> Scales::lfoShapeClip(0.0, 20.0, false);
DecibelScale<double> Scales::lfoShapeSkew(-20.0, 20.0, false);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);
UIntScale<double> Scales::oversampling(2);

} // namespace Synth
} // namespace Steinberg
