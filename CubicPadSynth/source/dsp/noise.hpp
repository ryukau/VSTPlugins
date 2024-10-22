// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../lib/vcl.hpp"

#include <cstdint>

namespace SomeDSP {

// Numerical Recipes In C p.284. Normalized to [0, 1).
template<typename Sample> class Random {
public:
  uint32_t seed = 0;

  Random() {}
  Random(uint32_t seed) : seed(seed) {}

  Sample process()
  {
    seed = 1664525L * seed + 1013904223L;
    return seed / Sample(UINT32_MAX); // Normalize to [0, 1).
  }
};

struct alignas(64) White16 {
  Vec16ui buffer{0};

  White16(uint32_t seed) { setSeed(seed); }

  void setSeed(uint32_t seed)
  {
    for (int idx = 0; idx < 16; ++idx) {
      seed = 1664525L * seed + 1013904223L;
      buffer.insert(idx, seed);
    }
  }

  Vec16f process()
  {
    buffer = 1664525L * buffer + 1013904223L;
    return to_float(buffer) / float(UINT32_MAX);
  }
};

} // namespace SomeDSP
