// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LogScale<double> Scales::delayTime(0.0, 1.0, 0.5, 0.05);
SemitoneScale<double> Scales::lowpassCutoffHz(33.0, 136.0, false);
SemitoneScale<double> Scales::highpassCutoffHz(-37.0, 57.0, true);
UIntScale<double>
  Scales::matrixType(FeedbackMatrixType::FeedbackMatrixType_ENUM_LENGTH - 1);
DecibelScale<double> Scales::gateThreshold(-140.0, 0.0, true);
DecibelScale<double> Scales::dry(-60.0, 24.0, true);
DecibelScale<double> Scales::wet(-60.0, 60.0, true);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);
LogScale<double> Scales::delayTimeInterpRate(0.0, 4.0, 0.5, 0.25);
UIntScale<double> Scales::seed(1 << 23);
LogScale<double> Scales::splitRotationHz(0.0, 10.0, 0.5, 0.2);
LinearScale<double> Scales::splitSkew(0.0, 6.0);

} // namespace Synth
} // namespace Steinberg
