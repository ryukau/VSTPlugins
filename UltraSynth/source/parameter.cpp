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

DecibelScale<double> Scales::outputGain(-60.0, 20.0, true);
DecibelScale<double> Scales::envelopeAttackSecond(-100.0, 20.0, true);
DecibelScale<double> Scales::envelopeSecond(-60.0, 20.0, false);
DecibelScale<double> Scales::gainSustainAmplitude(-40.0, 0.0, true);

UIntScale<double> Scales::octave(octaveOffset + 4);
LinearScale<double> Scales::fineTuneCent(-4800.0, 4800.0);
LinearScale<double> Scales::waveShape(eps, 1.0 - eps);
DecibelScale<double> Scales::phaseMod(-140.0, 0.0, true);

DecibelScale<double> Scales::lowpassCutoffHz(0.0, 120.0, false);
DecibelScale<double> Scales::lowpassQ(-40.0, 40.0, false);
DecibelScale<double> Scales::lowpassCutoffEnvelopeAmount(-80.0, 80.0, false);

UIntScale<double> Scales::lfoTempoUpper(255);
UIntScale<double> Scales::lfoTempoLower(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);
DecibelScale<double> Scales::lfoWaveShape(0.0, 20.0, false);
DecibelScale<double> Scales::lfoToPitch(-60.0, 0.0, true);
DecibelScale<double> Scales::lfoToCutoff(-80.0, 0.0, true);

UIntScale<double> Scales::semitone(semitoneOffset + 48);
LinearScale<double> Scales::cent(-100.0, 100.0);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::a4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::noteSlideTimeSecond(-100.0, 40.0, true);

} // namespace Synth
} // namespace Steinberg
