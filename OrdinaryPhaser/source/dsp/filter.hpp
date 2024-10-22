// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"

namespace SomeDSP {

// TODO: Provide information.
template<typename Sample> class ZDFOnePoleAllpass {
private:
  Sample s = 0;
  Sample out = 0;

public:
  void reset()
  {
    s = 0;
    out = 0;
  }

  Sample output() { return out; }

  // `cutoff` is normalized in [0, 1), where 1 is Nyquist frequency.
  Sample process(Sample x0, Sample cutoff)
  {
    auto xs = x0 - s;
    s += xs * Sample(2) * cutoff / (Sample(1.0 / pi) + cutoff);
    return out = s - xs;
  }
};

template<typename Sample> class Delay {
public:
  int wptr = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto size = size_t(sampleRate * maxTime) + 2;
    buf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  Sample process(Sample input, Sample timeInSample)
  {
    const int size = int(buf.size());

    // Set delay time.
    Sample clamped = std::clamp(timeInSample, Sample(0), Sample(size - 1));
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

} // namespace SomeDSP
