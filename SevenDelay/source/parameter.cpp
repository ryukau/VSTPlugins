// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::time(0.0001, maxDelayTime, 0.5, 1.0);
SPolyScale<double> Scales::offset(-1.0, 1.0, 0.8);
LogScale<double> Scales::lfoTimeAmount(0, 1.0, 0.5, 0.07);
LogScale<double> Scales::lfoToneAmount(0, 0.5, 0.5, 0.1);
LogScale<double> Scales::lfoFrequency(0.01, 100.0, 0.5, 1.0);
LogScale<double> Scales::lfoShape(0.01, 10.0, 0.5, 1.0);
LinearScale<double> Scales::lfoInitialPhase(0.0, 2.0 * pi);
LogScale<double> Scales::smoothness(0.0, 1.0, 0.3, 0.04);
LogScale<double> Scales::toneCutoff(90.0, maxToneFrequency, 0.5, 1000.0);
LogScale<double> Scales::toneQ(1e-5, 1.0, 0.5, 0.1);
LogScale<double> Scales::toneMix(0.0, 1.0, 0.9, 0.05);
LogScale<double> Scales::dckill(minDCKillFrequency, 120.0, 0.5, 20.0);
LogScale<double> Scales::dckillMix(0.0, 1.0, 0.9, 0.05);

} // namespace Synth
} // namespace Steinberg
