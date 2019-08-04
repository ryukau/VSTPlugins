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

// scale() maps [0, 1] to [min, max].
// invScale() maps [min, max] to [0, 1].
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
    scaleFactor = (max - min);
  }

  T map(T input) const
  {
    T value = input * scaleFactor + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T invmap(T input) const
  {
    T value = (input - min) / scaleFactor;
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  T scaleFactor;
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
    scaleFactor = (max - min);
  }

  T map(T input) const
  {
    if (input < 0.0) return min;
    if (input > 1) return max;
    T value = input <= 0.5 ? 0.5 * pow(2.0 * input, power)
                           : 1.0 - 0.5 * pow(2.0 - 2.0 * input, power);
    return value * scaleFactor + min;
  }

  T invmap(T input) const
  {
    if (input < min) return 0.0;
    if (input > max) return 1.0;
    T value = (input - min) / scaleFactor;
    return input <= 0.5 ? 0.5 * pow(2.0 * value, powerInv)
                        : 1.0 - 0.5 * pow(2.0 - 2.0 * value, powerInv);
  }

protected:
  T scaleFactor;
  T min;
  T max;
  T power;
  T powerInv;
};

// Based on superellipse. min /= max. power > 0.
template <class T>
class EllipticScale {
public:
  EllipticScale(T min, T max, T power = 2)
  {
    set(min, max, power);
  }

  void set(T min, T max, T power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = 1.0 / power;
    scaleFactor = (max - min);
  }

  T scale(T value) const
  {
    if (value < 0.0) return min;
    if (value > 1) return max;
    value = value <= 0.5 ? 0.5 * (1.0 - pow(cos(value * M_PI), power))
                         : 0.5 + 0.5 * pow(cos((1.0 - value) * M_PI), power);
    return value * scaleFactor + min;
  }

  T invscale(T value) const
  {
    if (value < min) return 0.0;
    if (value > max) return 1.0;
    value = (value - min) / scaleFactor;
    return value <= 0.5 ? acos(pow(1.0 - value * 2.0, powerInv)) / M_PI
                        : 1.0 - acos(pow(2.0 * value - 1.0, powerInv)) / M_PI;
  }

protected:
  T scaleFactor;
  T min;
  T max;
  T power;
  T powerInv;
};

// scale(inValue) == outValue.
// min /= max, inValue > 0.
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
    scaleFactor = (max - min);
    expo = log((outValue - min) / scaleFactor) / log(inValue);
    expoInv = 1.0 / expo;
  }

  T map(T input) const
  {
    T value = pow(input, expo) * scaleFactor + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T invmap(T input) const
  {
    T value = pow((input - min) / scaleFactor, expoInv);
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }

protected:
  T scaleFactor;
  T expo;
  T expoInv;
  T min;
  T max;
};

namespace Steinberg {
namespace Vst {

template <typename ParameterScale>
class ScaledParameter : public Parameter {
public:
  ScaledParameter(
    const TChar *title,
    ParamID tag,
    ParameterScale &scale,
    ParamValue defaultValue = 0.0,
    const TChar *units = nullptr,
    int32 flags = ParameterInfo::kCanAutomate,
    UnitID unitID = kRootUnitId)
    : Parameter(title, tag, units, defaultValue, 0, flags, unitID), scale(scale)
  {
    precision = 16;
  }

  virtual void toString(ParamValue normalized, String128 string) const SMTG_OVERRIDE
  {
    UString128 wrapper;
    wrapper.printFloat(toPlain(normalized), precision);
    wrapper.copyTo(string, 128);
  }

  virtual bool fromString(const TChar *string, ParamValue &normalized) const SMTG_OVERRIDE
  {
    UString wrapper((TChar *)string, strlen16(string));
    if (wrapper.scanFloat(normalized)) {
      normalized = toNormalized(normalized);
      return true;
    }
    return false;
  }

  virtual ParamValue toPlain(ParamValue normalized) const SMTG_OVERRIDE
  {
    return scale.map(normalized);
  }

  virtual ParamValue toNormalized(ParamValue plain) const SMTG_OVERRIDE
  {
    return scale.invmap(plain);
  }

  OBJ_METHODS(ScaledParameter, Parameter)

protected:
  ParameterScale &scale;
  ParamValue multiplier;
};

} // namespace Vst
} // namespace Steinberg
