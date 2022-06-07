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

#include "constants.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace SomeDSP {

// Exponential moving average filter.
template<typename Sample> class EMAFilter {
public:
  Sample kp = Sample(1); // In [0, 1].
  Sample value = 0;

  // Lower bound of cutoffHz is around 3 to 4 Hz for single presision (float).
  static Sample cutoffToP(Sample sampleRate, Sample cutoffHz)
  {
    auto omega_c = Sample(twopi) * cutoffHz / sampleRate;
    auto y = Sample(1) - std::cos(omega_c);
    return -y + std::sqrt((y + Sample(2)) * y);
  }

  void setCutoff(Sample sampleRate, Sample cutoffHz)
  {
    kp = Sample(EMAFilter<double>::cutoffToP(sampleRate, cutoffHz));
  }

  void setP(Sample p) { kp = std::clamp<Sample>(p, Sample(0), Sample(1)); };
  void reset(Sample value = 0) { this->value = value; }
  Sample process(Sample input) { return value += kp * (input - value); }
};

template<typename Sample> class DoubleEMAFilter {
private:
  Sample kp = Sample(1);
  Sample v1 = 0;
  Sample v2 = 0;

public:
  void reset(Sample value = 0)
  {
    v1 = value;
    v2 = value;
  }

  void setMin(Sample value)
  {
    v1 = std::min(v1, value);
    v2 = std::min(v2, value);
  }

  void setCutoff(Sample sampleRate, Sample cutoffHz)
  {
    kp = cutoffHz >= sampleRate / Sample(2)
      ? Sample(1)
      : Sample(EMAFilter<double>::cutoffToP(sampleRate, cutoffHz));
  }

  Sample process(Sample input)
  {
    auto &&v0 = input;
    v1 += kp * (v0 - v1);
    v2 += kp * (v1 - v2);
    return v2;
  }
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
    kp = Sample(EMAFilter<double>::cutoffToP(
      sampleRate, std::clamp<double>(1.0 / seconds, 0.0, sampleRate / 2.0)));
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
    if (std::fabs(value - target) < Sample(1e-5)) value = target;
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
  void reset(Sample value) { this->value = target = value; }
  void refresh() { push(target); }
  inline Sample getValue() { return value; }

  void push(Sample newTarget)
  {
    target = newTarget;
    if (timeInSamples < Common::bufferSize) {
      value = target;
      ramp = 0;
    } else {
      ramp = (target - value) / timeInSamples;
    }
  }

  Sample process()
  {
    value += ramp;
    if (std::fabs(value - target) < Sample(1e-5)) value = target;
    return value;
  }

protected:
  Sample sampleRate = 44100;
  Sample timeInSamples = -1;
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
      if (std::fabs(dist1) > dist2) {
        this->ramp = dist2 / Common::timeInSamples;
        return;
      }
    } else {
      auto dist2 = this->target - max - this->value;
      if (dist1 > std::fabs(dist2)) {
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
    this->value -= max * std::floor(this->value / max);

    auto diff = this->value - this->target;
    if (std::fabs(diff) < 1e-5) this->value = this->target;
    return this->value;
  }

private:
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
  Sample max = 1;
};

} // namespace SomeDSP
