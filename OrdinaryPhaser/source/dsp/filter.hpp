// (c) 2022 Takamitsu Endo
//
// This file is part of OrdinaryPhaser.
//
// OrdinaryPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OrdinaryPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OrdinaryPhaser.  If not, see <https://www.gnu.org/licenses/>.

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
