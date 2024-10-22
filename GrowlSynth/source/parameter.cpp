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
DecibelScale<double> Scales::safetyHighpassHz(ampToDB(0.1), ampToDB(20.0), false);
DecibelScale<double> Scales::cutoffHz(20.0, 100.0, false);
LinearScale<double> Scales::pulseBendOctave(0.0, 8.0);
LinearScale<double> Scales::formantOctave(-6.0, 6.0);

DecibelScale<double> Scales::envelopeSeconds(-60, 40.0, true);
DecibelScale<double> Scales::envelopeAM(-100.0, 0.0, true);

DecibelScale<double> Scales::frequencyRatio(-20.0, 20.0, false);
LinearScale<double> Scales::cutoffMod(-4.0, 4.0);
DecibelScale<double> Scales::filterQ(-20.0, 20.0, false);
DecibelScale<double> Scales::energyLossThreshold(-20.0, 20.0, false);
DecibelScale<double> Scales::frequencyHz(20.0, 80.0, false);
LinearScale<double> Scales::randomOctave(0, 2.0);
NegativeDecibelScale<double> Scales::feedbackGain(-80.0, 0.0, 1.0, true);
DecibelScale<double> Scales::slewRate(-40.0, 40.0, false);
DecibelScale<double> Scales::maxTimeSpreadSeconds(-80.0, ampToDB(0.2), true);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::a4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::noteSlideTimeSecond(-100.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
