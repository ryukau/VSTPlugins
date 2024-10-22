// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::frequency(0.0, 16.0, 0.5, 2.0);
LinearScale<double> Scales::feedback(-1.0, 1.0);
LogScale<double> Scales::range(0.0, 128.0, 0.5, 32.0);
LinearScale<double> Scales::phase(0.0, twopi);
LinearScale<double> Scales::cascadeOffset(0.0, twopi);
UIntScale<double> Scales::stage(4095);

LogScale<double> Scales::smoothness(0.04, 1.0, 0.5, 0.4);

} // namespace Synth
} // namespace Steinberg
