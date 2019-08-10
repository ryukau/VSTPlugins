// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include "../parameter.hpp"
#include "delay.hpp"
#include "iir.hpp"
#include "smoother.hpp"

#include <array>

namespace Steinberg {
namespace SevenDelay {

// Lagrange delay is very slow at debug build. If that's the case use Delay instead of
// DelayLagrange.
using DelayTypeName = DelayLagrange<float, 7>;
using FilterTypeName = SomeDSP::SVF<float>;

class DSPCore {
public:
  GlobalParameter param;

  void setup(double sampleRate);

  void reset();

  // Reset phase, random seed etc.
  void startup();

  // tempo is beat per minutes.
  void setParameters(double tempo);

  void process(const size_t length, float *in0, float *in1, float *out0, float *out1);

protected:
  const float pi = 3.14159265358979323846;

  std::array<LinearSmoother<float>, 2> interpTime{};
  LinearSmoother<float> interpWetMix;
  LinearSmoother<float> interpDryMix;
  LinearSmoother<float> interpFeedback;
  LinearSmoother<float> interpLfoAmount;
  LinearSmoother<float> interpLfoFrequency;
  LinearSmoother<float> interpLfoShape;
  std::array<LinearSmoother<float>, 2> interpPanIn{};
  std::array<LinearSmoother<float>, 2> interpPanOut{};
  LinearSmoother<float> interpTone;
  LinearSmoother<float> interpToneMix;

  double lfoPhase;
  double lfoPhaseTick;
  std::array<float, 2> delayOut{};
  std::array<std::unique_ptr<DelayTypeName>, 2> delay;
  std::array<std::unique_ptr<FilterTypeName>, 2> filter;
};

} // namespace SevenDelay
} // namespace Steinberg
