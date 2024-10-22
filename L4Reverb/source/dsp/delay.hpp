// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <vector>

namespace SomeDSP {

// 2x oversampled delay.
template<typename Sample> class Delay {
public:
  Sample w1 = 0;
  Sample rFraction = 0.0;
  int wptr = 0;
  int rptr = 0;
  int size = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample maxTime)
  {
    size = int(Sample(2) * sampleRate * maxTime) + 1;
    if (size < 4) size = 4;

    buf.resize(size);

    reset();
  }

  void reset()
  {
    w1 = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  Sample process(Sample input, Sample sampleRate, Sample seconds)
  {
    // Set delay time.
    Sample timeInSample
      = std::clamp<Sample>(Sample(2) * sampleRate * seconds, Sample(0), Sample(size));

    int timeInt = int(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr < 0) rptr += size;

    // Write to buffer.
    buf[wptr] = Sample(0.5) * (input + w1);
    ++wptr;
    if (wptr >= size) wptr -= size;

    buf[wptr] = input;
    ++wptr;
    if (wptr >= size) wptr -= size;

    w1 = input;

    // Read from buffer.
    const size_t i1 = rptr;
    ++rptr;
    if (rptr >= size) rptr -= size;

    const size_t i0 = rptr;
    ++rptr;
    if (rptr >= size) rptr -= size;

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

/**
Allpass filter with arbitrary length delay.
https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
*/
template<typename Sample> class LongAllpass {
public:
  Sample buffer = 0;
  Delay<Sample> delay;

  void setup(Sample sampleRate, Sample maxTime) { delay.setup(sampleRate, maxTime); }

  void reset()
  {
    buffer = 0;
    delay.reset();
  }

  // gain in [0, 1].
  Sample process(Sample input, Sample sampleRate, Sample seconds, Sample gain)
  {
    input -= gain * buffer;
    auto output = buffer + gain * input;
    buffer = delay.process(input, sampleRate, seconds);
    return output;
  }
};

template<typename Sample, size_t nest> class NestedLongAllpass {
public:
  std::array<ExpSmoother<Sample>, nest> seconds{};
  std::array<ExpSmoother<Sample>, nest> innerFeed{};
  std::array<ExpSmoother<Sample>, nest> outerFeed{};

  std::array<Sample, nest> in{};
  std::array<Sample, nest> buffer{};
  std::array<LongAllpass<Sample>, nest> allpass;

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &ap : allpass) ap.setup(sampleRate, maxTime);
  }

  void reset()
  {
    in.fill(0);
    buffer.fill(0);
    for (auto &ap : allpass) ap.reset();
  }

  Sample process(Sample input, Sample sampleRate)
  {
    for (size_t idx = 0; idx < nest; ++idx) {
      input -= outerFeed[idx].process() * buffer[idx];
      in[idx] = input;
    }

    Sample out = in.back();
    for (size_t idx = nest - 1; idx != size_t(-1); --idx) {
      auto apOut = allpass[idx].process(
        out, sampleRate, seconds[idx].process(), innerFeed[idx].process());
      out = buffer[idx] + outerFeed[idx].getValue() * in[idx];
      buffer[idx] = apOut;
    }

    return out;
  }
};

#define NESTED_ALLPASS(NAME, CHILD)                                                      \
  template<typename Sample, size_t nest> class NAME {                                    \
  public:                                                                                \
    std::array<Sample, nest> in{};                                                       \
    std::array<Sample, nest> buffer{};                                                   \
    std::array<ExpSmoother<Sample>, nest> feed;                                          \
    std::array<CHILD<Sample, nest>, nest> allpass;                                       \
                                                                                         \
    void setup(Sample sampleRate, Sample maxTime)                                        \
    {                                                                                    \
      for (auto &ap : allpass) ap.setup(sampleRate, maxTime);                            \
    }                                                                                    \
                                                                                         \
    void reset()                                                                         \
    {                                                                                    \
      in.fill(0);                                                                        \
      buffer.fill(0);                                                                    \
      for (auto &ap : allpass) ap.reset();                                               \
    }                                                                                    \
                                                                                         \
    Sample process(Sample input, Sample sampleRate)                                      \
    {                                                                                    \
      for (size_t idx = 0; idx < nest; ++idx) {                                          \
        input -= feed[idx].process() * buffer[idx];                                      \
        in[idx] = input;                                                                 \
      }                                                                                  \
                                                                                         \
      Sample out = in.back();                                                            \
      for (size_t idx = nest - 1; idx != size_t(-1); --idx) {                            \
        auto apOut = allpass[idx].process(out, sampleRate);                              \
        out = buffer[idx] + feed[idx].getValue() * in[idx];                              \
        buffer[idx] = apOut;                                                             \
      }                                                                                  \
                                                                                         \
      return out;                                                                        \
    }                                                                                    \
  };

NESTED_ALLPASS(NestD2, NestedLongAllpass)
NESTED_ALLPASS(NestD3, NestD2)
NESTED_ALLPASS(NestD4, NestD3)

} // namespace SomeDSP
