// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"

#include <cmath>
#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

template<typename T> inline T ampToDB(T amp) { return T(20) * std::log10(amp); }

constexpr auto eps = std::numeric_limits<float>::epsilon();

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);
UIntScale<double> Scales::seed(1 << 23);

DecibelScale<double> Scales::gain(-100.0, 60.0, true);
DecibelScale<double> Scales::safetyHighpassHz(ampToDB(0.1), ampToDB(100.0), false);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);
DecibelScale<double> Scales::noteSlideTimeSecond(-40.0, 40.0, false);

DecibelScale<double> Scales::noiseDecaySeconds(-40, ampToDB(0.5), false);

DecibelScale<double> Scales::wireFrequencyHz(0, ampToDB(1000), false);
DecibelScale<double> Scales::wireDecaySeconds(-40, 40, false);

DecibelScale<double> Scales::crossFeedbackGain(-12, 0, false);
DecibelScale<double> Scales::crossFeedbackConsistency(-60, 0, true);
DecibelScale<double> Scales::feedbackDecaySeconds(-40, 20, false);

LinearScale<double> Scales::pitchRandomCent(0, 1200);
DecibelScale<double> Scales::envelopeSeconds(-60, 40, false);
DecibelScale<double> Scales::envelopeModAmount(-20, 20, true);

UIntScale<double> Scales::pitchType(32);
DecibelScale<double> Scales::delayTimeHz(ampToDB(2), ampToDB(10000), false);
DecibelScale<double> Scales::delayTimeModAmount(-20, 100, true);
LinearScale<double> Scales::bandpassCutRatio(-8, 8);
DecibelScale<double> Scales::bandpassQ(-40, 40, false);

DecibelScale<double> Scales::collisionDistance(-80, 40, true);

LinearScale<double> Scales::amplitudeMeter(0.0, 1000.0);

} // namespace Synth
} // namespace Steinberg
