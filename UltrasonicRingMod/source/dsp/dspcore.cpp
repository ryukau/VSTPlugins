// (c) 2022 Takamitsu Endo
//
// This file is part of UltrasonicRingMod.
//
// UltrasonicRingMod is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltrasonicRingMod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltrasonicRingMod.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

constexpr float feedbackLimiterAttackSeconds = 64.0f / 48000.0f;

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  auto upRate = float(sampleRate) * upFold;

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(0.2f);

  reset();
  startup();
}

size_t DSPCore::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  pitchSmoothingKp = float(                                                              \
    EMAFilter<float>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getFloat()));       \
                                                                                         \
  interpMix.METHOD(pv[ID::mix]->getFloat());                                             \
  interpFrequencyHz.METHOD(pv[ID::frequencyHz]->getFloat());                             \
  interpDCOffset.METHOD(pv[ID::dcOffset]->getFloat());                                   \
  interpFeedbackGain.METHOD(pv[ID::feedbackGain]->getFloat());                           \
  interpModFrequencyScaling.METHOD(pv[ID::modFrequencyScaling]->getFloat());             \
  interpModClipMix.METHOD(pv[ID::modClipMix]->getFloat());                               \
  interpHardclipMix.METHOD(pv[ID::hardclipMix]->getFloat());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  interpPitch.reset(float(1));

  for (auto &us : upSampler) us.reset();
  for (auto &lp : firstStageLowpass) lp.reset();
  feedback.fill({});
  halfBandInput.fill({});
  for (auto &hb : halfbandIir) hb.reset();

  startup();
}

void DSPCore::startup() { phase = 0; }

void DSPCore::setParameters() { ASSIGN_PARAMETER(push); }

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  SmootherCommon<float>::setBufferSize(float(length));
  SmootherCommon<float>::setSampleRate(upRate);

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    float sig0 = in0[i];
    float sig1 = in1[i];

    upSampler[0].process(sig0);
    upSampler[1].process(sig1);

    for (size_t j = 0; j < 2; ++j) {                // Halfband downsampler.
      for (size_t k = 0; k < firstStateFold; ++k) { // Stage 1 downsampler.
        auto mix = interpMix.process();
        auto freq = interpPitch.process(pitchSmoothingKp) * interpFrequencyHz.process();
        auto dc = interpDCOffset.process();
        auto fbGain = interpFeedbackGain.process();
        auto modScale = interpModFrequencyScaling.process();
        auto modClip = interpModClipMix.process();
        auto hardclip = interpHardclipMix.process();

        phase += freq / upRate;
        phase -= std::floor(phase);

        // Sine.
        auto fb = feedback[0] + feedback[1];
        auto mod = std::sin(float(twopi) * phase + fbGain * fb);
        auto gain = (dc + mod) * (float(1) - float(0.5) * dc);

        gain *= float(1) + modScale * (freq - float(1));
        gain += modClip * (std::clamp(gain, float(-1), float(1)) - gain);
        gain = float(1) + mix * (gain - float(1));

        size_t index = k + firstStateFold * j;

        double s0 = gain * upSampler[0].output[index];
        double s1 = gain * upSampler[1].output[index];
        double h0 = std::clamp(s0, double(-1), double(1));
        double h1 = std::clamp(s1, double(-1), double(1));
        feedback[0] = float(s0 + hardclip * (h0 - s0));
        feedback[1] = float(s1 + hardclip * (h1 - s1));

        firstStageLowpass[0].push(feedback[0]);
        firstStageLowpass[1].push(feedback[1]);
      }
      halfBandInput[0][j] = firstStageLowpass[0].output();
      halfBandInput[1][j] = firstStageLowpass[1].output();
    }

    out0[i] = halfbandIir[0].process(halfBandInput[0]);
    out1[i] = halfbandIir[1].process(halfBandInput[1]);
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  interpPitch.push(calcNotePitch(info.pitch));

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    interpPitch.push(float(1));
  } else {
    interpPitch.push(calcNotePitch(noteStack.back().pitch));
  }
}

float DSPCore::calcNotePitch(float note, float equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto offset = pv[ID::centerNoteNumber]->getFloat();
  auto scale = pv[ID::noteScaling]->getFloat();
  if (pv[ID::noteScalingNegative]->getInt()) scale = -scale;
  return std::exp2(scale * (note - offset) / equalTemperament);
}
