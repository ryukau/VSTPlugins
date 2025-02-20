// SPDX-License-Identifier: GPL-3.0-only
//
// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// Takamitsu Endo (ryukau@gmail.com)

#include "plugprocessor.hpp"
#include "fuid.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#ifdef USE_VECTORCLASS
  #include "../../lib/vcl/vectorclass.h"
#endif

#include <iostream>

namespace Steinberg {
namespace Synth {

PlugProcessor::PlugProcessor() { setControllerClass(ControllerUID); }

tresult PLUGIN_API PlugProcessor::initialize(FUnknown *context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultTrue) return result;

  addAudioInput(STR16("StereoInput"), Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("StereoOutput"), Vst::SpeakerArr::kStereo);
  addEventInput(STR16("EventInput"), 1);

  return result;
}

tresult PLUGIN_API PlugProcessor::setBusArrangements(
  Vst::SpeakerArrangement *inputs,
  int32 numIns,
  Vst::SpeakerArrangement *outputs,
  int32 numOuts)
{
  if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0]) {
    return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
  }
  return kResultFalse;
}

uint32 PLUGIN_API PlugProcessor::getProcessContextRequirements()
{
  using Rq = Vst::IProcessContextRequirements;

  return Rq::kNeedProjectTimeMusic & Rq::kNeedTempo & Rq::kNeedTransportState
    & Rq::kNeedTimeSignature;
}

tresult PLUGIN_API PlugProcessor::setupProcessing(Vst::ProcessSetup &setup)
{
  dsp.setup(processSetup.sampleRate);
  return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API PlugProcessor::setActive(TBool state)
{
  if (state) {
    dsp.setup(processSetup.sampleRate);
  } else {
    dsp.reset();
    lastState = 0;
  }
  return AudioEffect::setActive(state);
}

tresult PLUGIN_API PlugProcessor::process(Vst::ProcessData &data)
{
  using ID = ParameterID::ID;

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
      size_t id = queue->getParameterId();
      if (id < dsp.param.value.size()) dsp.param.value[id]->setFromNormalized(value);
    }
  }

  if (data.processContext != nullptr) {
    uint64_t state = data.processContext->state;
    if (state & Vst::ProcessContext::kTempoValid) {
      dsp.tempo = data.processContext->tempo;
    }
    if (state & Vst::ProcessContext::kProjectTimeMusicValid) {
      dsp.beatsElapsed = data.processContext->projectTimeMusic;
    }
    if (state & Vst::ProcessContext::kTimeSigValid) {
      dsp.timeSigLower = data.processContext->timeSigDenominator;
      dsp.timeSigUpper = data.processContext->timeSigNumerator;
    }
    if (!dsp.isPlaying && (state & Vst::ProcessContext::kPlaying) != 0) {
      dsp.startup();
    }
    dsp.isPlaying = state & Vst::ProcessContext::kPlaying;
  }

  dsp.setParameters();

  if (data.numInputs == 0) return kResultOk;
  if (data.numOutputs == 0) return kResultOk;
  if (data.numSamples <= 0) return kResultOk;
  if (data.inputs[0].numChannels < 2) return kResultOk;
  if (data.outputs[0].numChannels < 2) return kResultOk;
  if (data.symbolicSampleSize == Vst::kSample64) return kResultOk;

  if (data.inputEvents != nullptr) handleEvent(data);

  const float *in0 = data.inputs[0].channelBuffers32[0];
  const float *in1 = data.inputs[0].channelBuffers32[1];
  float *out0 = data.outputs[0].channelBuffers32[0];
  float *out1 = data.outputs[0].channelBuffers32[1];
  dsp.process((size_t)data.numSamples, in0, in1, out0, out1);

  // // Send parameter changes for GUI.
  // if (!data.outputParameterChanges) return kResultOk;
  // int32 index = 0;
  // for (uint32 id = ID::ID_ENUM_GUI_START; id < ID::ID_ENUM_LENGTH; ++id) {
  //   auto queue = data.outputParameterChanges->addParameterData(id, index);
  //   if (!queue) continue;
  //   queue->addPoint(0, dsp.param.value[id]->getNormalized(), index);
  // }

  return kResultOk;
}

void PlugProcessor::handleEvent(Vst::ProcessData &data)
{
  for (int32 index = 0; index < data.inputEvents->getEventCount(); ++index) {
    Vst::Event event;
    if (data.inputEvents->getEvent(index, event) != kResultOk) continue;
    switch (event.type) {
      case Vst::Event::kNoteOnEvent: {
        dsp.pushMidiNote(
          true, event.sampleOffset,
          event.noteOn.noteId == -1 ? event.noteOn.pitch : event.noteOn.noteId,
          event.noteOn.pitch, event.noteOn.tuning, event.noteOn.velocity);
      } break;

      case Vst::Event::kNoteOffEvent: {
        dsp.pushMidiNote(
          false, event.sampleOffset,
          event.noteOff.noteId == -1 ? event.noteOff.pitch : event.noteOff.noteId, 0, 0,
          event.noteOff.velocity);
      } break;

        // Add other event type here.
    }
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
