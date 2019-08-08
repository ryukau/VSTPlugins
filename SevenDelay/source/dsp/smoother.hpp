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

template <typename Sample>
class LinearSmoother {
public:
  static void setSampleRate(Sample _sampleRate, Sample time = 0.04)
  {
    sampleRate = _sampleRate;
    setTime(time);
  }

  // set time in seconds.
  static void setTime(Sample time)
  {
    timeInSamples = time * sampleRate;
  }

  void push(Sample target)
  {
    this->target = target;
    if (timeInSamples > 1.0)
      ramp = (target - current) / timeInSamples;
    else
      ramp = target - current;
  }

  Sample process()
  {
    if (current == target) return current;
    current += ramp;

    auto diff = current - target;
    if (diff < 0) diff = -diff;
    if (diff < 1e-5) current = target;
    return current;
  }

  static Sample sampleRate;
  static Sample timeInSamples;
  Sample current = 1.0;
  Sample target = 1.0;
  Sample ramp = 0.0;
};

template <typename Sample>
Sample LinearSmoother<Sample>::sampleRate = 44100.0;
template <typename Sample>
Sample LinearSmoother<Sample>::timeInSamples = 0.0;
