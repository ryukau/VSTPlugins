// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <array>
#include <numeric>

namespace SomeDSP {

template<typename Sample, size_t nCascade> class EMAHighpass {
private:
  std::array<Sample, nCascade> v1{};

public:
  void reset(Sample value = 0) { v1.fill(value); }

  Sample process(Sample input, Sample kp)
  {
    Sample v0 = input;
    for (size_t idx = 0; idx < nCascade; ++idx) {
      v1[idx] += kp * (v0 - v1[idx]);
      v0 = v1[idx];
    }
    return input - v0;
  }
};

template<typename Sample> class RateLimiter {
private:
  Sample target = 0;
  Sample value = 0;

public:
  void reset(Sample value = 0)
  {
    this->value = value;
    this->target = value;
  }

  void push(Sample target) { this->target = target; }

  Sample process(Sample rate)
  {
    auto diff = target - value;
    if (diff > rate) {
      value += rate;
    } else if (diff < -rate) {
      value -= rate;
    } else {
      value = target;
    }
    return value;
  }
};

template<typename Sample, size_t nValue> class ParallelCombSmoother {
private:
  std::array<RateLimiter<Sample>, nValue> limiter;
  std::array<EMAFilter<Sample>, nValue> lowpass;

public:
  void resetAt(size_t index, Sample value = 0)
  {
    limiter[index].reset(value);
    lowpass[index].reset(value);
  }

  void pushAt(size_t index, Sample target) { limiter[index].push(target); }
  Sample at(size_t index) { return lowpass[index].value; }

  void process(Sample rate, Sample kp)
  {
    for (size_t idx = 0; idx < nValue; ++idx) {
      lowpass[idx].kp = kp;
      lowpass[idx].process(limiter[idx].process(rate));
    }
  }
};

template<typename Sample, size_t nTap> class ParallelComb {
private:
  size_t wptr = 0;
  std::vector<Sample> buf;

public:
  ParallelCombSmoother<Sample, nTap> time;

  ParallelComb() : buf(4) {}

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto &&size = size_t(sampleRate * maxTime) + 1;
    buf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset()
  {
    wptr = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  Sample process(Sample input, Sample rate, Sample kp)
  {
    if (++wptr >= buf.size()) wptr -= buf.size();
    buf[wptr] = input;

    time.process(rate, kp);

    Sample output = Sample(0);
    for (size_t idx = 0; idx < nTap; ++idx) {
      Sample clamped = std::clamp(time.at(idx), Sample(0), Sample(buf.size() - 1));
      size_t timeInt = size_t(clamped);
      Sample fraction = clamped - Sample(timeInt);

      size_t rptr0 = wptr - timeInt;
      size_t rptr1 = rptr0 - 1;
      if (rptr0 >= buf.size()) rptr0 += buf.size(); // Unsigned negative overflow case.
      if (rptr1 >= buf.size()) rptr1 += buf.size(); // Unsigned negative overflow case.

      output -= buf[rptr0] + fraction * (buf[rptr1] - buf[rptr0]);
    }
    return output;
  }
};

} // namespace SomeDSP
