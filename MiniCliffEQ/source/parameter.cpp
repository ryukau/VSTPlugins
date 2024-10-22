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

// Range in [1, 24000] Hz.
SemitoneScale<double> Scales::cutoffHz(-36.376316562295926, 138.232644862303, false);
DecibelScale<double> Scales::gain(-144.5, 144.5, true);

} // namespace Synth
} // namespace Steinberg
