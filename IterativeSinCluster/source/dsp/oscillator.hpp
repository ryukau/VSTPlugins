// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/somemath.hpp"
#include "../../../lib/vcl/vectorclass.h"
#include "../../../lib/vcl/vectormath_trig.h"

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
