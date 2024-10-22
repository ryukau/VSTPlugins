// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::gain(0.0, 1.0, 0.5, 0.2);
LogScale<double> Scales::boost(0.0, 8.0, 0.5, 1.0);

LogScale<double> Scales::exciterGain(0.001, 2.0, 0.5, 0.5);
LogScale<double> Scales::exciterAttack(0.001, 0.2, 0.5, 0.1);
LogScale<double> Scales::exciterDecay(0.001, 0.5, 0.5, 0.1);
LogScale<double> Scales::exciterLowpassCutoff(1.0, 20000.0, 0.5, 100.0);

LinearScale<double> Scales::combTime(0.0001, 0.002);
LogScale<double> Scales::frequency(12.0, 1000.0, 0.5, 100.0);
LogScale<double> Scales::lowpassCutoff(20.0, 20000.0, 0.5, 1000.0);
LogScale<double> Scales::highpassCutoff(20.0, 20000.0, 0.5, 400.0);

LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 4.0);
LogScale<double> Scales::envelopeS(0.0, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<double> Scales::distance(0.001, 1.0, 0.5, 0.2);
LogScale<double> Scales::propagation(0.001, 1.0, 0.5, 0.7);
UIntScale<double> Scales::seed(16777215); // 2^24 - 1

LogScale<double> Scales::randomFrequency(0.0, 1.0, 0.5, 0.05);

LogScale<double> Scales::compressorTime(0.001, 8.0, 0.5, 1.0);
LogScale<double> Scales::compressorThreshold(0.01, 2.0, 0.5, 0.5);

UIntScale<double> Scales::nVoice(15);
UIntScale<double> Scales::nUnison(7);
LogScale<double> Scales::unisonDetune(0.0, 0.3, 0.2, 0.001);

UIntScale<double> Scales::octave(16);
UIntScale<double> Scales::semitone(168);
UIntScale<double> Scales::milli(2000);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::pitchA4Hz(900);

} // namespace Synth
} // namespace Steinberg
