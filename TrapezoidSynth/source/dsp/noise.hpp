// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <cstdint>

namespace SomeDSP {

// Numerical Recipes In C p.284. Normalized to [0, 1).
template<typename Sample> class Random {
public:
  uint32_t seed;

  Random(uint32_t seed) : seed(seed) {}

  Sample process()
  {
    seed = 1664525L * seed + 1013904223L;
    return Sample(seed) / UINT32_MAX; // Normalize to [0, 1).
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
    return Sample(2) * std::round(rng.process()) - Sample(1);
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
  uint32_t seed;
  Sample drift = 1.0 / 16.0; // Range [0.0, 1.0].

  Brown(uint32_t seed) : seed(seed) {}

  Sample process()
  {
    if (drift < 1e-5) return 0;
    Sample output;
    do {
      seed = 1664525L * seed + 1013904223L;
      const Sample rnd = (Sample)((int32_t)seed)
        / ((Sample)INT32_MAX + Sample(1)); // Normalize to [-1, 1).
      output = last + rnd * drift;
    } while (std::fabs(output) > Sample(1));
    last = output;
    return output;
  }

private:
  Sample last = 0;
};

} // namespace SomeDSP
