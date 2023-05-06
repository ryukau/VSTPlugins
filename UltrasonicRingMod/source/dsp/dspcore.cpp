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

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

template<typename T> inline T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = double(sampleRate);
  upRate = double(sampleRate) * upFold;

  SmootherCommon<double>::setSampleRate(upRate);

  reset();
  startup();
}

size_t DSPCore::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  SmootherCommon<double>::setTime(pv[ID::parameterSmoothingSecond]->getDouble());        \
                                                                                         \
  pitchSmoothingKp = double(                                                             \
    EMAFilter<double>::secondToP(upRate, pv[ID::noteSlideTimeSecond]->getDouble()));     \
                                                                                         \
  interpPreClipGain.METHOD(pv[ID::preClipGain]->getDouble());                            \
  interpOutputGain.METHOD(pv[ID::outputGain]->getDouble());                              \
                                                                                         \
  interpMix.METHOD(pv[ID::mix]->getDouble());                                            \
  interpFrequencyHz.METHOD(pv[ID::frequencyHz]->getDouble());                            \
  interpDCOffset.METHOD(pv[ID::dcOffset]->getDouble());                                  \
  interpFeedbackGain.METHOD(pv[ID::feedbackGain]->getDouble());                          \
  interpModFrequencyScaling.METHOD(pv[ID::modFrequencyScaling]->getDouble());            \
  interpModWrapMix.METHOD(pv[ID::modWrapMix]->getDouble());                              \
  interpHardclipMix.METHOD(pv[ID::hardclipMix]->getDouble());

void DSPCore::reset()
{
  ASSIGN_PARAMETER(reset);

  midiNotes.clear();
  noteStack.clear();

  interpPitch.reset(double(1));

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
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<double>::setBufferSize(double(length));

  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    upSampler[0].process(in0[i]);
    upSampler[1].process(in1[i]);

    for (size_t j = 0; j < 2; ++j) {                // Halfband downsampler.
      for (size_t k = 0; k < firstStateFold; ++k) { // Stage 1 downsampler.
        auto preClipGain = interpPreClipGain.process();
        auto outputGain = interpOutputGain.process();
        auto mix = interpMix.process();
        auto freq = interpPitch.process(pitchSmoothingKp) * interpFrequencyHz.process();
        auto dc = interpDCOffset.process();
        auto fbGain = interpFeedbackGain.process();
        auto modScale = interpModFrequencyScaling.process();
        auto modWrap = interpModWrapMix.process();
        auto hardclip = interpHardclipMix.process();

        phase += freq / upRate;
        phase -= std::floor(phase);

        // Sine.
        auto fb = feedback[0] + feedback[1];
        auto mod = std::sin(double(twopi) * phase + fbGain * fb);
        auto gain = (dc + mod) * (double(1) - double(0.5) * dc);

        gain *= lerp(double(1), freq, modScale);
        gain -= modWrap * std::floor(gain);
        gain = lerp(double(1), gain, mix);

        size_t index = k + firstStateFold * j;

        double s0 = gain * upSampler[0].output[index];
        double s1 = gain * upSampler[1].output[index];
        double h0 = std::clamp(preClipGain * s0, double(-1), double(1));
        double h1 = std::clamp(preClipGain * s1, double(-1), double(1));
        feedback[0] = double(s0 + hardclip * (h0 - s0));
        feedback[1] = double(s1 + hardclip * (h1 - s1));

        firstStageLowpass[0].push(outputGain * feedback[0]);
        firstStageLowpass[1].push(outputGain * feedback[1]);
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
    interpPitch.push(double(1));
  } else {
    interpPitch.push(calcNotePitch(noteStack.back().pitch));
  }
}

double DSPCore::calcNotePitch(double note, double equalTemperament)
{
  using ID = ParameterID::ID;
  auto &pv = param.value;

  auto offset = pv[ID::noteOffset]->getDouble();
  auto scale = pv[ID::noteScaling]->getDouble();
  if (pv[ID::noteScalingNegative]->getInt()) scale = -scale;
  return std::exp2(scale * (note + offset - 69) / equalTemperament);
}
