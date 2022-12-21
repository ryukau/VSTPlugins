// (c) 2022 Takamitsu Endo
//
// This file is part of LongPhaser.
//
// LongPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LongPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LongPhaser.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample> class Delay {
public:
  size_t wptr = 0;
  RateLimiter<Sample> delayTime;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto size = size_t(sampleRate * maxTime) + 2;
    buf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), Sample(0));
    delayTime.reset();
  }

  Sample process(Sample input, Sample timeInSample, Sample rateLimit)
  {
    // Set delay time.
    Sample clamped = delayTime.process(
      std::clamp(timeInSample, Sample(0), Sample(buf.size() - 1)), rateLimit);
    size_t &&timeInt = size_t(clamped);
    Sample rFraction = clamped - Sample(timeInt);

    size_t rptr0 = wptr - timeInt;
    size_t rptr1 = rptr0 - 1;
    if (rptr0 >= buf.size()) rptr0 += buf.size(); // Unsigned negative overflow case.
    if (rptr1 >= buf.size()) rptr1 += buf.size(); // Unsigned negative overflow case.

    // Write to buffer.
    buf[wptr] = input;
    if (++wptr >= buf.size()) wptr -= buf.size();

    // Read from buffer.
    return buf[rptr0] + rFraction * (buf[rptr1] - buf[rptr0]);
  }
};

/**
Allpass filter with arbitrary length delay.
https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
*/
template<typename Sample> class LongAllpass {
public:
  Sample buffer = 0;
  Sample output = 0;
  Delay<Sample> delay;

  void setup(Sample sampleRate, Sample maxTime) { delay.setup(sampleRate, maxTime); }

  void reset()
  {
    buffer = 0;
    output = 0;
    delay.reset();
  }

  // `feed` is in [0, 1].
  Sample process(Sample input, Sample timeInSample, Sample rateLimit, Sample feed)
  {
    input -= feed * buffer;
    output = buffer + feed * input;
    buffer = delay.process(input, timeInSample, rateLimit);
    return output;
  }
};

} // namespace SomeDSP
