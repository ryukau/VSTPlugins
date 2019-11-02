// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

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

} // namespace SomeDSP
