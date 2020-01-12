// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <cmath>

#include "../../../lib/juce_FastMathApproximations.h"
#include "../../../lib/vcl/vectorclass.h"

namespace SomeDSP {

// Order 2 Thiran all-pass filter.
template<typename Sample> struct ThiranAllpass2 {
  Sample a1 = 0;
  Sample a2 = 0;
  Sample x0 = 0;
  Sample x1 = 0;
  Sample x2 = 0;
  Sample y0 = 0;
  Sample y1 = 0;
  Sample y2 = 0;

  // fraction in [0.01, 0.99].
  void setup(Sample fraction)
  {
    auto delay = Sample(2) - fraction;
    auto tmp = (delay - Sample(2)) / (delay + Sample(1));
    a1 = Sample(-2) * tmp;
    a2 = (delay - Sample(1)) / (delay + Sample(2)) * tmp;
  }

  void reset()
  {
    x0 = x1 = x2 = 0;
    y0 = y1 = y2 = 0;
  }

  Sample process(Sample input)
  {
    x2 = x1;
    x1 = x0;
    x0 = input;

    y2 = y1;
    y1 = y0;
    y0 = a2 * x0 + a1 * x1 + x2 - a1 * y1 - a2 * y2;

    return y0;
  }
};

template<typename Sample> struct Thiran2Phaser {
  std::array<ThiranAllpass2<Sample>, 16> allpass; // No sound if size > 16.
  Sample phase = 0;
  Sample offset = 0;
  Sample tick = 0; // frequency / sampleRate;
  Sample feedback = 0;
  Sample lfoAmp = 0;
  Sample lfoMin = 0; // Stable only if (lfoAmp - lfoMin) <= 0.99f.
  Sample buffer = 0;

  // Convert UI value to DSP value. Used to set offset outside of main processing loop.
  static Sample getOffset(Sample range, Sample min) { return min + range - 0.99f; }

  void setup(Sample offset, Sample tick, Sample feedback, Sample range, Sample min)
  {
    this->offset = offset;
    this->tick = tick;
    this->feedback = feedback;

    lfoAmp = range;
    lfoMin = min;
  }

  void reset()
  {
    for (auto &ap : allpass) ap.reset();
    buffer = 0;
  }

  Sample process(Sample input)
  {
    phase += tick;
    if (phase > Sample(pi)) phase -= Sample(twopi);

    auto lfo = lfoAmp * sinf(phase + offset) - lfoMin;

    input = juce::dsp::FastMathApproximations::tanh(input + feedback * buffer);
    for (auto &ap : allpass) {
      ap.setup(lfo);
      input = ap.process(input);
    }

    buffer = input;
    return input;
  }
};

struct alignas(64) ThiranAllpass2x16 {
  Vec16f x0 = 0.0f;
  Vec16f x1 = 0.0f;
  Vec16f x2 = 0.0f;
  Vec16f y0 = 0.0f;
  Vec16f y1 = 0.0f;
  Vec16f y2 = 0.0f;
  float a1 = 0.0f;
  float a2 = 0.0f;
  int index = 14;

  // fraction in [0.01, 0.99].
  void setup(float fraction)
  {
    auto delay = 2.0f - fraction;
    auto tmp = (delay - 2.0f) / (delay + 1.0f);
    a1 = -2.0f * tmp;
    a2 = (delay - 1.0f) / (delay + 2.0f) * tmp;
  }

  void reset()
  {
    x0 = 0.0f;
    x1 = 0.0f;
    x2 = 0.0f;
    y0 = 0.0f;
    y1 = 0.0f;
    y2 = 0.0f;
  }

  float process(float input)
  {
    x2 = x1;
    x1 = x0;
    x0 = permute16<V_DC, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14>(x0);
    x0.insert(0, input);

    y2 = y1;
    y1 = y0;
    y0 = a2 * x0 + a1 * x1 + x2 - a1 * y1 - a2 * y2;

    return y0.extract(index);
  }
};

struct Thiran2Phaser16 {
  ThiranAllpass2x16 allpass;
  float phase = 0;
  float offset = 0;
  float tick = 0; // frequency / sampleRate;
  float feedback = 0;
  float lfoAmp = 0;
  float lfoMin = 0; // Stable only if (lfoAmp - lfoMin) <= 0.99f.
  float buffer = 0;

  // Convert UI value to DSP value. Used to set offset outside of main processing loop.
  static float getOffset(float range, float min) { return min + range - 0.99f; }

  void setStage(int stage)
  {
    if (stage >= 0 && stage < 16) allpass.index = stage;
  }

  void setup(float offset, float tick, float feedback, float range, float min)
  {
    this->offset = offset;
    this->tick = tick;
    this->feedback = feedback;

    lfoAmp = range;
    lfoMin = min;
  }

  void reset()
  {
    allpass.reset();
    buffer = 0;
  }

  float process(float input)
  {
    phase += tick;
    if (phase > float(pi)) phase -= float(twopi);

    auto lfo = lfoAmp * sinf(phase + offset) - lfoMin;

    input = juce::dsp::FastMathApproximations::tanh(input + feedback * buffer);
    allpass.setup(lfo);
    buffer = allpass.process(input);
    return buffer;
  }
};

} // namespace SomeDSP
