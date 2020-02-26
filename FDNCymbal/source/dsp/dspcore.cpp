// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"
#include "../../../lib/juce_FastMathApproximations.h"

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float midiNoteToFrequency(float pitch, float tuning)
{
  return 440.0f * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning) / 1200.0f);
}

float paramToPitch(float bend) { return powf(2.0f, ((bend - 0.5f) * 400.0f) / 1200.0f); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = sampleRate;

  LinearSmoother<float>::setSampleRate(sampleRate);
  LinearSmoother<float>::setTime(0.01f);

  noteStack.reserve(128);
  noteStack.resize(0);

  pulsar.sampleRate = sampleRate;

  stickEnvelope.setup(sampleRate, 0.1f);
  for (auto &osc : stickOscillator) osc.setup(sampleRate, 100.0f);
  velvet.setup(sampleRate, sampleRate * 0.004, 0);

  for (auto &fdn : fdnCascade) fdn.setup(sampleRate, 0.5f);

  serialAP1.setup(sampleRate, 0.01f);
  for (auto &ap : serialAP2) ap.setup(sampleRate, 0.01f);
  serialAP1Highpass.setup(sampleRate);
  serialAP2Highpass.setup(sampleRate);

  tremoloDelay.setup(sampleRate, tremoloDelayMaxTime, tremoloDelayMaxTime);

  interpFDNFeedback.reset(param.value[ParameterID::fdnFeedback]->getFloat());
  interpFDNCascadeMix.reset(param.value[ParameterID::fdnCascadeMix]->getFloat());
  interpAllpassMix.reset(param.value[ParameterID::allpassMix]->getFloat());
  interpAllpass1Feedback.reset(param.value[ParameterID::allpass1Feedback]->getFloat());
  interpAllpass2Feedback.reset(param.value[ParameterID::allpass2Feedback]->getFloat());
  interpTremoloMix.reset(param.value[ParameterID::tremoloMix]->getFloat());
  interpTremoloDepth.reset(param.value[ParameterID::tremoloDepth]->getFloat());
  interpTremoloFrequency.reset(param.value[ParameterID::tremoloFrequency]->getFloat());
  interpTremoloDelayTime.reset(param.value[ParameterID::tremoloDelayTime]->getFloat());
  interpStickToneMix.reset(param.value[ParameterID::stickToneMix]->getFloat());
  interpMasterGain.reset(param.value[ParameterID::gain]->getFloat());

  reset();
  startup();
}

void DSPCore::reset()
{
  serialAP1Sig = 0.0f;
  serialAP1.reset();
  serialAP1Highpass.reset();

  serialAP2Sig = 0.0f;
  for (auto &ap : serialAP2) ap.reset();
  serialAP2Highpass.reset();

  fdnSig = 0.0f;
  for (auto &fdn : fdnCascade) fdn.reset();

  tremoloDelay.reset();

  startup();
}

void DSPCore::startup()
{
  rng.seed = param.value[ParameterID::seed]->getInt();
  tremoloPhase = 0.0f;
}

