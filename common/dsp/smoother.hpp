// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../lib/vcl/vectorclass.h"

#include "constants.hpp"
#include "somemath.hpp"

#include <algorithm>
#include <array>

namespace SomeDSP {

template<typename Sample> class SmootherCommon {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  static void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  static void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }
  static void setBufferIndex(Sample index) { ratio = index / bufferSize; }

  static Sample sampleRate;
  static Sample timeInSamples;
  static Sample bufferSize;
  static Sample ratio;
};

template<typename Sample> Sample SmootherCommon<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample SmootherCommon<Sample>::timeInSamples = 0.0;
template<typename Sample> Sample SmootherCommon<Sample>::bufferSize = 44100.0;
template<typename Sample> Sample SmootherCommon<Sample>::ratio = 0.0;

template<typename Sample> class LinearSmoother {
public:
  using Common = SmootherCommon<Sample>;

  virtual inline Sample getValue() { return value; }
  virtual void reset(Sample value) { this->value = v0 = v1 = value; }
  virtual void refresh() { push(target); }

  virtual void push(Sample newTarget)
  {
    target = newTarget;
    v1 = v0;
    v0 = (Common::timeInSamples >= Common::bufferSize)
        && (somefabs<Sample>(v0 - newTarget) >= 1e-5)
      ? (newTarget - v0) * Common::bufferSize / Common::timeInSamples + v0
      : newTarget;
  }

  Sample process() { return value = v1 + Common::ratio * (v0 - v1); }

protected:
  Sample target = 1.0;
  Sample value = 1.0;
  Sample v0 = 1;
  Sample v1 = 1;
};

template<typename Sample> class LinearSmootherLocal {
public:
  void setSampleRate(Sample sampleRate, Sample time = 0.04)
  {
    this->sampleRate = sampleRate;
    setTime(time);
  }

  void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  void setBufferSize(Sample bufferSize) { this->bufferSize = bufferSize; }
  void reset(Sample value) { this->value = v0 = v1 = value; }
  void refresh() { push(target); }
  inline Sample getValue() { return value; }

  void push(Sample newTarget)
  {
    target = newTarget;
    v1 = v0;
    v0 = (timeInSamples >= bufferSize) && (somefabs<Sample>(v0 - newTarget) >= 1e-5)
      ? (newTarget - v0) * bufferSize / timeInSamples + v0
      : newTarget;
  }

  virtual Sample process(Sample index)
  {
    return value = v1 + index / bufferSize * (v0 - v1);
  }

protected:
  Sample sampleRate = 44100;
  Sample timeInSamples = -1;
  Sample bufferSize = 0;
  Sample target = 1.0;
  Sample v0 = 1;
  Sample v1 = 1;
  Sample value = 0;
};

// This can be used when terminated value must be equal to target value.
template<typename Sample>
class LinearSmootherExactLocal : public LinearSmootherLocal<Sample> {
public:
  Sample process(Sample index) override
  {
    if (somefabs<Sample>(this->value - this->target) < 1e-5)
      return this->value = this->target;
    return LinearSmootherLocal<Sample>::process(index);
  }
};

// TODO: test.
class alignas(64) LinearSmoother16 {
public:
  using Common = SmootherCommon<float>;

  void push(
    float f0,
    float f1,
    float f2,
    float f3,
    float f4,
    float f5,
    float f6,
    float f7,
    float f8,
    float f9,
    float f10,
    float f11,
    float f12,
    float f13,
    float f14,
    float f15)
  {
    target = Vec16f(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);
    push(target);
  }

  void push(Vec16f newTarget)
  {
    target = newTarget;
    v1 = v0;
    v0 = (Common::timeInSamples >= Common::bufferSize)
      ? (target - v0) * Common::bufferSize / Common::timeInSamples + v0
      : target;
    select(abs(v0 - target) >= 1e-5, v0, target);
  }

  inline Vec16f getValue() { return value; }
  float operator[](const int index) { return value[index]; }
  void reset(Vec16f value) { this->value = v0 = v1 = value; }
  void reset(int index, float value) { this->value.insert(index, value); }

  Vec16f process() { return value = v1 + Common::ratio * (v0 - v1); }

protected:
  Vec16f target = 1.0;
  Vec16f value = 1.0;
  Vec16f v0 = 1;
  Vec16f v1 = 1;
};

// Unlike LinearSmoother, value is normalized in [0, 1].
template<typename Sample> class RotarySmoother {
public:
  using Common = SmootherCommon<Sample>;

  inline Sample getValue() { return value; }
  void reset(Sample value) { this->value = value; }
  void refresh() { push(target); }
  void setRange(Sample max) { this->max = max; }

  void push(Sample newTarget)
  {
    this->target = newTarget;
    if (Common::timeInSamples < Common::bufferSize) {
      this->value = this->target;
      return;
    }

    auto dist1 = this->target - this->value;

    if (dist1 < 0) {
      auto dist2 = this->target + max - this->value;
      if (somefabs<Sample>(dist1) > dist2) {
        this->ramp = dist2 / Common::timeInSamples;
        return;
      }
    } else {
      auto dist2 = this->target - max - this->value;
      if (dist1 > somefabs<Sample>(dist2)) {
        this->ramp = dist2 / Common::timeInSamples;
        return;
      }
    }
    this->ramp = dist1 / Common::timeInSamples;
  }

  Sample process()
  {
    if (this->value == this->target) return this->value;
    this->value += this->ramp;
    this->value -= max * somefloor<Sample>(this->value / max);

    auto diff = this->value - this->target;
    if (somefabs<Sample>(diff) < 1e-5) this->value = this->target;
    return this->value;
  }

private:
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
  Sample max = 1;
};

// PID controller without I and D.
template<typename Sample> class PController {
public:
  // Lower bound of cutoffHz is around 3 to 4 Hz for single presision (float).
  static Sample cutoffToP(Sample sampleRate, Sample cutoffHz)
  {
    auto omega_c = Sample(twopi) * cutoffHz / sampleRate;
    auto y = Sample(1) - somecos<Sample>(omega_c);
    return -y + somesqrt<Sample>((y + Sample(2)) * y);
  }

  void setP(Sample p) { kp = std::clamp<Sample>(p, Sample(0), Sample(1)); };
  void reset() { value = 0; }
  Sample process(Sample input) { return value += kp * (input - value); }

private:
  Sample kp; // In [0, 1].
  Sample value = 0;
};

class PController16 {
public:
  void setP(float p) { kp = std::clamp<float>(p, float(0), float(1)); };
  void setP(int index, float p) { kp.insert(index, p); };
  void reset() { value = 0; }
  Vec16f process(Vec16f input) { return value += kp * (input - value); }

private:
  Vec16f kp = 1; // In [0, 1].
  Vec16f value = 0;
};

} // namespace SomeDSP
