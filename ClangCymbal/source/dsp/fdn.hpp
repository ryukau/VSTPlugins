// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

template<typename Sample, size_t length> class ParallelDelay {
public:
  std::array<Sample, length> targetTime{};
  std::array<Sample, length> time{};
  std::array<size_t, length> wptr{};
  std::array<std::vector<Sample>, length> buffer;

  Sample rate = Sample(0.25);
  Sample kp = Sample(1);

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto size = size_t(sampleRate * maxTime) + 2;
    for (auto &bf : buffer) bf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset(Sample timeInSample = 0)
  {
    targetTime.fill(timeInSample);
    time.fill(timeInSample);
    for (auto &bf : buffer) std::fill(bf.begin(), bf.end(), Sample(0));
  }

  void setDelayTimeAt(size_t index, Sample sampleRate, Sample overtone, Sample noteFreq)
  {
    constexpr auto eps = std::numeric_limits<Sample>::epsilon();
    overtone = std::max(eps, overtone);
    noteFreq = std::max(eps, noteFreq);
    targetTime[index] = std::clamp(
      sampleRate / overtone / noteFreq, Sample(0), Sample(buffer[index].size() - 1));
  }

  void resetDelayTimeAt(size_t index, Sample sampleRate, Sample overtone, Sample noteFreq)
  {
    setDelayTimeAt(index, sampleRate, overtone, noteFreq);
    time[index] = targetTime[index];
  }

  void process(std::array<Sample, length> &input)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      // Interpolate delay time.
      auto prevTime = time[idx];
      time[idx] += kp * (targetTime[idx] - time[idx]);
      auto diff = time[idx] - prevTime;
      if (diff > rate) {
        time[idx] = prevTime + rate;
      } else if (diff < -rate) {
        time[idx] = prevTime - rate;
      }

      // Set delay time.
      size_t timeInt = size_t(time[idx]);
      Sample rFraction = time[idx] - Sample(timeInt);

      auto &buf = buffer[idx];

      size_t rptr0 = wptr[idx] - timeInt;
      size_t rptr1 = rptr0 - 1;
      if (rptr0 >= buf.size()) rptr0 += buf.size(); // Unsigned negative overflow case.
      if (rptr1 >= buf.size()) rptr1 += buf.size(); // Unsigned negative overflow case.

      // Write to buffer.
      buf[wptr[idx]] = input[idx];
      if (++wptr[idx] >= buf.size()) wptr[idx] -= buf.size();

      // Read from buffer.
      input[idx] = buf[rptr0] + rFraction * (buf[rptr1] - buf[rptr0]);
    }
  }
};

template<typename Sample, size_t length> class FeedbackDelayNetwork {
private:
  std::array<std::array<Sample, length>, length> matrix{};
  std::array<std::array<Sample, length>, 2> buf{};
  size_t bufIndex = 0;

public:
  ParallelDelay<Sample, length> delay;
  ParallelSVF<Sample, length> lowpass;
  ParallelSVF<Sample, length> highpass;

  /**
  If `identityAmount` is close to 0, then the result becomes close to identity matrix.

  This algorithm is ported from `scipy.stats.ortho_group` in SciPy v1.8.0.
  */
  void randomOrthogonal(
    unsigned seed,
    Sample identityAmount,
    Sample ratio,
    const std::vector<std::vector<Sample>> &randomBase)
  {
    pcg64 rng{};
    rng.seed(seed);
    std::normal_distribution<Sample> dist{}; // mean 0, stddev 1.

    matrix.fill({});
    for (size_t i = 0; i < length; ++i) matrix[i][i] = Sample(1);

    std::array<Sample, length> x;
    for (size_t n = 0; n < length; ++n) {
      auto xRange = length - n;

      x[0] = Sample(1);
      for (size_t i = 1; i < xRange; ++i) {
        auto mix = randomBase[n][i] + ratio * (dist(rng) - randomBase[n][i]);
        x[i] = identityAmount * mix;
      }

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
    delay.setup(sampleRate, maxTime);

    // Slightly below nyquist to prevent blow up.
    for (size_t idx = 0; idx < length; ++idx) {
      lowpass.resetCutoffAt(idx, Sample(0.499), Sample(0.5));
      highpass.resetCutoffAt(idx, Sample(5) / sampleRate, Sample(0.5));
    }

    reset();
  }

  void reset()
  {
    buf.fill({});
    delay.reset();
    lowpass.reset();
    highpass.reset();
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

    for (size_t idx = 0; idx < length; ++idx) front[idx] = input + feedback * front[idx];
    delay.process(front);
    lowpass.lowpass(front);
    highpass.highpass(front);

    return std::accumulate(front.begin(), front.end(), Sample(0));
  }
};

template<typename Sample> class Tremolo {
public:
  EMAFilter<Sample> timeOffsetSmoother;
  Delay<Sample> delay;

  void setup(Sample sampleRate, Sample maxDelayTime)
  {
    timeOffsetSmoother.setCutoff(sampleRate, Sample(1));
    delay.setup(sampleRate, maxDelayTime);
  }

  void reset(Sample timeModOffset)
  {
    timeOffsetSmoother.reset(timeModOffset);
    delay.reset();
  }

  Sample process(
    Sample input,
    Sample lfo,
    Sample timeModOffset,
    Sample timeInSample,
    Sample depth,
    Sample mix)
  {
    auto timeMod = lfo * depth / Sample(4) + timeOffsetSmoother.process(timeModOffset);
    auto delayOut = delay.process(input, timeInSample * timeMod);
    auto gain = Sample(1) + depth * (lfo - Sample(1));
    return input + mix * (gain * delayOut - input);
  }
};

} // namespace SomeDSP
