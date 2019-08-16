// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"

#define _USE_MATH_DEFINES
#include <cmath>

// Maps a value in [0, 1] to [min, max].
// min /= max.
template <class T>
class LinearScale {
public:
  LinearScale(T min, T max)
  {
    set(min, max);
  }

  void set(T min, T max)
  {
    this->min = min;
    this->max = max;
    scale = max - min;
  }

  T map(T input) const
  {
    T value = input * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T invmap(T input) const
  {
    T value = (input - min) / scale;
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  T scale;
  T min;
  T max;
};

// min /= max. power > 0.
template <class T>
class SPolyScale {
public:
  SPolyScale(T min, T max, T power = 2.0)
  {
    set(min, max, power);
  }

  void set(T min, T max, T power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = 1.0 / power;
    scale = max - min;
  }

  T map(T input) const
  {
    if (input < 0.0) return min;
    if (input > 1) return max;
    T value = input <= 0.5 ? 0.5 * pow(2.0 * input, power)
                           : 1.0 - 0.5 * pow(2.0 - 2.0 * input, power);
    return value * scale + min;
  }

  T invmap(T input) const
  {
    if (input < min) return 0.0;
    if (input > max) return 1.0;
    T value = (input - min) / scale;
    return input <= 0.5 ? 0.5 * pow(2.0 * value, powerInv)
                        : 1.0 - 0.5 * pow(2.0 - 2.0 * value, powerInv);
  }

protected:
  T scale;
  T min;
  T max;
  T power;
  T powerInv;
};

// map(inValue) == outValue.
// min > max, inValue > 0, outValue > min.
template <class T>
class LogScale {
public:
  LogScale(T min, T max, T inValue = 0.5, T outValue = 0.1)
  {
    set(min, max, inValue, outValue);
  }

  void set(T min, T max, T inValue, T outValue)
  {
    this->min = min;
    this->max = max;

    scale = abs(max - min);
    expo = log(abs(outValue - min) / scale) / log(inValue);
    expoInv = 1.0 / expo;
  }

  T map(T input) const
  {
    T value = pow(input, expo) * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T reverseMap(T input) const
  {
    T value = pow(1.0 - input, expo) * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T invmap(T input) const
  {
    T value = pow((input - min) / scale, expoInv);
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  T scale;
  T expo;
  T expoInv;
  T min;
  T max;
};
