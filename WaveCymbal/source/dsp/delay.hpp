// (c) 2019-2020 Takamitsu Endo
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

#include <vector>

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
private:
  Sample rFraction = 0;
  Sample w1 = 0;
  int wptr = 0;
  int rptr = 0;
  std::vector<Sample> buf;

public:
  Sample sampleRate = 44100;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = Sample(2) * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1, 0);

    setTime(time);
  }

  void setTime(Sample seconds)
  {
    auto timeInSample = std::clamp<Sample>(sampleRate * seconds, 0, buf.size());

    int timeInt = int(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr < 0) rptr += int(buf.size());
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), Sample(0));
    w1 = 0;
  }

  Sample process(const Sample input)
  {
    const int size = int(buf.size());

    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
    if (++wptr >= size) wptr -= size;

    buf[wptr] = input;
    if (++wptr >= size) wptr -= size;

    w1 = input;

    // Read from buffer.
    const int i1 = rptr;
    if (++rptr >= size) rptr -= size;

    const int i0 = rptr;
    if (++rptr >= size) rptr -= size;

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

} // namespace SomeDSP
