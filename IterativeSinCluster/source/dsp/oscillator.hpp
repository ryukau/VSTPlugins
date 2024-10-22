// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/vcl.hpp"
#include "../../../lib/vcl/vectormath_trig.h"

#include <array>
#include <cmath>

namespace SomeDSP {

template<size_t size> struct alignas(64) BiquadOsc {
public:
  std::array<Vec16f, size> frequency;
  std::array<Vec16f, size> gain;
  std::array<Vec16f, size> u1;
  std::array<Vec16f, size> u0;
  std::array<Vec16f, size> k;

  void setup(float sampleRate)
  {
    for (size_t i = 0; i < size; ++i) {
      u1[i] = 0;
      auto omega = float(twopi) * frequency[i] / sampleRate;
      u0[i] = -sincos(&k[i], omega);
      k[i] *= 2.0f;
    }
  }

  float process()
  {
    float sum = 0;
    for (size_t i = 0; i < size; ++i) {
      auto out = k[i] * u1[i] - u0[i];
      u0[i] = u1[i];
      u1[i] = out;
      sum += horizontal_add(gain[i] * out);
    }
    return sum / (8 * size);
  }
};

} // namespace SomeDSP
