// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LinearScale<double> Scales::masterOctave(-4.0, 4.0);
LinearScale<double> Scales::equalTemperament(1.0, 60.0);
LinearScale<double> Scales::pitchMultiply(0.0, 4.0);
LinearScale<double> Scales::pitchModulo(0.0, 60.0);

UIntScale<double> Scales::seed(16777215); // 2^24 - 1
LogScale<double> Scales::randomAmount(0.0, 1.0, 0.5, 0.1);

SPolyScale<double> Scales::chorusFrequency(-40.0, 40.0, 0.6);
LogScale<double> Scales::chorusDelayTimeRange(0.00003, 0.001, 0.5, 0.0001);
LogScale<double> Scales::chorusMinDelayTime(0, 0.01, 0.5, 0.001);
LinearScale<double> Scales::chorusPhase(0, twopi);
LinearScale<double> Scales::chorusOffset(0, twopi / 3.0);

UIntScale<double> Scales::oscSemi(120);
UIntScale<double> Scales::oscMilli(1000);
UIntScale<double> Scales::oscOctave(8);

LogScale<double> Scales::gain(0.0, 4.0, 0.5, 1.0);
LinearScale<double> Scales::gainBoost(1.0, 16.0);
DecibelScale<double> Scales::gainDecibel(-40.0, 0.0, true);

LinearScale<double> Scales::shelvingPitch(0.0, 60.0);
DecibelScale<double> Scales::shelvingGain(-30.0, 30.0, true);

LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 4.0);
LogScale<double> Scales::envelopeS(0.0, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

UIntScale<double> Scales::nVoice(5);
LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

} // namespace Synth
} // namespace Steinberg
