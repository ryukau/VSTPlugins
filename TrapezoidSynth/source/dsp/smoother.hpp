// (c) 2019 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace SomeDSP {

/**
LinearSmoother is used for generic parameter smoothing.

LinearSmootherLocal is used for specific parameter smoothing that should have independent
smoothing time. Pitch slide is an example use case.
*/
template<typename Sample> class LinearSmoother {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  static void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  static void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }

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

template<typename Sample> class LinearSmootherLocal {
public:
  void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  void setTime(Sample seconds) { timeInSamples = seconds * sampleRate; }
  void setBufferSize(Sample _bufferSize) { bufferSize = _bufferSize; }

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

// PID controller without I and D.
template<typename Sample> class PController {
public:
  Sample sampleRate = 44100;

  PController(Sample kp) : kp(kp) {}

  void reset() { value = 0; }

  Sample process(Sample input)
  {
    value += kp * (input - value) / sampleRate;
    return value;
  }

private:
  Sample kp;
  Sample value = 0;
};

} // namespace SomeDSP
