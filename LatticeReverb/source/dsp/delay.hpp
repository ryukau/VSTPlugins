// (c) 2020 Takamitsu Endo
//
// This file is part of LatticeReverb.
//
// LatticeReverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LatticeReverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LatticeReverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <array>
#include <climits>
#include <vector>

#include "../common/dsp/constants.hpp"
#include "../common/dsp/smoother.hpp"

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
    std::fill(buf.begin(), buf.end(), 0);
  }

  Sample process(Sample input, Sample sampleRate, Sample seconds)
  {
    // Set delay time.
    Sample timeInSample = std::clamp<Sample>(Sample(2) * sampleRate * seconds, 0, size);

    int timeInt = int(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr < 0) rptr += size;

    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
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

template<typename Sample> struct LongAllpassData {
  Sample seconds = 0;
  Sample outerFeed = 0; // in [-1, 1].
  Sample innerFeed = 0; // in [-1, 1].
  Sample lowpassKp = 0; // in [ 0, 1].
};

template<typename Sample, size_t nest> class NestedLongAllpass {
public:
  std::array<Sample, nest> in{};
  std::array<Sample, nest> buffer{};
  std::array<LongAllpass<Sample>, nest> allpass;
  std::array<LongAllpassData<Sample>, nest> data;
  std::array<PController<Sample>, nest> lowpass;

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &ap : allpass) ap.setup(sampleRate, maxTime);
  }

  void reset()
  {
    in.fill(0);
    buffer.fill(0);
    for (auto &ap : allpass) ap.reset();
    for (auto &dat : data) dat = {0, 0, 0};
    for (auto &lp : lowpass) lp.reset();
  }

  Sample process(Sample input, Sample sampleRate)
  {
    for (size_t idx = 0; idx < nest; ++idx) {
      input -= data[idx].outerFeed * buffer[idx];
      in[idx] = input;
    }

    Sample out = in.back();
    for (size_t idx = nest - 1; idx < nest; --idx) {
      auto apOut
        = allpass[idx].process(out, sampleRate, data[idx].seconds, data[idx].innerFeed);
      out = buffer[idx] + data[idx].outerFeed * in[idx];

      lowpass[idx].kp = data[idx].lowpassKp;
      buffer[idx] = lowpass[idx].process(apOut);

      // buffer[idx] = apOut;
    }

    return out;
  }
};

template<typename Sample, size_t nest> class StereoLongAllpass {
public:
  NestedLongAllpass<Sample, nest> apL;
  NestedLongAllpass<Sample, nest> apR;

  void setup(Sample sampleRate, Sample maxTime)
  {
    apL.setup(sampleRate, maxTime);
    apR.setup(sampleRate, maxTime);
  }

  void reset()
  {
    apL.reset();
    apR.reset();
  }

  std::array<Sample, 2>
  process(Sample inL, Sample inR, Sample sampleRate, Sample stereoCross = 0.2)
  {
    for (size_t idx = 0; idx < nest; idx += 2) {
      Sample tmpL = apL.buffer[idx];
      apL.buffer[idx] -= stereoCross * (apR.buffer[idx] + apL.buffer[idx]);
      apR.buffer[idx] -= stereoCross * (tmpL + apR.buffer[idx]);
    }

    return {apL.process(inL, sampleRate), apR.process(inR, sampleRate)};
  }
};

} // namespace SomeDSP
