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

#include "constants.hpp"
#include "somemath.hpp"

namespace SomeDSP {

// Maps a value in [0, 1] to [min, max].
// min < max.
template<typename Sample> class LinearScale {
public:
  LinearScale(Sample min, Sample max) { set(min, max); }

  void set(Sample min, Sample max)
  {
    this->min = min;
    this->max = max;
    scale = (max - min);
  }

  Sample map(Sample input) const
  {
    Sample value = input * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  Sample invmap(Sample input) const
  {
    Sample value = (input - min) / scale;
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  Sample scale;
  Sample min;
  Sample max;
};

// min < max. power > 0.
template<typename Sample> class SPolyScale {
public:
  SPolyScale(Sample min, Sample max, Sample power = 2.0) { set(min, max, power); }

  void set(Sample min, Sample max, Sample power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = 1.0 / power;
    scale = (max - min);
  }

  Sample map(Sample input) const
  {
    if (input < 0.0) return min;
    if (input > 1) return max;
    Sample value = input <= 0.5 ? 0.5 * somepow<Sample>(2.0 * input, power)
                                : 1.0 - 0.5 * somepow<Sample>(2.0 - 2.0 * input, power);
    return value * scale + min;
  }

  Sample invmap(Sample input) const
  {
    if (input < min) return 0.0;
    if (input > max) return 1.0;
    Sample value = (input - min) / scale;
    return input <= 0.5 ? 0.5 * somepow<Sample>(2.0 * value, powerInv)
                        : 1.0 - 0.5 * somepow<Sample>(2.0 - 2.0 * value, powerInv);
  }

protected:
  Sample scale;
  Sample min;
  Sample max;
  Sample power;
  Sample powerInv;
};

// Based on superellipse. min < max. power > 0.
template<typename Sample> class EllipticScale {
public:
  EllipticScale(Sample min, Sample max, Sample power = 2) { set(min, max, power); }

  void set(Sample min, Sample max, Sample power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = 1.0 / power;
    scale = (max - min);
  }

  Sample map(Sample value) const
  {
    if (value < 0.0) return min;
    if (value > 1) return max;
    value = value <= 0.5
      ? 0.5 * (1.0 - somepow<Sample>(somecos<Sample>(value * pi), power))
      : 0.5 + 0.5 * somepow<Sample>(somecos<Sample>((1.0 - value) * pi), power);
    return value * scale + min;
  }

  Sample invmap(Sample value) const
  {
    if (value < min) return 0.0;
    if (value > max) return 1.0;
    value = (value - min) / scale;
    return value <= 0.5
      ? someacos<Sample>(somepow<Sample>(1.0 - value * 2.0, powerInv)) / pi
      : 1.0 - someacos<Sample>(somepow<Sample>(2.0 * value - 1.0, powerInv)) / pi;
  }

protected:
  Sample scale;
  Sample min;
  Sample max;
  Sample power;
  Sample powerInv;
};

// map(inValue) == outValue.
// min < max, inValue > 0, outValue > min.
template<typename Sample> class LogScale {
public:
  LogScale(Sample min, Sample max, Sample inValue = 0.5, Sample outValue = 0.1)
  {
    set(min, max, inValue, outValue);
  }

  void set(Sample min, Sample max, Sample inValue, Sample outValue)
  {
    this->min = min;
    this->max = max;
    scale = max - min;
    expo = somelog<Sample>((outValue - min) / scale) / somelog<Sample>(inValue);
    expoInv = 1.0 / expo;
  }

  Sample map(Sample input) const
  {
    Sample value = somepow<Sample>(input, expo) * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  Sample reverseMap(Sample input) const { return map(1.0 - input); }

  Sample invmap(Sample input) const
  {
    Sample value = somepow<Sample>((input - min) / scale, expoInv);
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  Sample scale;
  Sample expo;
  Sample expoInv;
  Sample min;
  Sample max;
};

} // namespace SomeDSP
