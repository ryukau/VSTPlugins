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

#include "plugprocessor.hpp"
#include "fuid.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

namespace Steinberg {
namespace SevenDelay {

PlugProcessor::PlugProcessor()
{
  setControllerClass(ControllerUID);
}

tresult PLUGIN_API PlugProcessor::initialize(FUnknown *context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultTrue) return result;

  addAudioInput(STR16("StereoInput"), Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("StereoOutput"), Vst::SpeakerArr::kStereo);

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
        case ParameterID::time:
          dsp.param.time = value;
          break;
        case ParameterID::feedback:
          dsp.param.feedback = value;
          break;
        case ParameterID::offset:
          dsp.param.offset = value;
          break;
        case ParameterID::wetMix:
          dsp.param.wetMix = value;
          break;
        case ParameterID::dryMix:
          dsp.param.dryMix = value;
          break;
        case ParameterID::tempoSync:
          dsp.param.tempoSync = value > 0.5;
          break;
        case ParameterID::negativeFeedback:
          dsp.param.negativeFeedback = value > 0.5;
          break;
        case ParameterID::lfoAmount:
          dsp.param.lfoAmount = value;
          break;
        case ParameterID::lfoFrequency:
          dsp.param.lfoFrequency = value;
          break;
        case ParameterID::lfoShape:
          dsp.param.lfoShape = value;
          break;
        case ParameterID::lfoInitialPhase:
          dsp.param.lfoInitialPhase = value;
          break;
        case ParameterID::lfoHold:
          dsp.param.lfoHold = value > 0.5;
          break;
        case ParameterID::smoothness:
          dsp.param.smoothness = value;
          break;
        case ParameterID::inSpread:
          dsp.param.inSpread = value;
          break;
        case ParameterID::inPan:
          dsp.param.inPan = value;
          break;
        case ParameterID::outSpread:
          dsp.param.outSpread = value;
          break;
        case ParameterID::outPan:
          dsp.param.outPan = value;
          break;
        case ParameterID::tone:
          dsp.param.tone = value;
          break;

          // Add parameter here.
      }
    }
  }

  if (data.processContext == nullptr) return kResultOk;

  uint64_t state = data.processContext->state;
  if (
    (lastState & Vst::ProcessContext::kPlaying) == 0
    && (state & Vst::ProcessContext::kPlaying) != 0) {
    dsp.startup();
  }
  lastState = state;

  dsp.setParameters(data.processContext->tempo);

  if (data.numInputs == 0) return kResultOk;
  if (data.numOutputs == 0) return kResultOk;
  if (data.numSamples <= 0) return kResultOk;
  if (data.inputs[0].numChannels != 2) return kResultOk;
  if (data.outputs[0].numChannels != 2) return kResultOk;
  if (data.symbolicSampleSize == Vst::kSample64) return kResultOk;

  if (dsp.param.bypass) {
    processBypass(data);
  } else {
    float *in0 = data.inputs[0].channelBuffers32[0];
    float *in1 = data.inputs[0].channelBuffers32[1];
    float *out0 = data.outputs[0].channelBuffers32[0];
    float *out1 = data.outputs[0].channelBuffers32[1];
    dsp.process((size_t)data.numSamples, in0, in1, out0, out1);
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

} // namespace SevenDelay
} // namespace Steinberg
