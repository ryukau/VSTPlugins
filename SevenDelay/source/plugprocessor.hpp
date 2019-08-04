// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
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

#include "public.sdk/source/vst/vstaudioeffect.h"

#include "dsp/delay.hpp"
#include "dsp/iir.hpp"
#include "dsp/smoother.hpp"
#include "parameter.hpp"

#include <array>
#include <cmath>

namespace Steinberg {
namespace SevenDelay {

// Lagrange delay is very slow at debug build. If that's the case use linear delay.
// using DelayTypeName = Delay<float>;
using DelayTypeName = DelayLagrange<float, 7>;

using FilterTypeName = SomeDSP::SVF<float>;

class PlugProcessor : public Vst::AudioEffect {
public:
  PlugProcessor();

  tresult PLUGIN_API initialize(FUnknown *context) SMTG_OVERRIDE;
  tresult PLUGIN_API setBusArrangements(
    Vst::SpeakerArrangement *inputs,
    int32 numIns,
    Vst::SpeakerArrangement *outputs,
    int32 numOuts) SMTG_OVERRIDE;

  tresult PLUGIN_API setupProcessing(Vst::ProcessSetup &setup) SMTG_OVERRIDE;
  tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
  tresult PLUGIN_API process(Vst::ProcessData &data) SMTG_OVERRIDE;

  tresult PLUGIN_API setState(IBStream *state) SMTG_OVERRIDE;
  tresult PLUGIN_API getState(IBStream *state) SMTG_OVERRIDE;

  static FUnknown *createInstance(void *)
  {
    return (Vst::IAudioProcessor *)new PlugProcessor();
  }

  void processAudio(Vst::ProcessData &data);
  void processBypass(Vst::ProcessData &data);

protected:
  uint64_t lastState = 0;

  GlobalParameter param;

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
  std::array<std::shared_ptr<DelayTypeName>, 2> delay;
  std::array<std::shared_ptr<FilterTypeName>, 2> filter;
};

} // namespace SevenDelay
} // namespace Steinberg