void DSPCore::setParameters()
{
  using ID = ParameterID::ID;

  LinearSmoother<float>::setTime(param.value[ID::smoothness]->getFloat());

  if (!noteStack.empty()) {
    velocity = noteStack.back().velocity;
    const auto freq
      = noteStack.back().frequency * paramToPitch(param.value[ID::pitchBend]->getFloat());
    interpPitch.push(freq);
  } else {
    interpPitch.push(0.0f);
  }
  interpMasterGain.push(velocity * param.value[ID::gain]->getFloat());

  interpStickToneMix.push(param.value[ID::stickToneMix]->getFloat());
  interpStickPulseMix.push(param.value[ID::stickPulseMix]->getFloat());
  interpStickVelvetMix.push(param.value[ID::stickVelvetMix]->getFloat());

  interpFDNFeedback.push(param.value[ID::fdnFeedback]->getFloat());
  interpFDNCascadeMix.push(param.value[ID::fdnCascadeMix]->getFloat());

  interpAllpassMix.push(param.value[ID::allpassMix]->getFloat());
  interpAllpass1Feedback.push(param.value[ID::allpass1Feedback]->getFloat());
  interpAllpass2Feedback.push(param.value[ID::allpass2Feedback]->getFloat());

  interpTremoloMix.push(param.value[ID::tremoloMix]->getFloat());
  interpTremoloDepth.push(randomTremoloDepth * param.value[ID::tremoloDepth]->getFloat());
  interpTremoloFrequency.push(
    randomTremoloFrequency * param.value[ID::tremoloFrequency]->getFloat());
  interpTremoloDelayTime.push(
    randomTremoloDelayTime * param.value[ID::tremoloDelayTime]->getFloat());

  serialAP1Highpass.setCutoffQ(
    param.value[ID::allpass1HighpassCutoff]->getFloat(), highpassQ);
  serialAP2Highpass.setCutoffQ(
    param.value[ID::allpass2HighpassCutoff]->getFloat(), highpassQ);
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  LinearSmoother<float>::setBufferSize(length);

  for (auto &fdn : fdnCascade)
    for (auto &time : fdn.delayTime) time.refresh();
  for (auto &ap : serialAP1.allpass) ap.delayTime.refresh();
  for (auto &section : serialAP2)
    for (auto &ap : section.allpass) ap.delayTime.refresh();

  const bool enableFDN = param.value[ParameterID::fdn]->getInt();
  const bool allpass1Saturation = param.value[ParameterID::allpass1Saturation]->getInt();
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    float sample = 0.0f;
    if (in0 != nullptr) sample += in0[i];
    if (in1 != nullptr) sample += in1[i];

    const float pitch = interpPitch.process();
    if (!stickEnvelope.isTerminated) {
      const float toneMix = interpStickToneMix.process();
      const float pulseMix = interpStickPulseMix.process();
      const float velvetMix = interpStickVelvetMix.process();
      const float stickEnv = stickEnvelope.process();
      float stickTone = 0.0f;
      for (auto &osc : stickOscillator) stickTone += osc.process();
      velvet.setDensity(pitch);
      sample += pulseMix * pulsar.process()
        + stickEnv * (toneMix * stickTone + velvetMix * velvet.process());
    }

    // FDN.
    if (enableFDN) {
      const float fdnFeedback = interpFDNFeedback.process();
      fdnSig = fdnCascade[0].process(
        juce::dsp::FastMathApproximations::tanh<float>(sample + fdnFeedback * fdnSig));
      const float fdnCascadeMix = interpFDNCascadeMix.process();
      for (size_t j = 1; j < fdnCascade.size(); ++j) {
        fdnSig += fdnCascadeMix * (fdnCascade[j].process(fdnSig * 2.0f) - fdnSig);
      }
      sample = fdnSig * 1024.0;
    }

    // Allpass.
    serialAP1Sig = allpass1Saturation
      ? juce::dsp::FastMathApproximations::tanh(serialAP1Sig)
      : serialAP1Sig;
    serialAP1Sig
      = serialAP1.process(sample + interpAllpass1Feedback.process() * serialAP1Sig);
    float apOut = serialAP1Highpass.process(serialAP1Sig);

    serialAP2Sig = apOut + interpAllpass2Feedback.process() * serialAP2Sig;
    float sum = 0.0f;
    for (auto &ap : serialAP2) sum += ap.process(serialAP2Sig);
    serialAP2Sig = sum / serialAP2.size();
    apOut += 4.0f * serialAP2Highpass.process(serialAP2Sig);

    const float allpassMix = interpAllpassMix.process();
    sample += allpassMix * (apOut - sample);

    // Tremolo.
    tremoloPhase += interpTremoloFrequency.process() * float(twopi) / sampleRate;
    if (tremoloPhase >= float(twopi)) tremoloPhase -= float(twopi);

    const float tremoloLFO = 0.5f * (sinf(tremoloPhase) + 1.0f);
    tremoloDelay.setTime(interpTremoloDelayTime.process() * tremoloLFO);

    const float tremoloDepth = interpTremoloDepth.process();
    sample += interpTremoloMix.process()
      * ((tremoloDepth * tremoloLFO + 1.0f - tremoloDepth) * tremoloDelay.process(sample)
         - sample);

    const float masterGain = interpMasterGain.process();
    out0[i] = masterGain * sample;
    out1[i] = masterGain * sample;
  }
}

