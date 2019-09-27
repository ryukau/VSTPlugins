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

#include "dspcore.hpp"

namespace Steinberg {
namespace SevenDelay {

constexpr size_t channel = 2;

float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

void DSPCore::setup(double sampleRate)
{
  LinearSmoother<float>::setSampleRate(sampleRate);

  for (size_t i = 0; i < delay.size(); ++i)
    delay[i] = std::make_unique<DelayTypeName>(sampleRate, 1.0f, maxDelayTime);

  for (size_t i = 0; i < filter.size(); ++i)
    filter[i] = std::make_unique<FilterTypeName>(sampleRate, maxToneFrequency, 0.9);

  for (size_t i = 0; i < dcKiller.size(); ++i)
    dcKiller[i] = std::make_unique<DCKillerTypeName>(sampleRate, minDCKillFrequency, 0.1);

  lfoPhaseTick = 2.0 * pi / sampleRate;

  startup();
}

void DSPCore::free()
{
  for (size_t i = 0; i < channel; ++i) {
    delay[i].reset();
    filter[i].reset();
    dcKiller[i].reset();
  }
}

void DSPCore::reset()
{
  for (size_t i = 0; i < channel; ++i) {
    delay[i]->reset();
    filter[i]->reset();
    dcKiller[i]->reset();
  }
  startup();
}

void DSPCore::startup()
{
  delayOut[0] = 0.0f;
  delayOut[1] = 0.0f;
  lfoPhase = param.lfoInitialPhase;
}

void DSPCore::setParameters(double tempo)
{
  auto smoothness = GlobalParameter::scaleSmoothness.map(param.smoothness);
  LinearSmoother<float>::setTime(smoothness);

  // This won't work if sync is on and tempo < 15. Up to 8 sec or 8/16 beat.
  // 15.0 is come from (60 sec per minute) * (4 beat) / (16 beat).
  Vst::ParamValue time = GlobalParameter::scaleTime.map(param.time);
  if (param.tempoSync) {
    if (time < 1.0)
      time *= 15.0 / tempo;
    else
      time = floor(2.0 * time) * 7.5 / tempo;
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
  interpLfoTimeAmount.push(GlobalParameter::scaleLfoTimeAmount.map(param.lfoTimeAmount));
  interpLfoToneAmount.push(GlobalParameter::scaleLfoToneAmount.map(param.lfoToneAmount));
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

  interpToneCutoff.push(GlobalParameter::scaleToneCutoff.map(param.toneCutoff));
  interpToneQ.push(GlobalParameter::scaleToneQ.map(param.toneQ));
  interpToneMix.push(GlobalParameter::scaleToneMix.map(param.toneCutoff));

  interpDCKill.push(GlobalParameter::scaleDCKill.map(param.dckill));
  interpDCKillMix.push(GlobalParameter::scaleDCKillMix.reverseMap(param.dckill));
}

void DSPCore::process(
  const size_t length, float *in0, float *in1, float *out0, float *out1)
{
  for (size_t i = 0; i < length; ++i) {
    auto sign = (pi < lfoPhase) - (lfoPhase < pi);
    const float lfo = 1.0f + sign * powf(fabsf(sin(lfoPhase)), interpLfoShape.process());
    const float lfoTime = interpLfoTimeAmount.process() * lfo;

    delay[0]->setTime(interpTime[0].process() + lfoTime);
    delay[1]->setTime(interpTime[1].process() + lfoTime);

    const float feedback = interpFeedback.process();
    const float inL = in0[i] + feedback * delayOut[0];
    const float inR = in1[i] + feedback * delayOut[1];
    delayOut[0] = delay[0]->process(inL + interpPanIn[0].process() * (inR - inL));
    delayOut[1] = delay[1]->process(inL + interpPanIn[1].process() * (inR - inL));

    const float lfoTone = 1.0f - interpLfoToneAmount.process() * lfo;
    float toneCutoff = interpToneCutoff.process() * lfoTone;
    if (toneCutoff < 20.0f) toneCutoff = 20.0f;
    const float toneQ = interpToneQ.process();
    filter[0]->setCutoff(toneCutoff);
    filter[0]->setQ(toneQ);
    filter[1]->setCutoff(toneCutoff);
    filter[1]->setQ(toneQ);
    float filterOutL = filter[0]->process(delayOut[0]);
    float filterOutR = filter[1]->process(delayOut[1]);
    const float toneMix = interpToneMix.process() * lfoTone;
    delayOut[0] = filterOutL + toneMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + toneMix * (delayOut[1] - filterOutR);

    const float dckill = interpDCKill.process();
    dcKiller[0]->setCutoff(dckill);
    dcKiller[1]->setCutoff(dckill);
    filterOutL = dcKiller[0]->process(delayOut[0]);
    filterOutR = dcKiller[1]->process(delayOut[1]);
    const float dckillMix = interpDCKillMix.process();
    // dckillmix == 1 -> delayout
    delayOut[0] = filterOutL + dckillMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + dckillMix * (delayOut[1] - filterOutR);

    const float wet = interpWetMix.process();
    const float dry = interpDryMix.process();
    const float outL = wet * delayOut[0];
    const float outR = wet * delayOut[1];
    out0[i] = dry * in0[i] + outL + interpPanOut[0].process() * (outR - outL);
    out1[i] = dry * in1[i] + outL + interpPanOut[1].process() * (outR - outL);

    if (!param.lfoHold) {
      lfoPhase += interpLfoFrequency.process() * lfoPhaseTick;
      if (lfoPhase > 2.0 * pi) lfoPhase -= pi;
    }
  }
}

} // namespace SevenDelay
} // namespace Steinberg
