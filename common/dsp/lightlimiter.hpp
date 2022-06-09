// (c) 2020-2022 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "smoother.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>

namespace SomeDSP {

/**
Fixed integer time delay. `length` is delay time in samples.
*/
template<typename Sample, size_t length> class FixedIntDelay {
public:
  std::array<Sample, length> buf{};
  size_t ptr = 0;

  void reset(Sample value = 0)
  {
    ptr = 0;
    buf.fill(value);
  }

  Sample process(Sample input)
  {
    if (++ptr >= buf.size()) ptr = 0;
    auto output = buf[ptr];
    buf[ptr] = input;
    return output;
  }
};

/**
Replacement of std::deque with reduced memory allocation. Buffer is fixed length.
*/
template<typename T, size_t length> struct RingQueueArray {
  std::array<T, length> buf;
  size_t wptr = 0;
  size_t rptr = 0;
  size_t sz = 0;

  void reset(T value = 0)
  {
    sz = 0;
    wptr = 0;
    rptr = 0;
    buf.fill(value);
  }

  inline size_t size() { return sz; }

  inline bool empty() { return sz == 0; }

  T &front() { return buf[increment(rptr, length)]; }
  T &back() { return buf[wptr]; }

  inline size_t increment(size_t idx, size_t wrap)
  {
    if (++idx >= wrap) idx -= wrap;
    return idx;
  }

  inline size_t decrement(size_t idx, size_t wrap)
  {
    if (--idx >= wrap) idx += wrap; // Unsigned negative overflow case.
    return idx;
  }

  void push_back(T value)
  {
    sz = increment(sz, length + 1);
    wptr = increment(wptr, length);
    buf[wptr] = value;
  }

  void pop_front()
  {
    sz = decrement(sz, length + 1);
    rptr = increment(rptr, length);
  }

  T pop_back()
  {
    sz = decrement(sz, length + 1);
    T value = buf[wptr];
    wptr = decrement(wptr, length);
    return value;
  }
};

/**
LightLimiter aims to be used as makeshift or secondary stage limiter. Sometimes it's
convenient to have a quick-to-use limiter when tuning parameters.

- Fixed length attack (or smoothing time).
- No sustain.
- Fast but crude release. 2 cascaded EMA filter.

Template parameter `length` sets the length of internal buffers and smoothing time in
samples. For 48000Hz, setting `length = 64` provides acceptable quality. Shorter `length`
tends to introduce audible artifact when input exceeds threshold.
*/
template<typename Sample, size_t length> class LightLimiter {
private:
  static constexpr Sample thresholdSafeMultiplier
    = Sample(1) - Sample(8) * std::numeric_limits<Sample>::epsilon();
  static constexpr Sample defaultGain = Sample(1);

  Sample thresholdAmp = thresholdSafeMultiplier;
  size_t firIndex = 0;

  FixedIntDelay<Sample, length> buf;
  RingQueueArray<Sample, length> queue;
  DoubleEMAFilter<Sample> releaseFilter;
  std::array<Sample, length> gain;
  std::array<std::array<Sample, length>, length> fir;

public:
  LightLimiter()
  {
    fillFir(Sample(0.5 * pi));
    reset();
  }

  /**
  Fill `fir` with Kaiser window.

  `beta` parameter for kaiser window changes the character of noise.
  Recommend to use one from the following two candidates.

  - `0.5 * pi` : Less low frequency noise, high noise floor.
  - `1.5 * pi` : More low frequency noise, low noise floor.

  On macOS, triangular windows is used instead. This is because Apple Clang doesn't
  provide cmath special functions.
  */
  void fillFir(Sample beta)
  {
    constexpr Sample N = Sample(length - 1);
    for (size_t n = 0; n < fir[0].size(); ++n) {
#ifdef __APPLE__
      // Triangular window where cyl_bessel_i is not available.
      size_t half = length / 2;
      fir[0][n] = n < half ? Sample(n + 1) : Sample(length - n);
#else
      // Kaiser window where cyl_bessel_i is available.
      auto &&A = Sample(2) * Sample(n) / N - Sample(1);
      auto &&value = std::cyl_bessel_i(Sample(0), beta * std::sqrt(Sample(1) - A * A))
        / std::cyl_bessel_i(Sample(0), beta);
      fir[0][n] = Sample(value);
#endif
    }
    auto sum = std::accumulate(fir[0].begin(), fir[0].end(), Sample(0));
    if (sum <= Sample(1e-15)) return; // Avoid zero division.
    for (size_t n = 0; n < fir[0].size(); ++n) fir[0][n] /= Sample(sum);

    // Copy and rotate coefficients .
    for (size_t i = 1; i < fir.size(); ++i) {
      fir[i] = fir[0];
      std::rotate(fir[i].rbegin(), fir[i].rbegin() + i, fir[i].rend());
    }
  }

  size_t latency() { return length; }

  void reset()
  {
    buf.reset(0);
    gain.fill(defaultGain);
    queue.reset(0);
    releaseFilter.reset();
    firIndex = 0;
  }

  void prepare(Sample sampleRate, Sample releaseSeconds, Sample thresholdAmplitude)
  {
    releaseFilter.setCutoff(sampleRate, Sample(1) / releaseSeconds);
    thresholdAmp = thresholdAmplitude * thresholdSafeMultiplier;
  }

  inline Sample applyCharacteristicCurve(Sample x0, Sample thresholdAmp)
  {
    return x0 > thresholdAmp ? thresholdAmp / x0 : Sample(1);
  }

  inline Sample processPeakHold(Sample x0, Sample delayOut)
  {
    while (!queue.empty()) {
      if (queue.back() >= x0) {
        if (delayOut == queue.front()) {
          Sample value = queue.front();
          queue.pop_front();
          queue.push_back(x0);
          return std::max(queue.front(), value);
        }
        queue.push_back(x0);
        return queue.front();
      }
      queue.pop_back();
    }
    queue.push_back(x0);
    return x0;
  }

  inline Sample convolve()
  {
    Sample smoothed = 0;
    for (size_t i = 0; i < length; ++i) smoothed += fir[firIndex][i] * gain[i];
    return smoothed;
  }

  Sample process(Sample input)
  {
    auto delayed = buf.process(input);

    auto peakHold = processPeakHold(std::fabs(input), std::fabs(delayed));
    auto candidate = applyCharacteristicCurve(peakHold, thresholdAmp);

    releaseFilter.setMin(candidate);
    auto &&released = releaseFilter.process(candidate);

    gain[firIndex] = std::min(released, candidate);
    if (++firIndex >= length) firIndex -= length;

    return convolve() * delayed;
  }
};

} // namespace SomeDSP
