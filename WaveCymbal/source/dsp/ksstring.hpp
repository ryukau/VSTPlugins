// (c) 2019 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <memory>

#include "../../../lib/juce_FastMathApproximations.h"
#include "delay.hpp"
#include "smoother.hpp"
#include "somemath.hpp"
#include "wave.hpp"

namespace SomeDSP {

// One-Zero filter
// https://ccrma.stanford.edu/~jos/filters/One_Zero.html
//
// b1 in [-1, 1].
//
template<typename Sample> class OneZeroLP {
public:
  Sample z1 = 0;
  Sample b1 = 0;

  OneZeroLP(Sample b1) { this->b1 = b1; }

  void reset() { z1 = 0; }

  Sample process(Sample input)
  {
    auto output = b1 * (input - z1) + z1;
    z1 = input;
    return output;
  }
};

// https://en.wikipedia.org/wiki/High-pass_filter
// alpha is smoothing factor.
template<typename Sample> class RCHP {
public:
  Sample alpha = 0;
  Sample y = 0;
  Sample z1 = 0;

  RCHP(Sample alpha)
  {
    this->alpha = alpha;
    y = 0;
    z1 = 0;
  }

  void reset()
  {
    y = 0;
    z1 = 0;
  }

  Sample process(Sample input)
  {
    y = alpha * y + alpha * (input - z1);
    z1 = input;
    return y;
  }
};

// Karplus-Strong algorithm. Min 10hz.
template<typename Sample> class KSString {
public:
  void setup(Sample sampleRate, Sample frequency, Sample decay)
  {
    delay.setup(sampleRate, Sample(1.0) / frequency, Sample(0.1));
    set(frequency, decay);
  }

  void set(Sample frequency, Sample decay)
  {
    this->decay = frequency < Sample(1e-5)
      ? Sample(1.0)
      : somepow<Sample>(Sample(0.5), decay / frequency);

    interpDelayTime.push(Sample(1.0) / frequency);
  }

  void reset()
  {
    feedback = 0;
    decay = Sample(1.0);
    lowpass.reset();
    delay.reset();
  }

  Sample process(Sample input)
  {
    delay.setTime(interpDelayTime.process());
    auto output = delay.process(input + feedback);
    feedback = lowpass.process(output) * decay;
    return highpass.process(output);
  }

protected:
  Sample feedback = 0;
  Sample decay = 0;
  OneZeroLP<Sample> lowpass{0.5};
  RCHP<Sample> highpass{0.5};
  LinearSmoother<Sample> interpDelayTime;
  Delay<Sample> delay;
};

template<typename Sample> class BiquadBandpass {
public:
  Sample fs = 44100;
  Sample f0 = 100;
  Sample q = 0.5;

  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample a0 = 1.0;
  Sample a1 = 0.0;
  Sample a2 = 0.0;

  Sample x1 = 0.0;
  Sample x2 = 0.0;
  Sample y1 = 0.0;
  Sample y2 = 0.0;

  void setup(Sample sampleRate) { fs = sampleRate; }

  void reset()
  {
    a0 = 1;
    b0 = b1 = b2 = a1 = a2 = 0;
    clear();
  }

  void clear()
  {
    x1 = x2 = 0;
    y1 = y2 = 0;
  }

  Sample clamp(Sample value, Sample low, Sample high)
  {
    return value < low ? low : value > high ? high : value;
  }

  void setCutoffQ(Sample hz, Sample q)
  {
    f0 = clamp(hz, Sample(20.0), Sample(20000.0));
    this->q = clamp(q, Sample(1e-5), Sample(1.0));

    Sample w0 = twopi * f0 / fs;
    Sample cos_w0 = somecos<Sample>(w0);
    Sample sin_w0 = somesin<Sample>(w0);

    // 0.34657359027997264 = log(2) / 2.
    Sample alpha
      = sin_w0 * somesinh<Sample>(Sample(0.34657359027997264) * q * w0 / sin_w0);
    b0 = alpha;
    b1 = Sample(0.0);
    b2 = -alpha;
    a0 = Sample(1.0) + alpha;
    a1 = Sample(-2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;
  }

  Sample process(Sample input)
  {
    Sample output = (b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;

    x2 = x1;
    x1 = input;

    y2 = y1;
    y1 = output;

    if (std::isfinite(output)) return output;
    clear();
    return 0.0;
  }
};

// Numerical Recipes In C p.284. Normalized to [0, 1).
template<typename Sample> class Random {
public:
  uint32_t seed = 0;

  Random(uint32_t seed) : seed(seed) {}

  Sample process()
  {
    seed = 1664525L * seed + 1013904223L;
    return Sample(seed) / UINT32_MAX; // Normalize to [0, 1).
  }
};

enum class CrossoverType { log, linear };

template<typename Sample, size_t maxStack> class WaveString {
public:
  size_t stack = 24;
  Wave1D<Sample, maxStack> wave1d;

  std::array<Sample, maxStack> stringRnd{};
  std::array<KSString<Sample>, maxStack> string;

  std::array<Sample, maxStack> bandpassRnd{};
  std::array<BiquadBandpass<Sample>, maxStack> bandpass;

  void setup(Sample sampleRate)
  {
    wave1d.setup(sampleRate, maxStack, 0.5, 0.5, 0.1);
    for (auto &str : string) str.setup(sampleRate, Sample(100.0), Sample(0.5));
    for (auto &bp : bandpass) bp.setup(sampleRate);
    stringRnd.fill(1);
    bandpassRnd.fill(1);
  }

  void trigger(Random<Sample> &rnd)
  {
    for (auto &random : stringRnd) random = rnd.process();
    for (auto &random : bandpassRnd) random = rnd.process();
  }

  void set(
    size_t stack,
    Sample minFrequency,
    Sample maxFrequency,
    Sample damping,
    Sample pulsePosition,
    Sample pulseWidth,
    Sample decay,
    Sample bandpassQ,
    CrossoverType crossoverType,
    Sample randomAmount)
  {
    this->stack = stack < maxStack ? stack : maxStack;

    wave1d.set(this->stack, damping, pulsePosition, pulseWidth);

    Sample low = 20;
    Sample high = 20;
    for (size_t i = 0; i < this->stack; ++i) {
      string[i].set(
        (Sample(1.0) - randomAmount * stringRnd[i]) * maxFrequency + minFrequency, decay);

      high = getCrossoverFrequency(20, 20000, i + 1, this->stack, crossoverType);
      bandpass[i].setCutoffQ(
        low + (high - low) * (Sample(1.0) - randomAmount * bandpassRnd[i]), bandpassQ);
      low = high;
    }
  }

  void reset()
  {
    wave1d.reset();
    for (auto &str : string) str.reset();
    for (auto &bp : bandpass) bp.reset();
  }

  Sample getCrossoverFrequency(
    Sample low, Sample high, Sample index, Sample length, CrossoverType type)
  {
    return type == CrossoverType::linear
      ? low + (high - low) * index / length
      : someexp<Sample>(
        somelog<Sample>(high / low) * index / length + somelog<Sample>(low));
  }

  Sample process(Sample input)
  {
    wave1d.process(input);
    Sample output = 0;
    Sample denom = stack * 1024;
    for (size_t i = 0; i < stack; ++i) {
      const auto rendered = string[i].process(bandpass[i].process(wave1d[i]));
      wave1d[i] += rendered / denom;
      output += rendered;
    }
    return output;
  }
};

template<typename Sample> class WaveHat {
public:
  static const size_t maxStack = 64;
  static const size_t maxCymbal = 4;

  size_t nCymbal = 0;
  Sample distance = 100;
  std::array<WaveString<Sample, maxStack>, maxCymbal> string;

  void setup(Sample sampleRate)
  {
    for (auto &str : string) str.setup(sampleRate);
  }

  void trigger(Random<Sample> &rnd)
  {
    for (size_t i = 0; i < nCymbal; ++i) string[i].trigger(rnd);
  }

  void set(
    size_t nCymbal,
    size_t stack,
    Sample minFrequency,
    Sample maxFrequency,
    Sample distance,
    Sample damping,
    Sample pulsePosition,
    Sample pulseWidth,
    Sample decay,
    Sample bandpassQ,
    CrossoverType crossoverType,
    Sample randomAmount)
  {
    this->nCymbal = nCymbal > maxCymbal ? maxCymbal : nCymbal;
    this->distance = distance;

    for (size_t i = 0; i < nCymbal; ++i) {
      string[i].set(
        stack, minFrequency, maxFrequency, damping, pulsePosition, pulseWidth, decay,
        bandpassQ, crossoverType, randomAmount);
    }
  }

  void reset()
  {
    for (auto &str : string) str.reset();
  }

  void collide(Wave1D<Sample, maxStack> &w1, Wave1D<Sample, maxStack> &w2)
  {
    for (size_t i = 0; i < w1.length; ++i) {
      const auto intersection = w1[i] - w2[i] + distance / Sample(1024);
      if (intersection < 0) w1[i] = -w1[i];
    }
  }

  Sample process(Sample input, bool collision = true)
  {
    Sample output = 0;
    for (size_t i = 0; i < nCymbal; ++i) output += string[i].process(input);

    if (collision) {
      size_t end = nCymbal - 1;
      for (size_t i = 0; i < end; ++i) collide(string[i].wave1d, string[i + 1].wave1d);
    }

    return output / nCymbal;
  }
};

template<typename Sample> class Comb {
public:
  void setup(Sample sampleRate, Sample time, Sample gain, Sample feedback)
  {
    this->gain = gain;
    this->feedback = feedback;
    delay.setup(sampleRate, time, 0.4);
  }

  // random is in [0, 1].
  void trigger(Sample random) { this->random = random; }

  void set(Sample timeSec, Sample gain, Sample feedback, Sample randomAmount)
  {
    this->gain = gain;
    this->feedback = feedback;
    interpDelayTime.push(timeSec * (Sample(1.0) - randomAmount * random));
  }

  void reset()
  {
    delay.reset();
    buf = 0;
  }

  Sample process(Sample input)
  {
    delay.setTime(interpDelayTime.process());
    input -= feedback * buf;
    buf = delay.process(input);
    return gain * input;
  }

protected:
  Sample random = 0;
  Sample buf = 0;
  Sample gain = 0;
  Sample feedback = 0;
  LinearSmoother<Sample> interpDelayTime;
  Delay<Sample> delay;
};

template<typename Sample> class Excitor {
public:
  Excitor() {}

  void setup(Sample sampleRate)
  {
    for (auto &cmb : comb)
      cmb.setup(sampleRate, Sample(0.002), -Sample(1.0), Sample(1.0));
  }

  void trigger(Random<Sample> &rnd)
  {
    for (auto &cmb : comb) cmb.trigger(rnd.process());
  }

  void set(Sample pickCombTime, Sample pickCombFB, Sample randomAmount)
  {
    for (auto &cmb : comb) cmb.set(pickCombTime, -Sample(1.0), pickCombFB, randomAmount);
  }

  Sample process(Sample input)
  {
    for (auto &cmb : comb) input = cmb.process(input);
    return input;
  }

protected:
  std::array<Comb<Sample>, 8> comb;
};

template<typename Sample> class Pulsar {
public:
  Sample sampleRate = 44100;
  Sample tick = 0;
  Sample phase = 0;

  Pulsar(Sample sampleRate, Sample frequency)
    : sampleRate(sampleRate), tick(frequency / sampleRate)
  {
  }

  void setFrequency(Sample hz) { tick = hz / sampleRate; }

  void reset()
  {
    tick = 0;
    phase = 0;
  }

  Sample process()
  {
    phase += tick;
    if (phase >= Sample(1.0)) {
      phase -= Sample(1.0);
      return Sample(1.0);
    }
    return 0;
  }
};

template<typename Sample> class VelvetNoise {
public:
  VelvetNoise(Sample sampleRate, Sample density, uint32_t seed)
    : sampleRate(sampleRate), rng(seed)
  {
    setDensity(density);
  }

  // Average distance in samples between impulses.
  void setDensity(Sample density) { tick = rng.process() * density / sampleRate; }

  Sample process()
  {
    phase += tick;
    if (phase < Sample(1)) return 0;
    phase -= Sample(1);
    return Sample(2) * someround<Sample>(rng.process()) - Sample(1);
  }

  Sample sampleRate = 44100;

  Sample phase = 0;
  Sample tick = 0;
  Random<Sample> rng{0};
};

// This class outputs direct current.
// RNG algorithm is from Numerical Recipes In C p.284.
template<typename Sample> class Brown {
public:
  int32_t seed;
  Sample drift = 1.0 / 16.0; // Range [0.0, 1.0].

  Brown(Sample seed) : seed(seed) {}

  Sample process()
  {
    if (drift < 1e-5) return 0;
    Sample output;
    do {
      seed = 1664525L * seed + 1013904223L;
      const Sample rnd
        = (Sample)seed / ((Sample)INT32_MAX + Sample(1.0)); // Normalize to [-1, 1).
      output = last + rnd * drift;
    } while (somefabs<Sample>(output) > Sample(1.0));
    last = output;
    return output;
  }

private:
  Sample last = 0.0;
};

} // namespace SomeDSP
