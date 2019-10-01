// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "plugprocessor.hpp"
#include "fuid.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace Steinberg {
namespace Synth {

PlugProcessor::PlugProcessor() { setControllerClass(ControllerUID); }

tresult PLUGIN_API PlugProcessor::initialize(FUnknown *context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultTrue) return result;

  addAudioOutput(STR16("StereoOutput"), Vst::SpeakerArr::kStereo);
  addEventInput(STR16("EventInput"), 1);

  return result;
}

tresult PLUGIN_API PlugProcessor::setBusArrangements(Vst::SpeakerArrangement *inputs,
  int32 numIns,
  Vst::SpeakerArrangement *outputs,
  int32 numOuts)
{
  if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0]) {
    return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
  }
  return kResultFalse;
}

tresult PLUGIN_API PlugProcessor::setupProcessing(Vst::ProcessSetup &setup)
{
  return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API PlugProcessor::setActive(TBool state)
{
  if (state) {
    dsp.setup(processSetup.sampleRate);
  } else {
    dsp.free();
    lastState = 0;
  }
  return AudioEffect::setActive(state);
}

tresult PLUGIN_API PlugProcessor::process(Vst::ProcessData &data)
{
  // Read inputs parameter changes.
  if (data.inputParameterChanges) {
    int32 parameterCount = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < parameterCount; index++) {
      auto queue = data.inputParameterChanges->getParameterData(index);
      if (!queue) continue;
      Vst::ParamValue value;
      int32 sampleOffset;
      if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) != kResultTrue)
        continue;
      switch (queue->getParameterId()) {
        case ParameterID::bypass:
          dsp.param.bypass = value > 0.5f;
          break;

        case ParameterID::osc1Gain:
          dsp.param.osc1Gain = value;
          break;
        case ParameterID::osc1Semi:
          dsp.param.osc1Semi = value;
          break;
        case ParameterID::osc1Cent:
          dsp.param.osc1Cent = value;
          break;
        case ParameterID::osc1Sync:
          dsp.param.osc1Sync = value;
          break;
        case ParameterID::osc1SyncType:
          dsp.param.osc1SyncType = toDiscrete(value, 3.0);
          break;
        case ParameterID::osc1PTROrder:
          dsp.param.osc1PTROrder = toDiscrete(value, 11.0);
          break;
        case ParameterID::osc1Phase:
          dsp.param.osc1Phase = value;
          break;
        case ParameterID::osc1PhaseLock:
          dsp.param.osc1PhaseLock = value > 0.5f;
          break;

        case ParameterID::osc2Gain:
          dsp.param.osc2Gain = value;
          break;
        case ParameterID::osc2Semi:
          dsp.param.osc2Semi = value;
          break;
        case ParameterID::osc2Cent:
          dsp.param.osc2Cent = value;
          break;
        case ParameterID::osc2Sync:
          dsp.param.osc2Sync = value;
          break;
        case ParameterID::osc2SyncType:
          dsp.param.osc2SyncType = toDiscrete(value, 3.0);
          break;
        case ParameterID::osc2PTROrder:
          dsp.param.osc2PTROrder = toDiscrete(value, 11.0);
          break;
        case ParameterID::osc2Phase:
          dsp.param.osc2Phase = value;
          break;
        case ParameterID::osc2PhaseLock:
          dsp.param.osc2PhaseLock = value > 0.5f;
          break;
        case ParameterID::osc2Invert:
          dsp.param.osc2Invert = value > 0.5f;
          break;

        case ParameterID::fmOsc1ToSync1:
          dsp.param.fmOsc1ToSync1 = value;
          break;
        case ParameterID::fmOsc1ToFreq2:
          dsp.param.fmOsc1ToFreq2 = value;
          break;
        case ParameterID::fmOsc2ToSync1:
          dsp.param.fmOsc2ToSync1 = value;
          break;

        case ParameterID::gain:
          dsp.param.gain = value;
          break;
        case ParameterID::gainA:
          dsp.param.gainA = value;
          break;
        case ParameterID::gainD:
          dsp.param.gainD = value;
          break;
        case ParameterID::gainS:
          dsp.param.gainS = value;
          break;
        case ParameterID::gainR:
          dsp.param.gainR = value;
          break;
        case ParameterID::gainEnvelopeCurve:
          dsp.param.gainEnvelopeCurve = value;
          break;

        case ParameterID::filterCutoff:
          dsp.param.filterCutoff = value;
          break;
        case ParameterID::filterResonance:
          dsp.param.filterResonance = value;
          break;
        case ParameterID::filterFeedback:
          dsp.param.filterFeedback = value;
          break;
        case ParameterID::filterSaturation:
          dsp.param.filterSaturation = value;
          break;
        case ParameterID::filterDirty:
          dsp.param.filterDirty = value > 0.5f;
          break;
        case ParameterID::filterType:
          dsp.param.filterType = toDiscrete(value, 4.0);
          break;
        case ParameterID::filterShaper:
          dsp.param.filterShaper = toDiscrete(value, 3.0);
          break;
        case ParameterID::filterA:
          dsp.param.filterA = value;
          break;
        case ParameterID::filterD:
          dsp.param.filterD = value;
          break;
        case ParameterID::filterS:
          dsp.param.filterS = value;
          break;
        case ParameterID::filterR:
          dsp.param.filterR = value;
          break;
        case ParameterID::filterCutoffAmount:
          dsp.param.filterCutoffAmount = value;
          break;
        case ParameterID::filterResonanceAmount:
          dsp.param.filterResonanceAmount = value;
          break;
        case ParameterID::filterKeyToCutoff:
          dsp.param.filterKeyToCutoff = value;
          break;
        case ParameterID::filterKeyToFeedback:
          dsp.param.filterKeyToFeedback = value;
          break;

        case ParameterID::modEnvelopeA:
          dsp.param.modEnvelopeA = value;
          break;
        case ParameterID::modEnvelopeCurve:
          dsp.param.modEnvelopeCurve = value;
          break;
        case ParameterID::modEnvelopeToFreq1:
          dsp.param.modEnvelopeToFreq1 = value;
          break;
        case ParameterID::modEnvelopeToSync1:
          dsp.param.modEnvelopeToSync1 = value;
          break;
        case ParameterID::modEnvelopeToFreq2:
          dsp.param.modEnvelopeToFreq2 = value;
          break;
        case ParameterID::modEnvelopeToSync2:
          dsp.param.modEnvelopeToSync2 = value;
          break;
        case ParameterID::modLFOFrequency:
          dsp.param.modLFOFrequency = value;
          break;
        case ParameterID::modLFONoiseMix:
          dsp.param.modLFONoiseMix = value;
          break;
        case ParameterID::modLFOToFreq1:
          dsp.param.modLFOToFreq1 = value;
          break;
        case ParameterID::modLFOToSync1:
          dsp.param.modLFOToSync1 = value;
          break;
        case ParameterID::modLFOToFreq2:
          dsp.param.modLFOToFreq2 = value;
          break;
        case ParameterID::modLFOToSync2:
          dsp.param.modLFOToSync2 = value;
          break;

          // Add parameter here.
      }
    }
  }

  if (data.processContext == nullptr) return kResultOk;

  uint64_t state = data.processContext->state;
  if ((lastState & Vst::ProcessContext::kPlaying) == 0
    && (state & Vst::ProcessContext::kPlaying) != 0) {
    dsp.startup();
  }
  lastState = state;

  dsp.setParameters(data.processContext->tempo);

  if (data.numOutputs == 0) return kResultOk;
  if (data.numSamples <= 0) return kResultOk;
  if (data.outputs[0].numChannels != 2) return kResultOk;
  if (data.symbolicSampleSize == Vst::kSample64) return kResultOk;

  if (data.inputEvents == nullptr) return kResultOk;
  for (int32 index = 0; index < data.inputEvents->getEventCount(); ++index) {
    Vst::Event event;
    if (data.inputEvents->getEvent(index, event) != kResultOk) continue;
    switch (event.type) {
      case Vst::Event::kNoteOnEvent:
        dsp.noteOn(event.noteOn.noteId, event.noteOn.pitch, event.noteOn.tuning,
          event.noteOn.velocity);
        break;

      case Vst::Event::kNoteOffEvent:
        dsp.noteOff(event.noteOff.noteId, event.noteOff.pitch);
        break;

        // Add other event type here.
    }
  }

  if (dsp.param.bypass) {
    processBypass(data);
  } else {
    float *out0 = data.outputs[0].channelBuffers32[0];
    float *out1 = data.outputs[0].channelBuffers32[1];
    dsp.process((size_t)data.numSamples, out0, out1);
  }

  return kResultOk;
}

void PlugProcessor::processBypass(Vst::ProcessData &data)
{
  float **in = data.inputs[0].channelBuffers32;
  float **out = data.outputs[0].channelBuffers32;
  for (int32_t ch = 0; ch < data.inputs[0].numChannels; ch++) {
    if (in[ch] != out[ch]) memcpy(out[ch], in[ch], data.numSamples * sizeof(float));
  }
}

tresult PLUGIN_API PlugProcessor::setState(IBStream *state)
{
  if (!state) return kResultFalse;
  return dsp.param.setState(state);
}

tresult PLUGIN_API PlugProcessor::getState(IBStream *state)
{
  return dsp.param.getState(state);
}

} // namespace Synth
} // namespace Steinberg
