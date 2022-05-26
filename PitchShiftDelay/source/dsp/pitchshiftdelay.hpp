// (c) 2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>

namespace SomeDSP {

template<typename Sample> struct PitchShiftDelayReader {
  Sample rptr0 = 0;
  Sample rptr1 = 0;

  void setup(size_t bufSize)
  {
    rptr0 = 0;
    rptr1 = Sample(bufSize / 2);
  }

  void reset(size_t wptr, size_t bufSize)
  {
    rptr0 = Sample(wptr);

    auto mid = wptr + bufSize / 2;
    rptr1 = mid < bufSize ? Sample(mid) : Sample(mid - bufSize);
  }

  // buf.size() must be less than 2^24 for single precision.
  Sample process(Sample wptr, Sample pitch, std::vector<Sample> &buf)
  {
    Sample bufSize = Sample(buf.size());

    rptr0 = std::fmod(rptr0 + pitch, bufSize);
    rptr1 = std::fmod(rptr1 + pitch, bufSize);

    auto i0 = size_t(rptr0);
    auto i1 = size_t(rptr1);

    size_t j0 = i0 - 1;
    size_t j1 = i1 - 1;

    if (j0 >= buf.size()) j0 += buf.size(); // Unsigned negative overflow case.
    if (j1 >= buf.size()) j1 += buf.size(); // Unsigned negative overflow case.

    auto frac0 = rptr0 - Sample(i0);
    auto frac1 = rptr1 - Sample(i1);

    auto v0 = buf[i0] + frac0 * (buf[j0] - buf[i0]);
    auto v1 = buf[i1] + frac1 * (buf[j1] - buf[i1]);

    auto amp = std::fabs(Sample(2) * (wptr - rptr0) / bufSize);
    if (amp > Sample(1)) amp = Sample(2) - amp;

    return v1 + amp * (v0 - v1);
  }
};

/*
amp = std::fabs(2 * (wptr - rptr) / bufSize);
if (amp > 1) amp = 2 - amp;
 */

// TODO: multi tap
// TODO: antialiasing
template<typename Sample> class PitchShiftDelay {
private:
  size_t wptr = 0;
  PitchShiftDelayReader<Sample> reader;
  std::vector<Sample> buf;

public:
  void setup(Sample sampleRate, Sample maxTime)
  {
    auto &&size = size_t(sampleRate * maxTime) + 2;
    buf.resize(size < 4 ? 4 : size);

    reader.setup(Sample(buf.size()));

    reset();
  }

  void reset()
  {
    wptr = 0;
    reader.reset(wptr, buf.size());
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  // `pitch` is a multiplier relative to input pitch.
  Sample process(Sample input, Sample pitch)
  {
    buf[wptr] = input;
    if (++wptr >= buf.size()) wptr -= buf.size();

    return reader.process(wptr, pitch, buf);
  }
};

} // namespace SomeDSP
