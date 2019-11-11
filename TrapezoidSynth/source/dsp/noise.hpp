// (c) 2019 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

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
