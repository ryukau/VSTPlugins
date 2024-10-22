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

DecibelScale<double> Scales::decayTargetGain(-150.0, 0.0, false);
DecibelScale<double> Scales::fmIndex(-60.0, 40.0, true);
UIntScale<double> Scales::pulseWidthModRate(511);
LinearScale<double> Scales::randomizeFmIndex(0.0, 4.0);

LinearScale<double> Scales::filterDecayRatio(-10.0, 10.0);
LinearScale<double> Scales::filterCutoffBaseOctave(-8.0, 16.0);
LinearScale<double> Scales::filterCutoffModOctave(-16, 16);
LinearScale<double> Scales::filterNotchBaseOctave(-4, 4);
LinearScale<double> Scales::filterNotchModOctave(-4, 4);

LinearScale<double> Scales::polynomialPointY(-0.5, 0.5);

UIntScale<double> Scales::transposeOctave(2 * transposeOctaveOffset);
UIntScale<double> Scales::transposeSemitone(2 * transposeSemitoneOffset);
LinearScale<double> Scales::transposeCent(-3600, 3600);
UIntScale<double> Scales::tuningType(63);
UIntScale<double> Scales::tuningRootSemitone(63);

UIntScale<double> Scales::arpeggioNotesPerBeat(15);
UIntScale<double> Scales::arpeggioLoopLengthInBeat(256);
UIntScale<double> Scales::arpeggioDurationVariation(15);
UIntScale<double> Scales::arpeggioScale(255);
LinearScale<double> Scales::arpeggioPicthDriftCent(0, 200);
UIntScale<double> Scales::arpeggioOctave(7);

UIntScale<double> Scales::unisonVoice(255);
LinearScale<double> Scales::unisonDetuneCent(0, 1200);

} // namespace Synth
} // namespace Steinberg
