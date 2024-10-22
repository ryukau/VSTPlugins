// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::envelopeA(0.0, 16.0, 0.5, 0.5);
LogScale<double> Scales::envelopeD(0.0, 4.0, 0.5, 0.5);
DecibelScale<double> Scales::gainDecibel(-40.0, 0.0, true);
LogScale<double> Scales::saturation(0.5, 5.0, 0.5, 2.0);

LogScale<double> Scales::gain(0.0, 32.0, 0.5, 8.0);
LinearScale<double> Scales::gainBoost(1.0, 8.0);

LinearScale<double> Scales::masterOctave(-4.0, 4.0);
LinearScale<double> Scales::equalTemperament(1.0, 60.0);
LinearScale<double> Scales::pitchMultiply(0.001, 4.0);
LinearScale<double> Scales::pitchModulo(0.0, 60.0);

UIntScale<double> Scales::seed(16777215); // 2^24 - 1
LinearScale<double> Scales::randomGain(0.0, 4.0);
LogScale<double> Scales::randomFrequency(0.0, 1.0, 0.5, 0.1);
LinearScale<double> Scales::randomAttack(0.0, 1.0);
LinearScale<double> Scales::randomDecay(0.0, 1.0);
LinearScale<double> Scales::randomSaturation(0.0, 1.0);
LinearScale<double> Scales::randomPhase(0.0, 1.0);

LogScale<double> Scales::overtoneExpand(0.01, 4.0, 0.4, 1.0);
LinearScale<double> Scales::overtoneShift(0.0, 64.0);

LogScale<double> Scales::envelopeMultiplier(0.001, 4.0, 0.4, 1.0);
LinearScale<double> Scales::gainPower(0.001, 16.0);

LogScale<double> Scales::phaserFrequency(0.000, 16.0, 0.5, 2.0);
LinearScale<double> Scales::phaserFeedback(-1.0, 1.0);
LogScale<double> Scales::phaserRange(0.0, 128.0, 0.5, 32.0);
LinearScale<double> Scales::phaserPhase(0.0, twopi);
UIntScale<double> Scales::phaserStage(15);

UIntScale<double> Scales::nVoice(5);
LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

} // namespace Synth
} // namespace Steinberg
