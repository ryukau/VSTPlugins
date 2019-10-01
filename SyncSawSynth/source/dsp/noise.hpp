// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>

#include "somemath.hpp"

namespace SomeDSP {

// Numerical Recipes In C p.284.
template<typename Sample> class White {
public:
  int32_t seed;

  White(int32_t seed) : seed(seed) {}

  Sample process()
  {
    seed = 1664525L * seed + 1013904223L;
    return (Sample)seed / ((Sample)INT32_MAX + 1.0); // Normalize to [-1, 1).
  }
};

// Paul Kellet's refined method in Allan's analysis.
// http://www.firstpr.com.au/dsp/pink-noise/
template<typename Sample> class Pink {
public:
  Pink(int32_t seed) : rng(seed) {}

  Sample process()
  {
    const Sample gain = 0.125;
    auto white = rng.process();
    b0 = 0.99886 * b0 + white * 0.0555179 * gain;
    b1 = 0.99332 * b1 + white * 0.0750759 * gain;
    b2 = 0.96900 * b2 + white * 0.1538520 * gain;
    b3 = 0.86650 * b3 + white * 0.3104856 * gain;
    b4 = 0.55000 * b4 + white * 0.5329522 * gain;
    b5 = -0.7616 * b5 - white * 0.0168980 * gain;
    auto pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362 * gain;
    b6 = white * 0.115926;
    return pink;
  }

private:
  White<Sample> rng;
  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample b3 = 0.0;
  Sample b4 = 0.0;
  Sample b5 = 0.0;
  Sample b6 = 0.0;
};

} // namespace SomeDSP
