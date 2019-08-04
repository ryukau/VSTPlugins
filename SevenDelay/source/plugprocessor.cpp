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

#include <cmath>
#define M_PI 3.14159265358979323846

namespace Steinberg {
namespace SevenDelay {

PlugProcessor::PlugProcessor()
{
  setControllerClass(ControllerUID);
}

tresult PLUGIN_API PlugProcessor::initialize(FUnknown *context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultTrue) return kResultFalse;

  addAudioInput(STR16("StereoInput"), Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("StereoOutput"), Vst::SpeakerArr::kStereo);

  return kResultTrue;
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
  LinearSmoother<float>::setSampleRate(setup.sampleRate);

  delay[0] = std::make_shared<DelayTypeName>(setup.sampleRate, 1.0f, maxDelayTime);
  delay[1] = std::make_shared<DelayTypeName>(setup.sampleRate, 1.0f, maxDelayTime);
  filter[0] = std::make_shared<FilterTypeName>(
    setup.sampleRate, SomeDSP::SVFType::allpass, maxToneFrequency, 0.9, -3.0);
  filter[1] = std::make_shared<FilterTypeName>(
    setup.sampleRate, SomeDSP::SVFType::allpass, maxToneFrequency, 0.9, -3.0);

  delayOut[0] = 0.0f;
  delayOut[1] = 0.0f;
  lfoPhase = param.lfoInitialPhase;
  lfoPhaseTick = 2.0 * M_PI / setup.sampleRate;

  return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API PlugProcessor::setActive(TBool state)
{
  if (state) {
  } else {
    delay[0]->reset();
    delay[1]->reset();
    filter[0]->reset();
    filter[1]->reset();
    delayOut[0] = 0.0f;
    delayOut[1] = 0.0f;
    lfoPhase = param.lfoInitialPhase;
  }
  return AudioEffect::setActive(state);
}

float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
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
          param.bypass = value > 0.5f;
          break;
        case ParameterID::time:
          param.time = value;
          break;
        case ParameterID::feedback:
          param.feedback = value;
          break;
        case ParameterID::offset:
          param.offset = value;
          break;
        case ParameterID::wetMix:
          param.wetMix = value;
          break;
        case ParameterID::dryMix:
          param.dryMix = value;
          break;
        case ParameterID::tempoSync:
          param.tempoSync = value > 0.5;
          break;
        case ParameterID::negativeFeedback:
          param.negativeFeedback = value > 0.5;
          break;
        case ParameterID::lfoAmount:
          param.lfoAmount = value;
          break;
        case ParameterID::lfoFrequency:
          param.lfoFrequency = value;
          break;
        case ParameterID::lfoShape:
          param.lfoShape = value;
          break;
        case ParameterID::lfoInitialPhase:
          param.lfoInitialPhase = value;
          break;
        case ParameterID::lfoHold:
          param.lfoHold = value > 0.5;
          break;
        case ParameterID::smoothness:
          param.smoothness = value;
          break;
        case ParameterID::inSpread:
          param.inSpread = value;
          break;
        case ParameterID::inPan:
          param.inPan = value;
          break;
        case ParameterID::outSpread:
          param.outSpread = value;
          break;
        case ParameterID::outPan:
          param.outPan = value;
          break;
        case ParameterID::tone:
          param.tone = value;
          break;

