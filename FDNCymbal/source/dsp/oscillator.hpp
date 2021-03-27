// (c) 2019-2020 Takamitsu Endo
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

#pragma once

namespace SomeDSP {

template<typename Sample> class Pulsar {
public:
  Sample sampleRate = 44100;
  Sample tick = 0;
  Sample phase = 0;
  Sample buffer = 0;

  Pulsar(Sample sampleRate, Sample frequency)
    : sampleRate(sampleRate), tick(frequency / sampleRate)
  {
  }

  void setFrequency(Sample hz) { tick = hz / sampleRate; }

  void reset()
  {
    tick = 0;
    phase = 0;
    buffer = 0;
  }

  Sample process()
  {
    phase += tick;
    if (phase >= Sample(1.0)) {
      phase -= Sample(1.0);
      buffer = Sample(-1.0);
      return Sample(1.0);
    }
    Sample output = buffer;
    buffer = 0;
    return output;
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
    return Sample(seed) / Sample(UINT32_MAX); // Normalize to [0, 1).
  }
};

template<typename Sample> class VelvetNoise {
public:
  void setup(Sample sampleRate, Sample density, uint32_t seed)
  {
    this->sampleRate = sampleRate;
    rng.seed = seed;
    setDensity(density);
  }

  // Average distance in samples between impulses.
  void setDensity(Sample density) { tick = rng.process() * density / sampleRate; }

  void reset(uint32_t seed)
  {
    phase = 0;
    tick = 0;
    rng.seed = seed;
  }

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

// Mostly uniform gain range.
// - double : freq > 0.25Hz.
// - float  : freq > 8Hz. Huge bump around 1Hz.
template<typename Sample> class BiquadOsc {
public:
  void setup(Sample sampleRate, Sample frequency)
  {
    this->sampleRate = sampleRate;
    setFrequency(frequency);
  }

  void setFrequency(Sample hz)
  {
    auto omega = Sample(twopi) * hz / sampleRate;
    u1 = 0;
    u0 = -std::sin(omega);
    k = Sample(2) * std::cos(omega);
  }

  void reset()
  {
    u1 = 0;
    u0 = 0;
    k = 0;
  }

  Sample process()
  {
    auto output = k * u1 - u0;
    u0 = u1;
    u1 = output;
    return output;
  }

protected:
  Sample sampleRate = 44100;
  Sample u1 = 0;
  Sample u0 = 0;
  Sample k = 0;
};

} // namespace SomeDSP
