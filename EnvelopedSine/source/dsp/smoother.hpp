// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "somemath.hpp"

#include <array>

namespace SomeDSP {

template<typename Sample> class LinearSmoother {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  static void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  static void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }

  virtual void push(Sample newTarget)
  {
    target = newTarget;
    if (timeInSamples < bufferSize)
      value = target;
    else
      ramp = (target - value) / timeInSamples;
  }

  inline Sample getValue() { return value; }

  virtual Sample process()
  {
    if (value == target) return value;
    value += ramp;

    auto diff = value - target;
    if (somefabs<Sample>(diff) < 1e-5) value = target;
    return value;
  }

protected:
  static Sample sampleRate;
  static Sample timeInSamples;
  static Sample bufferSize;
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template<typename Sample> Sample LinearSmoother<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample LinearSmoother<Sample>::timeInSamples = 0.0;
template<typename Sample> Sample LinearSmoother<Sample>::bufferSize = 44100.0;

// Unlike LinearSmoother, value is normalized in [0, 1].
template<typename Sample> class RotarySmoother : public LinearSmoother<Sample> {
public:
  void setRange(Sample max) { this->max = max; }

  void push(Sample newTarget) override
  {
    this->target = newTarget;
    if (this->timeInSamples < this->bufferSize) {
      this->value = this->target;
      return;
    }

    auto dist1 = this->target - this->value;

    if (dist1 < 0) {
      auto dist2 = this->target + max - this->value;
      if (somefabs<Sample>(dist1) > dist2) {
        this->ramp = dist2 / this->timeInSamples;
        return;
      }
    } else {
      auto dist2 = this->target - max - this->value;
      if (dist1 > somefabs<Sample>(dist2)) {
        this->ramp = dist2 / this->timeInSamples;
        return;
      }
    }
    this->ramp = dist1 / this->timeInSamples;
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

} // namespace SomeDSP