          // Add parameter here.
      }
    }
  }

  if (data.processContext == nullptr) return kResultOk;

  auto smoothness = GlobalParameter::scaleSmoothness.map(param.smoothness);
  LinearSmoother<float>::setTime(smoothness);

  uint64_t state = data.processContext->state;
  if (
    (lastState & Vst::ProcessContext::kPlaying) == 0
    && (state & Vst::ProcessContext::kPlaying) != 0) {
    // Reset phase and random seed here.
    lfoPhase = param.lfoInitialPhase;
  }
  lastState = state;

  // This won't work if sync is on and tempo < 15. Up to 8 sec or 8/16 beat.
  // 15.0 is come from (60 sec per minute) * (4 beat) / (16 beat).
  Vst::ParamValue time = GlobalParameter::scaleTime.map(param.time);
  if (param.tempoSync) {
    if (time < 1.0) {
      time *= 15.0 / data.processContext->tempo;
    } else {
      time = floor(2.0 * time) * 7.5 / data.processContext->tempo;
    }
  }

  Vst::ParamValue offset = GlobalParameter::scaleOffset.map(param.offset);
  if (offset < 0.0) {
    interpTime[0].push(time * (1.0 + offset));
    interpTime[1].push(time);
  } else if (offset > 0.0) {
    interpTime[0].push(time);
    interpTime[1].push(time * (1.0 - offset));
  } else {
    interpTime[0].push(time);
    interpTime[1].push(time);
  }

  interpWetMix.push(param.wetMix);
  interpDryMix.push(param.dryMix);
  interpFeedback.push(param.negativeFeedback ? -param.feedback : param.feedback);
  interpLfoAmount.push(GlobalParameter::scaleLfoAmount.map(param.lfoAmount));
  interpLfoFrequency.push(GlobalParameter::scaleLfoFrequency.map(param.lfoFrequency));
  interpLfoShape.push(GlobalParameter::scaleLfoShape.map(param.lfoShape));

  float inPan = 2 * param.inPan;
  float panInL = clamp(inPan + param.inSpread - 1.0, 0.0, 1.0);
  float panInR = clamp(inPan - param.inSpread, 0.0, 1.0);
  interpPanIn[0].push(panInL);
  interpPanIn[1].push(panInR);

  float outPan = 2 * param.outPan;
  float panOutL = clamp(outPan + param.outSpread - 1.0, 0.0, 1.0);
  float panOutR = clamp(outPan - param.outSpread, 0.0, 1.0);
  interpPanOut[0].push(panOutL);
  interpPanOut[1].push(panOutR);

  interpTone.push(GlobalParameter::scaleTone.map(param.tone));
  interpToneMix.push(GlobalParameter::scaleToneMix.map(param.tone));

  if (data.numInputs == 0) return kResultOk;
  if (data.numOutputs == 0) return kResultOk;
  if (data.numSamples <= 0) return kResultOk;
  if (data.inputs[0].numChannels != 2) return kResultOk;
  if (data.outputs[0].numChannels != 2) return kResultOk;
  if (data.symbolicSampleSize == Vst::kSample64) return kResultOk;

  if (param.bypass)
    processBypass(data);
  else
    processAudio(data);

  return kResultOk;
}

void PlugProcessor::processAudio(Vst::ProcessData &data)
{
  float *in0 = data.inputs[0].channelBuffers32[0];
  float *in1 = data.inputs[0].channelBuffers32[1];
  float *out0 = data.outputs[0].channelBuffers32[0];
  float *out1 = data.outputs[0].channelBuffers32[1];

  for (int32_t i = 0; i < data.numSamples; ++i) {
    auto sign = (M_PI < lfoPhase) - (lfoPhase < M_PI);
    float lfo = sign * powf(abs(sin(lfoPhase)), interpLfoShape.process());
    lfo = interpLfoAmount.process() * (lfo + 1.0f);

    delay[0]->setTime(interpTime[0].process() + lfo);
    delay[1]->setTime(interpTime[1].process() + lfo);

    const float feedback = interpFeedback.process();
    const float inL = in0[i] + feedback * delayOut[0];
    const float inR = in1[i] + feedback * delayOut[1];
    delayOut[0] = delay[0]->process(inL + interpPanIn[0].process() * (inR - inL));
    delayOut[1] = delay[1]->process(inL + interpPanIn[1].process() * (inR - inL));

    const float tone = interpTone.process();
    const float toneMix = interpToneMix.process();
    filter[0]->setCutoff(tone);
    filter[1]->setCutoff(tone);
    const float filterOutL = filter[0]->process(delayOut[0]);
    const float filterOutR = filter[1]->process(delayOut[1]);
    delayOut[0] = filterOutL + toneMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + toneMix * (delayOut[1] - filterOutR);

    const float wet = interpWetMix.process();
    const float dry = interpDryMix.process();
    const float outL = wet * delayOut[0];
    const float outR = wet * delayOut[1];
    out0[i] = dry * in0[i] + outL + interpPanOut[0].process() * (outR - outL);
    out1[i] = dry * in1[i] + outL + interpPanOut[1].process() * (outR - outL);

    if (!param.lfoHold) {
      lfoPhase += interpLfoFrequency.process() * lfoPhaseTick;
      if (lfoPhase > 2.0 * M_PI) lfoPhase -= M_PI;
    }
  }
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
  return param.setState(state);
}

tresult PLUGIN_API PlugProcessor::getState(IBStream *state)
{
  return param.getState(state);
}

} // namespace SevenDelay
} // namespace Steinberg
