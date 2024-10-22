// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample> class Delay {
public:
  int wptr = 0;
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
    const int size = int(buf.size());

    // Set delay time.
    Sample clamped = delayTime.process(
      std::clamp(timeInSample, Sample(0), Sample(size - 1)), rateLimit);
    int timeInt = int(clamped);
    Sample rFraction = clamped - Sample(timeInt);

    int rptr0 = wptr - timeInt;
    if (rptr0 < 0) rptr0 += size;

    int rptr1 = rptr0 - 1;
    if (rptr1 < 0) rptr1 += size;

    // Write to buffer.
    buf[wptr] = input;
    if (++wptr >= size) wptr -= size;

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
