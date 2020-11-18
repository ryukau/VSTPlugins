// (c) 2019-2020 Takamitsu Endo
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
namespace Synth {

constexpr size_t channel = 2;

inline std::array<float, 2> calcPan(float inL, float inR, float pan, float spread)
{
  float balanceL = std::clamp<float>(spread, 0.0f, 1.0f);
  float balanceR = std::clamp<float>(1.0f - spread, 0.0f, 1.0f);
  float sigL = inL + balanceL * (inR - inL);
  float sigR = inL + balanceR * (inR - inL);

  pan = std::clamp<float>(pan, 0.0f, 1.0f);
  if (pan < 0.5f) {
    return {
      (0.5f + pan) * sigL + (0.5f - pan) * sigR,
      sigR * 2.0f * pan,
    };
  }
  return {
    sigL * (2.0f - 2.0f * pan),
    (pan - 0.5f) * sigL + (1.5f - pan) * sigR,
  };
}

void DSPCore::setup(double sampleRate)
{
  SmootherCommon<float>::setSampleRate(sampleRate);

  for (size_t i = 0; i < delay.size(); ++i)
    delay[i].setup(sampleRate, 1.0f, maxDelayTime);

  for (size_t i = 0; i < filter.size(); ++i) filter[i].setup(sampleRate);

  for (size_t i = 0; i < dcKiller.size(); ++i) dcKiller[i].setup(sampleRate, 0.1);

  lfoPhaseTick = 2.0 * pi / sampleRate;

  startup();
}

void DSPCore::reset()
{
  for (size_t i = 0; i < channel; ++i) {
    delay[i].reset();
    filter[i].reset();
    dcKiller[i].reset();
  }
  startup();
}

void DSPCore::startup()
{
  delayOut[0] = 0.0f;
  delayOut[1] = 0.0f;
  lfoPhase = param.value[ParameterID::lfoInitialPhase]->getFloat();
}

void DSPCore::setParameters(double tempo)
{
  SmootherCommon<float>::setTime(param.value[ParameterID::smoothness]->getFloat());

  // This won't work if sync is on and tempo < 15. Up to 8 sec or 8/16 beat.
  // 15.0 comes from (60 sec per minute) * (4 beat) / (16 beat).
  auto time = param.value[ParameterID::time]->getFloat();
  if (param.value[ParameterID::tempoSync]->getInt()) {
    if (time < 1.0)
      time *= 15.0 / tempo;
    else
      time = floor(2.0 * time) * 7.5 / tempo;
  }

  auto offset = param.value[ParameterID::offset]->getFloat();
  interpTime[0].push(offset < 0.0 ? time * (1.0 + offset) : time);
  interpTime[1].push(offset > 0.0 ? time * (1.0 - offset) : time);

  interpWetMix.push(param.value[ParameterID::wetMix]->getFloat());
  interpDryMix.push(param.value[ParameterID::dryMix]->getFloat());
  interpFeedback.push(
    param.value[ParameterID::negativeFeedback]->getInt()
      ? -param.value[ParameterID::feedback]->getFloat()
      : param.value[ParameterID::feedback]->getFloat());
  interpLfoTimeAmount.push(param.value[ParameterID::lfoTimeAmount]->getFloat());
  interpLfoToneAmount.push(param.value[ParameterID::lfoToneAmount]->getFloat());
  interpLfoFrequency.push(param.value[ParameterID::lfoFrequency]->getFloat());
  interpLfoShape.push(param.value[ParameterID::lfoShape]->getFloat());

  interpPanIn.push(param.value[ParameterID::inPan]->getFloat());
  interpSpreadIn.push(param.value[ParameterID::inSpread]->getFloat());
  interpPanOut.push(param.value[ParameterID::outPan]->getFloat());
  interpSpreadOut.push(param.value[ParameterID::outSpread]->getFloat());

  interpToneCutoff.push(param.value[ParameterID::toneCutoff]->getFloat());
  interpToneQ.push(param.value[ParameterID::toneQ]->getFloat());
  interpToneMix.push(
    Scales::toneMix.map(param.value[ParameterID::toneCutoff]->getNormalized()));

  interpDCKill.push(param.value[ParameterID::dckill]->getFloat());
  interpDCKillMix.push(
    Scales::dckillMix.reverseMap(param.value[ParameterID::dckill]->getNormalized()));
}

void DSPCore::process(
  const size_t length, float *in0, float *in1, float *out0, float *out1)
{
  SmootherCommon<float>::setBufferSize(length);

  const bool lfoHold = !param.value[ParameterID::lfoHold]->getInt();
  for (size_t i = 0; i < length; ++i) {
    auto sign = (pi < lfoPhase) - (lfoPhase < pi);
    const float lfo = sign * powf(fabsf(float(sin(lfoPhase))), interpLfoShape.process());
    const float lfoTime = interpLfoTimeAmount.process() * (1.0f + lfo);

    delay[0].setTime(interpTime[0].process() + lfoTime);
    delay[1].setTime(interpTime[1].process() + lfoTime);

    const float feedback = interpFeedback.process();
    const auto inDelay
      = calcPan(in0[i], in1[i], interpPanIn.process(), interpSpreadIn.process());
    delayOut[0] = delay[0].process(inDelay[0] + feedback * delayOut[0]);
    delayOut[1] = delay[1].process(inDelay[1] + feedback * delayOut[1]);

    const float lfoTone = interpLfoToneAmount.process() * (0.5f * lfo + 0.5f);
    float toneCutoff = interpToneCutoff.process() * lfoTone * lfoTone;
    if (toneCutoff < 20.0f) toneCutoff = 20.0f;
    const float toneQ = interpToneQ.process();
    filter[0].setCutoffQ(toneCutoff, toneQ);
    filter[1].setCutoffQ(toneCutoff, toneQ);
    float filterOutL = filter[0].process(delayOut[0]);
    float filterOutR = filter[1].process(delayOut[1]);
    const float toneMix = interpToneMix.process();
    delayOut[0] = filterOutL + toneMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + toneMix * (delayOut[1] - filterOutR);

    const float dckill = interpDCKill.process();
    dcKiller[0].setCutoff(dckill);
    dcKiller[1].setCutoff(dckill);
    filterOutL = dcKiller[0].process(delayOut[0]);
    filterOutR = dcKiller[1].process(delayOut[1]);
    const float dckillMix = interpDCKillMix.process();
    // dckillmix == 1 -> delayout
    delayOut[0] = filterOutL + dckillMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + dckillMix * (delayOut[1] - filterOutR);
    delayOut = calcPan(
      delayOut[0], delayOut[1], interpPanOut.process(), interpSpreadOut.process());

    const float wet = interpWetMix.process();
    const float dry = interpDryMix.process();
    out0[i] = dry * in0[i] + wet * delayOut[0];
    out1[i] = dry * in1[i] + wet * delayOut[1];

    if (lfoHold) {
      lfoPhase += interpLfoFrequency.process() * lfoPhaseTick;
      if (lfoPhase > 2.0f * pi) lfoPhase -= 2.0f * pi;
    }
  }
}

} // namespace Synth
} // namespace Steinberg
