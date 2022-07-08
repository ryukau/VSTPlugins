// (c) 2022 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "svf.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>

namespace SomeDSP {

template<typename Sample> class DoubleEMAFilterKp {
private:
  Sample v1 = 0;
  Sample v2 = 0;

public:
  void reset(Sample value = 0)
  {
    v1 = value;
    v2 = value;
  }

  Sample process(Sample input, Sample kp)
  {
    auto &&v0 = input;
    v1 += kp * (v0 - v1);
    v2 += kp * (v1 - v2);
    return v2;
  }
};

template<typename Sample> class RateLimiter {
private:
  Sample target = 0;
  Sample value = 0;

public:
  inline Sample getValue() { return value; }

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

template<typename Sample> class Delay {
public:
  size_t wptr = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto &&size = size_t(sampleRate * maxTime) + 2;
    buf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  Sample process(Sample input, Sample timeInSample)
  {
    // Set delay time.
    Sample clamped = std::clamp(timeInSample, Sample(0), Sample(buf.size() - 1));
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
If `length` is too long, compiler might silently fail to allocate stack.
*/
template<typename Sample, size_t length> class FeedbackDelayNetwork {
private:
  std::array<std::array<Sample, length>, length> matrix{};
  std::array<std::array<Sample, length>, 2> buf{};
  std::array<Delay<Sample>, length> delay;

  size_t bufIndex = 0;

public:
  Sample rate = Sample(1);
  std::array<RateLimiter<Sample>, length> delayTimeSample;
  // std::array<Sample, length> lowpassKp{};
  std::array<SVF<Sample, 0>, length> lowpass;
  std::array<SVF<Sample, 2>, length> highpass;

  /**
  If `identityAmount` is close to 0, then the result becomes close to identity matrix.

  This algorithm is ported from `scipy.stats.ortho_group` in SciPy v1.8.0.
  */
  void randomOrthogonal(unsigned seed, Sample identityAmount)
  {
    pcg64 rng{};
    rng.seed(seed);
    std::normal_distribution<Sample> dist{}; // mean 0, stddev 1.

    matrix.fill({});
    for (size_t i = 0; i < length; ++i) matrix[i][i] = Sample(1);

    std::array<Sample, length> x;
    for (size_t n = 0; n < length; ++n) {
      auto xRange = length - n;
      // for (size_t i = 0; i < xRange; ++i) x[i] = dist(rng);
      x[0] = Sample(1);
      for (size_t i = 1; i < xRange; ++i) x[i] = identityAmount * dist(rng);

      Sample norm2 = 0;
      for (size_t i = 0; i < xRange; ++i) norm2 += x[i] * x[i];

      Sample x0 = x[0];

      Sample D = x0 >= 0 ? Sample(1) : Sample(-1);
      x[0] += D * std::sqrt(norm2);

      Sample denom = std::sqrt((norm2 - x0 * x0 + x[0] * x[0]) / Sample(2));
      for (size_t i = 0; i < xRange; ++i) x[i] /= denom;

      for (size_t row = 0; row < length; ++row) {
        Sample dotH = 0;
        for (size_t col = 0; col < xRange; ++col) dotH += matrix[col][row] * x[col];
        for (size_t col = 0; col < xRange; ++col) {
          matrix[col][row] = D * (matrix[col][row] - dotH * x[col]);
        }
      }
    }
  }

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &dl : delay) dl.setup(sampleRate, maxTime);

    // Slightly below nyquist to prevent blow up.
    for (auto &lp : lowpass) lp.setup(Sample(0.499), Sample(0.5));
    for (auto &hp : highpass) hp.setup(Sample(5) / sampleRate, Sample(0.5));

    reset();
  }

  void reset()
  {
    buf.fill({});
    for (auto &dl : delay) dl.reset();
    for (auto &lp : lowpass) lp.reset();
    for (auto &hp : highpass) hp.reset();
  }

  Sample process(Sample input, Sample feedback)
  {
    bufIndex ^= 1;
    auto &front = buf[bufIndex];
    auto &back = buf[bufIndex ^ 1];
    front.fill(0);
    for (size_t i = 0; i < length; ++i) {
      for (size_t j = 0; j < length; ++j) front[i] += matrix[i][j] * back[j];
    }

    for (size_t idx = 0; idx < length; ++idx) {
      auto crossed = front[idx];
      auto sig = input + feedback * crossed;
      auto delayed = delay[idx].process(sig, delayTimeSample[idx].process(rate));
      auto lowpassed = lowpass[idx].process(delayed);
      front[idx] = highpass[idx].process(lowpassed);
    }

    return std::accumulate(front.begin(), front.end(), Sample(0));
  }
};

template<typename Sample> class NoteGate {
private:
  Sample signal = 0;
  DoubleEMAFilter<Sample> filter;

public:
  void reset()
  {
    signal = Sample(1);
    filter.reset(Sample(1));
    filter.kp = Sample(0);
  }

  void prepare(Sample sampleRate, Sample seconds)
  {
    if (seconds < std::numeric_limits<Sample>::epsilon())
      filter.kp = Sample(1);
    else
      filter.setCutoff(sampleRate, Sample(1) / seconds);
  }

  void release() { signal = Sample(0); }

  Sample process() { return filter.process(signal); }
};

} // namespace SomeDSP
