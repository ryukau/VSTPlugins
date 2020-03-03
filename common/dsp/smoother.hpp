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

#include "somemath.hpp"

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

  static Sample sampleRate;
  static Sample timeInSamples;
  static Sample bufferSize;
};

template<typename Sample> Sample SmootherCommon<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample SmootherCommon<Sample>::timeInSamples = 0.0;
template<typename Sample> Sample SmootherCommon<Sample>::bufferSize = 44100.0;

template<typename Sample> class LinearSmoother {
public:
  virtual void push(Sample newTarget)
  {
    target = newTarget;
    if (SmootherCommon<Sample>::timeInSamples < SmootherCommon<Sample>::bufferSize)
      value = target;
    else
      ramp = (target - value) / SmootherCommon<Sample>::timeInSamples;
  }

  virtual inline Sample getValue() { return value; }
  virtual void reset(Sample value) { this->value = value; }
  virtual void refresh() { push(target); }

  virtual Sample process()
  {
    if (value == target) return value;
    value += ramp;

    auto diff = value - target;
    if (somefabs<Sample>(diff) < Sample(1e-5)) value = target;
    return value;
  }

protected:
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template<typename Sample> class LinearSmootherLocal {
public:
  void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  void setBufferSize(Sample bufferSize) { this->bufferSize = bufferSize; }

  void reset(Sample value)
  {
    this->value = target = value;
    ramp = 0;
  }

  void refresh() { push(target); }

  void push(Sample newTarget)
  {
    target = newTarget;
    if (timeInSamples < bufferSize)
      value = target;
    else
      ramp = (target - value) / timeInSamples;
  }

  inline Sample getValue() { return value; }

  Sample process()
  {
    if (value == target) return value;
    value += ramp;

    auto diff = value - target;
    if (diff < 0) diff = -diff;
    if (diff < 1e-5) value = target;
    return value;
  }

protected:
  Sample sampleRate = 44100;
  Sample timeInSamples = -1;
  Sample bufferSize = 0;
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

class alignas(64) LinearSmoother16 {
public:
  void push(
    float v0,
    float v1,
    float v2,
    float v3,
    float v4,
    float v5,
    float v6,
    float v7,
    float v8,
    float v9,
    float v10,
    float v11,
    float v12,
    float v13,
    float v14,
    float v15)
  {
    target = Vec16f(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
    if (SmootherCommon<float>::timeInSamples < SmootherCommon<float>::bufferSize)
      value = target;
    else
      ramp = (target - value) / SmootherCommon<float>::timeInSamples;
  }

  void push(Vec16f newTarget)
  {
    target = newTarget;
    if (SmootherCommon<float>::timeInSamples < SmootherCommon<float>::bufferSize)
      value = target;
    else
      ramp = (target - value) / SmootherCommon<float>::timeInSamples;
  }

  inline Vec16f getValue() { return value; }
  float operator[](const int index) { return value[index]; }
  void reset(Vec16f value) { this->value = value; }
  void reset(int index, float value) { this->value.insert(index, value); }

  Vec16f process()
  {
    value = select(value == target, value, value + ramp);
    value = select(abs(value - target) < float(1e-5), target, value);
    return value;
  }

protected:
  Vec16f value = 1.0;
  Vec16f target = 1.0;
  Vec16f ramp = 0.0;
};

// Unlike LinearSmoother, value is normalized in [0, 1].
template<typename Sample> class RotarySmoother : public LinearSmoother<Sample> {
public:
  void setRange(Sample max) { this->max = max; }

  void push(Sample newTarget) override
  {
    this->target = newTarget;
    if (SmootherCommon<Sample>::timeInSamples < SmootherCommon<Sample>::bufferSize) {
      this->value = this->target;
      return;
    }

    auto dist1 = this->target - this->value;

    if (dist1 < 0) {
      auto dist2 = this->target + max - this->value;
      if (somefabs<Sample>(dist1) > dist2) {
        this->ramp = dist2 / SmootherCommon<Sample>::timeInSamples;
        return;
      }
    } else {
      auto dist2 = this->target - max - this->value;
      if (dist1 > somefabs<Sample>(dist2)) {
        this->ramp = dist2 / SmootherCommon<Sample>::timeInSamples;
        return;
      }
    }
    this->ramp = dist1 / SmootherCommon<Sample>::timeInSamples;
  }

  Sample process() override
  {
    if (this->value == this->target) return this->value;
    this->value += this->ramp;
    this->value -= max * somefloor<Sample>(this->value / max);

    auto diff = this->value - this->target;
    if (somefabs<Sample>(diff) < 1e-5) this->value = this->target;
    return this->value;
  }

private:
  Sample max = 1;
};

// PID controller without I and D.
template<typename Sample> class PController {
public:
  // p in [0, 1].
  void setup(Sample sampleRate, Sample p)
  {
    this->sampleRate = sampleRate;
    kp = p;
  };

  void reset() { value = 0; }

  Sample process(Sample input)
  {
    value += kp * (input - value);
    return value;
  }

private:
  Sample sampleRate = 44100;
  Sample kp;
  Sample value = 0;
};

class PController16 {
public:
  // p in [0, 1].
  void setP(float p) { kp = p; };
  void setP(int index, float p) { kp.insert(index, p); };

  void reset() { value = 0; }

  Vec16f process(Vec16f input)
  {
    value += kp * (input - value);
    return value;
  }

private:
  Vec16f kp = 1;
  Vec16f value = 0;
};

} // namespace SomeDSP
