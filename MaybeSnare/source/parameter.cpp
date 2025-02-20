// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

constexpr auto eps = std::numeric_limits<float>::epsilon();

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);
UIntScale<double> Scales::seed(1 << 23);

DecibelScale<double> Scales::outputGain(-100.0, 40.0, true);

DecibelScale<double> Scales::impactAmplitude(-20.0, 60.0, false);
DecibelScale<double> Scales::impactDecaySeconds(-80.0, 20.0, false);

LinearScale<double> Scales::snareSideOctaveOffset(-4.0, 4.0);
DecibelScale<double> Scales::couplingAmount(-60.0, 40.0, true);
DecibelScale<double> Scales::couplingDecaySeconds(-60.0, 40.0, false);

DecibelScale<double> Scales::fdnMatrixIdentityAmount(-60.0, 60.0, false);
NegativeDecibelScale<double> Scales::fdnFeedback(-60.0, 0.0, 1.0, true);
DecibelScale<double> Scales::fdnInterpRate(-40.0, 40.0, true);
LinearScale<double> Scales::fdnMaxModulationRatio(0.0, 0.9);
DecibelScale<double>
  Scales::filterCutoffHz(0.0, 86.02059991327963, false); // 1 to 20000 Hz.
LinearScale<double> Scales::filterQ(0.01, halfSqrt2);

DecibelScale<double> Scales::envelopeSeconds(-80.0, 40.0, true);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::a4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::noteSlideTimeSecond(-100.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
