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
#include <limits>
#include <numeric>
#include <random>

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

/**
PitchShiftDelay is a traditional time domain pitch shifter with variable delay time.

Internally, it's computing decimation and expansion. So it's better to oversample for
anti-aliasing. When oversampling ratio is `M`, then max pitch multiplier is `2 * M - 1`.

At the start of sound, PitchShiftDelay introduces delay.

Internally, it has 2 read pointers (`rptr0` and `rptr1` in `processs()`). One read pointer
is half the delay time behind to the other read pointer (`rptr1 = rptr0 - delayTime / 2`,
without wrap around). Crossfade is used to smooth the output of those 2 read pointers
(`amp` in `process()`).
*/
template<typename Sample> class PitchShiftDelay {
public:
  static constexpr size_t minSize = 4;

  EMAHighpass<Sample, 1> highpass;
  size_t wptr = 0;
  Sample phase = 0;
  std::vector<Sample> buf;

  PitchShiftDelay() : buf(minSize) {}

  // bufferSize must be less than 2^24 for single precision float.
  void setup(size_t bufferSize)
  {
    buf.resize(bufferSize < minSize ? minSize : bufferSize);
    reset();
  }

  void reset()
  {
    highpass.reset();
    wptr = 0;
    phase = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  void syncPhase(Sample target, Sample kp)
  {
    auto d1 = target - phase;
    if (d1 < 0) {
      auto d2 = d1 + Sample(1);
      phase += kp * (d2 < -d1 ? d2 : d1);
    } else {
      auto d2 = d1 - Sample(1);
      phase += kp * (-d2 < d1 ? d2 : d1);
    }
  }

  // `pitch` is a multiplier relative to input pitch. It can be negative number.
  Sample process(
    Sample input, Sample feedback, Sample highpassKp, Sample pitch, Sample timeInSample)
  {
    // Write to buffer.
    buf[wptr] = input + highpass.process(feedback, highpassKp);
    if (++wptr >= buf.size()) wptr -= buf.size();

    // Read from buffer.
    Sample bufSize = Sample(buf.size());

    auto delayTime = std::clamp(timeInSample, Sample(0), Sample(buf.size() - 1));

    if (delayTime >= std::numeric_limits<Sample>::epsilon()) {
      phase -= (pitch - Sample(1)) / delayTime;
      phase -= std::floor(phase);
    }
    auto rptr0 = std::fmod(Sample(wptr) - delayTime * phase, bufSize);
    if (rptr0 < 0) rptr0 += bufSize;

    auto ph1 = phase + Sample(0.5);
    ph1 -= std::floor(ph1);
    auto rptr1 = std::fmod(Sample(wptr) - delayTime * ph1, bufSize);
    if (rptr1 < 0) rptr1 += bufSize;

    auto i0 = size_t(rptr0);
    auto i1 = size_t(rptr1);

    size_t j0 = i0 - 1;
    size_t j1 = i1 - 1;

    if (j0 >= buf.size()) j0 += buf.size(); // Unsigned negative overflow case.
    if (j1 >= buf.size()) j1 += buf.size(); // Unsigned negative overflow case.

    auto frac0 = rptr0 - Sample(i0);
    auto frac1 = rptr1 - Sample(i1);

    auto v0 = buf[j0] + frac0 * (buf[i0] - buf[j0]);
    auto v1 = buf[j1] + frac1 * (buf[i1] - buf[j1]);

    auto amp = Sample(2) * (phase <= Sample(0.5) ? phase : Sample(1) - phase);

    return v1 + amp * (v0 - v1);
  }
};

} // namespace SomeDSP
