// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);
UIntScale<double> Scales::seed(1 << 23);

DecibelScale<double> Scales::gain(-60.0, 60.0, true);
DecibelScale<double> Scales::gateAttackSecond(-80.0, 40, true);
DecibelScale<double> Scales::gateReleaseSecond(-80.0, 40, false);

UIntScale<double> Scales::octave(16);
UIntScale<double> Scales::semitone(168);
UIntScale<double> Scales::milli(2000);
UIntScale<double> Scales::equalTemperament(119);
UIntScale<double> Scales::pitchA4Hz(900);
LinearScale<double> Scales::pitchBendRange(0.0, 120.0);

DecibelScale<double> Scales::smoothingTimeSecond(-120.0, 40.0, true);
UIntScale<double> Scales::slideType(2);

DecibelScale<double> Scales::oscAttack(-80.0, 40, false);
DecibelScale<double> Scales::oscDecay(-80.0, 40, false);
DecibelScale<double> Scales::oscDensityHz(-20.0, 100, false);
LinearScale<double> Scales::oscDensityKeyFollow(0.0, 2.0);
DecibelScale<double> Scales::oscBounceCurve(-20.0, 20.0, false);
DecibelScale<double> Scales::oscLowpassQ(-40, 60.0, false);

DecibelScale<double> Scales::impulseGain(-100.0, 0.0, true);

DecibelScale<double> Scales::fdnMatrixIdentityAmount(-60.0, 60.0, false);
NegativeDecibelScale<double> Scales::fdnFeedback(-100.0, 0.0, 1.0, true);
DecibelScale<double> Scales::fdnOvertoneAdd(-60.0, 40.0, true);
DecibelScale<double> Scales::fdnOvertoneMul(-60.0, 40.0, true);
LinearScale<double> Scales::fdnOvertoneOffset(0.0, 64.0);
DecibelScale<double> Scales::fdnOvertoneModulo(-60.0, 60.0, true);
DecibelScale<double> Scales::fdnInterpRate(-40.0, 40.0, false);
DecibelScale<double> Scales::fdnInterpLowpassSecond(-120.0, 40.0, true);

LinearScale<double> Scales::filterCutoffSemi(-120.0, 200.0);
LinearScale<double> Scales::filterCutoffSlope(-12.0, 12.0);
LinearScale<double> Scales::filterQ(0.01, halfSqrt2);
LinearScale<double>
  Scales::filterQSlope(-halfSqrt2 / fdnMatrixSize, halfSqrt2 / fdnMatrixSize);
LinearScale<double> Scales::filterCutoffSemiOffset(-120.0, 120.0);
LinearScale<double> Scales::filterQOffset(-halfSqrt2, halfSqrt2);

UIntScale<double> Scales::wavetableInterpolation(2);

LinearScale<double> Scales::lfoToPitchAmount(-120.0, 120.0);
DecibelScale<double> Scales::modEnvelopeTime(-80.0, 40.0, false);

DecibelScale<double> Scales::tremoloDepth(-80.0, 0.0, true);
DecibelScale<double> Scales::tremoloDelayTime(-100.0, -20.0, false);
DecibelScale<double> Scales::tremoloModulationRateHz(-40.0, 40.0, false);

} // namespace Synth
} // namespace Steinberg
