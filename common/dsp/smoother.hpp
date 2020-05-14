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

// PID controller without I and D.
template<typename Sample> class PController {
public:
  Sample kp; // In [0, 1].
  Sample value = 0;

  // Lower bound of cutoffHz is around 3 to 4 Hz for single presision (float).
  static Sample cutoffToP(Sample sampleRate, Sample cutoffHz)
  {
    auto omega_c = Sample(twopi) * cutoffHz / sampleRate;
    auto y = Sample(1) - somecos<Sample>(omega_c);
    return -y + somesqrt<Sample>((y + Sample(2)) * y);
  }

  void setP(Sample p) { kp = std::clamp<Sample>(p, Sample(0), Sample(1)); };
  void reset(Sample value = 0) { this->value = value; }
  Sample process(Sample input) { return value += kp * (input - value); }
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

template<typename Sample> class SmootherCommon {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  static void setTime(Sample seconds)
  {
    timeInSamples = seconds * sampleRate;
    kp = PController<double>::cutoffToP(
      sampleRate, std::clamp<double>(1.0 / seconds, 0.0, sampleRate / 2.0));
  }
  static void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }

  static Sample sampleRate;
  static Sample timeInSamples;
  static Sample kp;
  static Sample bufferSize;
};

template<typename Sample> Sample SmootherCommon<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample SmootherCommon<Sample>::timeInSamples = 0.0;
template<typename Sample> Sample SmootherCommon<Sample>::kp = 1.0;
template<typename Sample> Sample SmootherCommon<Sample>::bufferSize = 44100.0;

template<typename Sample> class ExpSmoother {
public:
  Sample value = 0;
  Sample target = 0;

  inline Sample getValue() { return value; }
  void reset(Sample value = 0) { this->value = value; }
  void push(Sample newTarget) { target = newTarget; }
  Sample process() { return value += SmootherCommon<Sample>::kp * (target - value); }
};

/**
Legacy smoother for LightPadSynth or earlier plugins. Use ExpSmoother instead.

I tried more faster and stable implementation of LinearSmoother. But ended up this slow
and unstable implementation. This is because some host changes the size of buffer for each
processing call, and faster implementation produces noise in such situation.

I observed FL Studio 20.6 was using variable size buffer. Probably there are more hosts
doing this.
 */
template<typename Sample> class LinearSmoother {
public:
  using Common = SmootherCommon<Sample>;

  inline Sample getValue() { return value; }
  virtual void refresh() { push(target); }

  void reset(Sample value)
  {
    this->value = value;
    target = value;
  }

  void push(Sample newTarget)
  {
    target = newTarget;
    if (Common::timeInSamples < Common::bufferSize) {
      value = target;
      ramp = 0;
    } else {
      ramp = (target - value) / Common::timeInSamples;
    }
  }

  Sample process()
  {
    value += ramp;
    if (somefabs<Sample>(value - target) < Sample(1e-5)) value = target;
    return value;
  }

protected:
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template<typename Sample> class LinearSmootherLocal {
public:
  using Common = SmootherCommon<Sample>;

  void setSampleRate(Sample sampleRate, Sample time = 0.04)
  {
    this->sampleRate = sampleRate;
    setTime(time);
  }

  void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  void setBufferSize(Sample bufferSize) { this->bufferSize = bufferSize; }
  void reset(Sample value) { this->value = target = value; }
  void refresh() { push(target); }
  inline Sample getValue() { return value; }

  void push(Sample newTarget)
  {
    target = newTarget;
    if (Common::timeInSamples < Common::bufferSize) {
      value = target;
      ramp = 0;
    } else {
      ramp = (target - value) / Common::timeInSamples;
    }
  }

  Sample process()
  {
    value += ramp;
    if (somefabs<Sample>(value - target) < Sample(1e-5)) value = target;
    return value;
  }

protected:
  Sample sampleRate = 44100;
  Sample timeInSamples = -1;
  Sample bufferSize = 0;
  Sample target = 1.0;
  Sample ramp = 0.0;
  Sample value = 0;
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

} // namespace SomeDSP
