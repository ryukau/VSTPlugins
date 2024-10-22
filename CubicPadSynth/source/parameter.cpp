// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

constexpr double pitchRange = 128.0;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

DecibelScale<double> Scales::overtoneGain(-60.0, 0.0, true);
LinearScale<double> Scales::overtoneWidth(0.0, 100.0);
LogScale<double> Scales::overtonePitch(0.0625, 16.0, 0.5, 1.0);
LinearScale<double> Scales::overtonePhase(0.0, twopi);

LogScale<double> Scales::tableBaseFrequency(0.1, 100.0, 0.5, 10.0);
LogScale<double> Scales::overtoneGainPower(0.2, 10.0, 0.5, 1.0);
LogScale<double> Scales::overtoneWidthMultiply(0.05, 12.0, 0.5, 1.0);
LinearScale<double> Scales::overtonePitchMultiply(0.0001, 16.0);
LinearScale<double> Scales::overtonePitchModulo(0.0, 136.0);
LogScale<double> Scales::spectrumExpand(0.03125, 32.0, 0.5, 1.0);
UIntScale<double> Scales::spectrumShift(2 * spectrumSize);
UIntScale<double> Scales::profileComb(255);
LogScale<double> Scales::profileShape(0.0, 4.0, 0.5, 1.0);
UIntScale<double> Scales::seed(16777215); // 2^24 - 1

LogScale<double> Scales::gain(0.0, 2.0, 0.5, 0.5);
LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 2.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 4.0);
LogScale<double> Scales::envelopeS(0.0001, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

UIntScale<double> Scales::oscOctave(16);
UIntScale<double> Scales::oscSemi(168);
UIntScale<double> Scales::oscMilli(2000);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::pitchA4Hz(900);

LogScale<double> Scales::pitchAmount(0.0, pitchRange, 0.5, 24.0);

LinearScale<double> Scales::lfoWavetable(-1.0, 1.0);
UIntScale<double> Scales::lfoWavetableType(2);
UIntScale<double> Scales::lfoTempoNumerator(255);
UIntScale<double> Scales::lfoTempoDenominator(255);
LogScale<double> Scales::lfoFrequencyMultiplier(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::lfoLowpass(0.0, 1.0, 0.5, 0.01);

LinearScale<double> Scales::tableLowpass(0.0, 1.0);
LinearScale<double> Scales::tableLowpassAmount(0.0, pitchRange);

UIntScale<double> Scales::nUnison(15);
LogScale<double> Scales::unisonDetune(0.0, 1.0, 0.5, 0.05);
UIntScale<double> Scales::unisonPanType(9);

UIntScale<double> Scales::nVoice(7);
LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

} // namespace Synth
} // namespace Steinberg
