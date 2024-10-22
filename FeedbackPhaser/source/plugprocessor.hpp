// SPDX-License-Identifier: GPL-3.0-only
//
// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

#include "dsp/dspcore.hpp"

namespace Steinberg {
namespace Synth {

class PlugProcessor : public Vst::AudioEffect {
public:
  PlugProcessor();

  tresult PLUGIN_API initialize(FUnknown *context) SMTG_OVERRIDE;
  tresult PLUGIN_API setBusArrangements(
    Vst::SpeakerArrangement *inputs,
    int32 numIns,
    Vst::SpeakerArrangement *outputs,
    int32 numOuts) SMTG_OVERRIDE;
  uint32 PLUGIN_API getProcessContextRequirements() SMTG_OVERRIDE;

  tresult PLUGIN_API setupProcessing(Vst::ProcessSetup &setup) SMTG_OVERRIDE;
  tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
  tresult PLUGIN_API process(Vst::ProcessData &data) SMTG_OVERRIDE;

  tresult PLUGIN_API setState(IBStream *state) SMTG_OVERRIDE;
  tresult PLUGIN_API getState(IBStream *state) SMTG_OVERRIDE;

  static FUnknown *createInstance(void *)
  {
    return (Vst::IAudioProcessor *)new PlugProcessor();
  }

  void processBypass(Vst::ProcessData &data);

protected:
  void handleEvent(Vst::ProcessData &data);

  inline int32 toDiscrete(Vst::ParamValue normalized, int32 stepCount)
  {
    return int32(std::min<double>(stepCount, normalized * (stepCount + 1.0)));
  }

  DSPCore dsp;
};

} // namespace Synth
} // namespace Steinberg