void DSPCore::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  NoteInfo info;
  info.id = noteId;
  info.frequency = midiNoteToFrequency(pitch, tuning);
  info.velocity = velocity;
  noteStack.push_back(info);

  const auto seed = param.value[ParameterID::seed]->getInt();

  // Set stick oscillator.
  if (param.value[ParameterID::stick]->getInt()) {
    if (param.value[ParameterID::retriggerStick]->getInt()) {
      rngStick.seed = seed;
      velvet.rng.seed = seed;
    }

    pulsar.phase = 1.0f;

    stickEnvelope.reset(param.value[ParameterID::stickDecay]->getFloat());
    const auto upperBound = 0.95f * sampleRate / 2.0f;
    auto oscFreq = info.frequency;
    while (oscFreq >= upperBound) oscFreq *= 0.5f;
    for (auto &osc : stickOscillator) {
      if (oscFreq < 20.0f) oscFreq += 20.0f;
      osc.setFrequency(oscFreq);
      oscFreq *= 1.0f + rngStick.process();
      if (oscFreq > upperBound) oscFreq = info.frequency * (1.0f + rngStick.process());
    }

    velvet.setDensity(sampleRate * 0.004f * (pitch + 1) / 32.0f);
  }

  // Set fdnCascade.
  if (param.value[ParameterID::retriggerTime]->getInt())
    rng.seed = param.value[ParameterID::seed]->getInt();

  const float fdnTime = param.value[ParameterID::fdnTime]->getFloat();
  for (size_t n = 0; n < fdnCascade.size(); ++n) {
    float diagMod = float(n + 1) / fdnCascade.size();
    float delayTimeMod = somepow<float>(diagMod * 2.0f, 0.8f);
    for (size_t i = 0; i < fdnMatrixSize; ++i) {
      for (size_t j = 0; j < fdnMatrixSize; ++j) {
        if (i == j)
          fdnCascade[n].matrix[i][j] = 1 - diagMod - 0.5f * (rng.process() - diagMod);
        else
          fdnCascade[n].matrix[i][j] = -0.5f * rng.process();
      }
      fdnCascade[n].gain[i] = (rng.process() < 0.5f ? 1.0f : -1.0f)
        * (0.1f + rng.process()) * 2.0f / fdnMatrixSize;
      fdnCascade[n].delayTime[i].push(rng.process() * delayTimeMod * fdnTime);
    }
  }

  // Set serialAP.
  float ap1Time = param.value[ParameterID::allpass1Time]->getFloat();
  for (auto &ap : serialAP1.allpass) {
    ap.set(0.001f + 0.999f * rng.process(), ap1Time + ap1Time * rng.process());
    ap1Time *= 1.5f;
  }

  float ap2Time = param.value[ParameterID::allpass2Time]->getFloat();
  for (auto &allpass : serialAP2) {
    for (auto &ap : allpass.allpass)
      ap.set(0.001f + 0.999f * rng.process(), ap2Time + ap2Time * rng.process());
    ap2Time *= 1.5f;
  }

  // Set tremolo.
  if (param.value[ParameterID::retriggerTremolo]->getInt()) rngTremolo.seed = seed;

  randomTremoloDepth = 1.0f
    + param.value[ParameterID::randomTremoloDepth]->getFloat()
      * (rngTremolo.process() - 1.0f);
  randomTremoloFrequency = 1.0f
    + param.value[ParameterID::randomTremoloFrequency]->getFloat()
      * (rngTremolo.process() - 1.0f);
  randomTremoloDelayTime = 1.0f
    + param.value[ParameterID::randomTremoloDelayTime]->getFloat()
      * (rngTremolo.process() - 1.0f);
}

void DSPCore::noteOff(int32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);
}
