// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <climits>
#include <vector>

namespace SomeDSP {

// 2x oversampled delay with feedback.
template<typename Sample> class Delay {
public:
  Sample w1 = 0;
  Sample r1 = 0;
  Sample rFraction = 0.0;
  int wptr = 0;
  int rptr = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    auto size = int(Sample(2) * sampleRate * maxTime) + 1;
    buf.resize(size < 0 ? 4 : size);

    setTime(sampleRate, time);
    reset();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), 0);
    w1 = 0;
    r1 = 0;
  }

  void setTime(Sample sampleRate, Sample seconds)
  {
    Sample timeInSample
      = std::clamp<Sample>(Sample(2) * sampleRate * seconds, 0, buf.size());

    int timeInt = int(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr < 0) rptr += int(buf.size());
  }

  Sample process(Sample input, Sample feedback)
  {
    input += feedback * r1;

    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
    ++wptr;
    if (wptr >= int(buf.size())) wptr -= int(buf.size());

    buf[wptr] = input;
    ++wptr;
    if (wptr >= int(buf.size())) wptr -= int(buf.size());

    w1 = input;

    // Read from buffer.
    const size_t i1 = rptr;
    ++rptr;
    if (rptr >= int(buf.size())) rptr -= int(buf.size());

    const size_t i0 = rptr;
    ++rptr;
    if (rptr >= int(buf.size())) rptr -= int(buf.size());

    return r1 = buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

} // namespace SomeDSP
