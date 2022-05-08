// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"

#include <array>
#include <numeric>
#include <random>

namespace SomeDSP {

template<typename Sample> class DoubleEMAFilter {
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

template<typename Sample> class EMAHighpass {
private:
  Sample v1 = 0;

public:
  void reset(Sample value = 0) { v1 = value; }

  Sample process(Sample input, Sample kp)
  {
    v1 += kp * (input - v1);
    return input - v1;
  }
};

template<typename Sample> class RateLimiter {
private:
  Sample target = 0;
  Sample value = 0;

public:
  static Sample rate;

  void reset(Sample value = 0)
  {
    this->value = value;
    this->target = value;
  }

  void push(Sample target) { this->target = target; }

  Sample process()
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

template<typename Sample> Sample RateLimiter<Sample>::rate = 1;

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
  std::array<DoubleEMAFilter<Sample>, length> lowpass;
  std::array<EMAHighpass<Sample>, length> highpass;

  std::array<Sample, length> splitGain;
  size_t cycle = 100000;
  size_t counter = 0;
  size_t bufIndex = 0;

public:
  std::array<RateLimiter<Sample>, length> delayTimeSample;
  std::array<Sample, length> lowpassKp{};
  std::array<Sample, length> highpassKp{};

  /**
  Randomize `matrix` as special orthogonal matrix. The algorithm is ported from
  `scipy.stats.special_ortho_group` in SciPy v1.8.0.
  */
  void randomizeMatrix(unsigned seed = 0)
  {
    using T = Sample;
    constexpr size_t dim = length;

    pcg64 rng{};
    rng.seed(seed);
    std::normal_distribution<T> dist{}; // mean 0, stddev 1.

    auto &H = matrix;
    for (size_t i = 0; i < dim; ++i) {
      for (size_t j = 0; j < dim; ++j) H[i][j] = i == j ? T(1) : T(0);
    }

    std::array<T, dim> x;
    std::array<T, dim> D;
    for (size_t n = 0; n < dim; ++n) {
      auto xRange = dim - n;
      for (size_t i = 0; i < xRange; ++i) x[i] = dist(rng);

      T norm2 = 0;
      for (size_t i = 0; i < xRange; ++i) norm2 += x[i] * x[i];

      T x0 = x[0];

      D[n] = x0 >= 0 ? T(1) : T(-1);
      x[0] += D[n] * std::sqrt(norm2);

      T denom = std::sqrt((norm2 - x0 * x0 + x[0] * x[0]) / T(2));
      for (size_t i = 0; i < xRange; ++i) x[i] /= denom;

      for (size_t row = 0; row < dim; ++row) {
        T dotH = 0;
        for (size_t col = 0; col < xRange; ++col) dotH += H[col][row] * x[col];
        for (size_t col = 0; col < xRange; ++col) H[col][row] -= dotH * x[col];
      }
    }

    size_t back = dim - 1;
    D[back] = (back & 0b1) == 0 ? T(1) : T(-1);
    for (size_t i = 0; i < back; ++i) D[back] *= D[i];

    for (size_t row = 0; row < dim; ++row) {
      for (size_t col = 0; col < dim; ++col) H[col][row] *= D[row];
    }
  }

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &dl : delay) dl.setup(sampleRate, maxTime);

    lowpassKp.fill(Sample(1));
    highpassKp.fill(Sample(0.0006542843087824565)); // 5Hz cutoff when fs=48000Hz.

    reset();
  }

  void prepare(Sample sampleRate, Sample splitRotationHz)
  {
    auto &&inv = Sample(1) / splitRotationHz;
    cycle
      = inv >= Sample(std::numeric_limits<size_t>::max()) ? 1 : size_t(sampleRate * inv);
  }

  void reset()
  {
    buf.fill({});
    for (auto &dl : delay) dl.reset();
    for (auto &lp : lowpass) lp.reset();
    for (auto &hp : highpass) hp.reset();
  }

  /**
  `offset` is normalized phase in [0, 1].
  `skew` >= 0.
  */
  void fillSplitGain(Sample offset, Sample skew)
  {
    for (size_t idx = 0; idx < splitGain.size(); ++idx) {
      auto &&phase = offset + Sample(idx) / Sample(splitGain.size());
      splitGain[idx] = std::exp(skew * std::sin(Sample(twopi) * phase));
    }
    auto sum = std::accumulate(splitGain.begin(), splitGain.end(), Sample(0));
    for (auto &value : splitGain) value /= sum;
  }

  Sample preProcess(Sample splitPhaseOffset, Sample splitSkew)
  {
    if (++counter >= cycle) counter = 0;

    fillSplitGain(splitPhaseOffset + Sample(counter) / Sample(cycle), splitSkew);

    bufIndex ^= 1;
    auto &front = buf[bufIndex];
    auto &back = buf[bufIndex ^ 1];
    front.fill(0);
    for (size_t i = 0; i < length; ++i) {
      for (size_t j = 0; j < length; ++j) front[i] += matrix[i][j] * back[j];
    }
    return std::accumulate(front.begin(), front.end(), Sample(0));
  }

  Sample process(Sample input, Sample crossIn, Sample stereoCross, Sample feedback)
  {
    auto &front = buf[bufIndex];

    crossIn /= -Sample(length);
    for (size_t idx = 0; idx < length; ++idx) {
      auto &&crossed = front[idx] + stereoCross * (crossIn - front[idx]);
      auto &&sig = splitGain[idx] * input + feedback * crossed;
      auto &&delayed = delay[idx].process(sig, delayTimeSample[idx].process());
      auto &&lowpassed = lowpass[idx].process(delayed, lowpassKp[idx]);
      front[idx] = highpass[idx].process(lowpassed, highpassKp[idx]);
    }

    return std::accumulate(front.begin(), front.end(), Sample(0));
  }
};

} // namespace SomeDSP
