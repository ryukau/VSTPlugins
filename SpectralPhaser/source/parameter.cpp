// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LinearScale<double> Scales::bipolarScale(-1.0, 1.0);

DecibelScale<double> Scales::parameterSmoothingSecond(-120.0, 40.0, true);

DecibelScale<double> Scales::gain(-60.0, 60.0, true);

UIntScale<double> Scales::transform(static_cast<int>(SomeDSP::TransformType::LENGTH) - 1);
UIntScale<double> Scales::frameSize(maxFrameSizeLog2 - maxFrameSizeStart);
DecibelScale<double> Scales::feedback(-60.0, 0.0, true);

UIntScale<double>
  Scales::maskWaveform(static_cast<int>(SomeDSP::MaskWaveform::LENGTH) - 1);
DecibelScale<double> Scales::maskFreq(-20.0, 60.0, true);

UIntScale<double> Scales::lfoWaveform(static_cast<int>(SomeDSP::LfoWaveform::LENGTH) - 1);
UIntScale<double> Scales::lfoTempoSync(255);
DecibelScale<double> Scales::lfoRate(-60.0, 60.0, true);

} // namespace Synth
} // namespace Steinberg
