// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EsPhaser.
//
// EsPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EsPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EsPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "smoother.hpp"

#include "../../../lib/juce_FastMathApproximations.h"
#include "../../../lib/vcl/vectorclass.h"
#include "../../../lib/vcl/vectormath_trig.h"

#include <array>
#include <iostream>

namespace SomeDSP {

struct alignas(64) ThiranAllpass2x16 {
  Vec16f x0 = 0.0f;
  Vec16f x1 = 0.0f;
  Vec16f x2 = 0.0f;
  Vec16f y0 = 0.0f;
  Vec16f y1 = 0.0f;
  Vec16f y2 = 0.0f;
  Vec16f a1 = 0.0f;
  Vec16f a2 = 0.0f;

  void reset()
  {
    x0 = 0.0f;
    x1 = 0.0f;
    x2 = 0.0f;
    y0 = 0.0f;
    y1 = 0.0f;
    y2 = 0.0f;
  }

  // fraction > 0.01.
  void step(float input, Vec16f fraction)
  {
    auto delay = 2.0f - fraction;
    auto tmp = (delay - 2.0f) / (delay + 1.0f);
    a1 = -2.0f * tmp;
    a2 = (delay - 1.0f) / (delay + 2.0f) * tmp;

    x2 = x1;
    x1 = x0;
    x0 = permute16<V_DC, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14>(x0);
    x0.insert(0, input);

    y2 = y1;
    y1 = y0;
    y0 = a2 * x0 + a1 * x1 + x2 - a1 * y1 - a2 * y2;
  }

  float get(int index) { return y0.extract(index); }
};

struct alignas(64) Thiran2Phaser {
  std::array<ThiranAllpass2x16, 256> allpass;
  Vec16f phase{0};
  float buffer = 0;
  float sampleRate = 44100;
  std::array<int, 2> stage{15, 15};
  std::array<int, 2> index{};
  int arrayStop = 255;
  LinearSmootherLocal<float> interpStage;

  // Convert UI value to DSP value. Used to set offset outside of main processing loop.
  static float getLfoMin(float range, float min) { return min + range - 0.99f; }

  void setStage(int newStage)
  {
    if (newStage < 0) return;
    if (interpStage.getValue() != 1.0f) return;

    interpStage.reset(0);
    interpStage.push(1.0f);

    index[0] = index[1];
    index[1] = newStage >> 4;

    arrayStop = index[0] > index[1] ? index[0] : index[1];

    stage[0] = stage[1];
    stage[1] = newStage - (arrayStop << 4);
  }

  void setup(float sampleRate)
  {
    this->sampleRate = sampleRate;
    interpStage.setSampleRate(sampleRate);
    interpStage.setTime(0.04f);
    interpStage.reset(1.0f);
  }

  void reset()
  {
    for (auto &ap : allpass) ap.reset();
    phase = 0;
    buffer = 0;
  }

  // tick = frequency / sampleRate.
  // Stable only if (lfoRange - lfoMin) <= 0.99f.
  float process(
    const size_t bufferSize,
    float input,
    float freqSpread,
    float cascadeOffset,
    float stereoOffset,
    float tick,
    float feedback,
    float lfoRange,
    float lfoMin)
  {
    interpStage.setBufferSize(bufferSize);

    Vec16f tck;
    Vec16f offset;
    for (int i = 0; i < 16; ++i) {
      tck.insert(i, freqSpread * i);
      offset.insert(i, stereoOffset + i * cascadeOffset);
    }

    phase += tick / (1.0f + tck);
    phase = select(phase > float(pi), phase - float(twopi), phase);

    Vec16f lfo = lfoRange * sin(phase + offset) - lfoMin;

    buffer = juce::dsp::FastMathApproximations::tanh(input + feedback * buffer);
    for (int i = 0; i <= arrayStop; ++i) {
      allpass[i].step(buffer, lfo);
      buffer = allpass[i].get(15);
    }

    auto sig0 = allpass[index[0]].get(stage[0]);
    auto sig1 = allpass[index[1]].get(stage[1]);
    buffer = sig0 + interpStage.process() * (sig1 - sig0);

    return buffer;
  }
};

} // namespace SomeDSP
