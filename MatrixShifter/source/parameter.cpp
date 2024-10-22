// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LinearScale<double> Scales::shiftSemi(0.0, 10.0);
LogScale<double>
  Scales::shiftDelay(0, maxShiftDelaySeconds, 0.5, 0.2 * maxShiftDelaySeconds);
DecibelScale<double> Scales::shiftGain(-30, 0, true);
DecibelScale<double> Scales::shiftFeedbackGain(-18.0, 0.0, true);
SemitoneScale<double>
  Scales::shiftFeedbackCutoff(minFeedbackCutoffNote, maxFeedbackCutoffNote, false);
LogScale<double> Scales::shiftSemiMultiplier(0.0, 8.0, 0.5, 0.2);

SemitoneScale<double> Scales::lfoRate(freqToNote(0.01), freqToNote(20.0), true);
LinearScale<double> Scales::lfoLrPhaseOffset(-0.5, 0.5);
UIntScale<double> Scales::lfoSync(255);
LinearScale<double> Scales::lfoToPitchShift(-1.0, 1.0);
LinearScale<double> Scales::lfoToFeedbackCutoff(-1.0, 1.0);

DecibelScale<double> Scales::gain(-24, 24, true);

LogScale<double> Scales::smoothness(0.04, 8.0, 0.5, 1.0);

} // namespace Synth
} // namespace Steinberg
