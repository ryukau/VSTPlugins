// (c) 2019 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

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

  static void setTime(Sample seconds)
  {
    timeInSamples = seconds * sampleRate;
    if (timeInSamples < 1.0) timeInSamples = 1.0;
  }

  void push(Sample newTarget)
  {
    target = newTarget;
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
  Sample value = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template<typename Sample> Sample LinearSmoother<Sample>::sampleRate = 44100.0;
template<typename Sample> Sample LinearSmoother<Sample>::timeInSamples = 0.0;

} // namespace SomeDSP
