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

#include <algorithm>

#include "constants.hpp"
#include "somemath.hpp"

namespace SomeDSP {

// If there are elements of 0, 1, 2, then max is 2.
template<typename T> class IntScale {
public:
  IntScale(uint32_t max) : max(max) {}
  uint32_t map(T input) const { return uint32_t(std::min<T>(max, input * (max + 1))); }
  uint32_t reverseMap(T input) const { return map(T(1.0) - input); }
  T invmap(uint32_t input) const { return input / T(max); }
  T getMin() { return T(0); }
  T getMax() { return T(max); }

protected:
  const uint32_t max;
};

// Maps a value in [0, 1] to [min, max].
// min < max.
template<typename T> class LinearScale {
public:
  LinearScale(T min, T max) { set(min, max); }

  void set(T min, T max)
  {
    this->min = min;
    this->max = max;
    scale = (max - min);
  }

  T map(T input) const
  {
    T value = input * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    T value = (input - min) / scale;
    if (value < T(0.0)) return T(0.0);
    if (value > T(1.0)) return T(1.0);
    return value;
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T min;
  T max;
};

// map(inValue) == outValue.
// min < max, inValue > 0, outValue > min.
template<typename T> class LogScale {
public:
  LogScale(T min, T max, T inValue = T(0.5), T outValue = T(0.1))
  {
    set(min, max, inValue, outValue);
  }

  void set(T min, T max, T inValue, T outValue)
  {
    this->min = min;
    this->max = max;
    scale = max - min;
    expo = somelog<T>((outValue - min) / scale) / somelog<T>(inValue);
    expoInv = T(1.0) / expo;
  }

  T map(T input) const
  {
    if (input < T(0.0)) return min;
    if (input > T(1.0)) return max;
    T value = pow(input, expo) * scale + min;
    return value;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    if (input < min) return T(0.0);
    if (input > max) return T(1.0);
    T value = pow((input - min) / scale, expoInv);
    return value;
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T expo;
  T expoInv;
  T min;
  T max;
};

} // namespace SomeDSP
