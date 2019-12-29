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

// min < max. power > 0.
template<typename T> class SPolyScale {
public:
  SPolyScale(T min, T max, T power = T(2.0)) { set(min, max, power); }

  void set(T min, T max, T power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = T(1.0) / power;
    scale = (max - min);
  }

  T map(T input) const
  {
    if (input < T(0.0)) return min;
    if (input > T(1.0)) return max;
    T value = input <= T(0.5)
      ? T(0.5) * somepow<T>(T(2.0) * input, power)
      : T(1.0) - T(0.5) * somepow<T>(T(2.0) - T(2.0) * input, power);
    return value * scale + min;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    if (input < min) return T(0.0);
    if (input > max) return T(1.0);
    T value = (input - min) / scale;
    return value <= T(0.5)
      ? T(0.5) * somepow<T>(T(2.0) * value, powerInv)
      : T(1.0) - T(0.5) * somepow<T>(T(2.0) - T(2.0) * value, powerInv);
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T min;
  T max;
  T power;
  T powerInv;
};

// Based on superellipse. min < max. power > 0.
template<typename T> class EllipticScale {
public:
  EllipticScale(T min, T max, T power = T(2.0)) { set(min, max, power); }

  void set(T min, T max, T power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = T(1.0) / power;
    scale = (max - min);
  }

  T map(T value) const
  {
    if (value < T(0.0)) return min;
    if (value > T(1.0)) return max;
    value = value <= T(0.5)
      ? T(0.5) * (T(1.0) - somepow<T>(somecos<T>(value * pi), power))
      : T(0.5) + T(0.5) * somepow<T>(somecos<T>((T(1.0) - value) * pi), power);
    return value * scale + min;
  }

  T reverseMap(T input) const { return map(T(T(1.0)) - input); }

  T invmap(T value) const
  {
    if (value < min) return T(0.0);
    if (value > max) return T(1.0);
    value = (value - min) / scale;
    return value <= T(0.5)
      ? someacos<T>(somepow<T>(T(1.0) - value * T(2.0), powerInv)) / pi
      : T(1.0) - someacos<T>(somepow<T>(T(2.0) * value - T(1.0), powerInv)) / pi;
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T min;
  T max;
  T power;
  T powerInv;
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

// Maps value normalized in [0, 1] -> dB -> amplitude.
template<typename T> class DecibelScale {
public:
  DecibelScale(T minDB, T maxDB, bool minToZero) { set(minDB, maxDB, minToZero); }

  void set(T minDB, T maxDB, bool minToZero)
  {
    this->minToZero = minToZero;
    this->minDB = minDB;
    this->maxDB = maxDB;
    this->minAmp = minToZero ? T(0) : dbToAmp(minDB);
    this->maxAmp = dbToAmp(maxDB);
    scaleDB = (maxDB - minDB);
  }

  T map(T normalized)
  {
    if (minToZero && normalized <= T(0)) return T(0);
    T dB = normalized * scaleDB + minDB;
    if (dB < minDB)
      dB = minDB;
    else if (dB > maxDB)
      dB = maxDB;
    return dbToAmp(dB);
  }

  T reverseMap(T input) const { return map(T(1) - input); }

  T invmap(T amplitude)
  {
    if (amplitude <= T(0)) return T(0);
    T normalized = (ampToDB(amplitude) - minDB) / scaleDB;
    if (normalized < T(0)) return T(0);
    if (normalized > T(1)) return T(1);
    return normalized;
  }

  T getMin() { return minAmp; }
  T getMax() { return maxAmp; }

  inline T dbToAmp(T dB) { return somepow<T>(T(10), dB / T(20)); }
  inline T ampToDB(T amplitude) { return T(20) * somelog10<T>(amplitude); }

protected:
  bool minToZero;
  T scaleDB;
  T minDB;
  T maxDB;
  T minAmp;
  T maxAmp;
};

} // namespace SomeDSP
