// (c) 2023 Takamitsu Endo
//
// This file is part of GenericDrum.
//
// GenericDrum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenericDrum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenericDrum.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include <algorithm>
#include <array>
#include <complex>
#include <limits>
#include <numbers>
#include <numeric>
#include <random>
#include <vector>

namespace SomeDSP {

template<typename Sample> class ComplexLowpass {
private:
  Sample x1 = 0;
  std::complex<Sample> y1{};
  ExpSmoother<std::complex<Sample>> b{};
  ExpSmoother<std::complex<Sample>> a1{};

  inline Sample setR(Sample cut, Sample lowR, Sample highR, Sample lowCut, Sample highCut)
  {
    if (cut <= lowCut) return lowR;
    if (cut >= highCut) return highR;
    return lowR + (highR - lowR) * (cut - lowCut) / (highCut - lowCut);
  }

public:
  void push(Sample freqNormalized)
  {
    auto cut = std::exp(std::complex<Sample>{
      Sample(0), Sample(2) * std::numbers::pi_v<Sample> * freqNormalized});
    auto R = setR(freqNormalized, Sample(0.25), Sample(0.9), Sample(0.01), Sample(0.2));
    a1.push(cut * std::pow(R, cut.imag()));
    b.push((Sample(1) - a1.target) / Sample(2));
  }

  /* freqNormalized in [0, 0.5). R in [0, 1]. */
  void reset(Sample freqNormalized)
  {
    x1 = 0;
    y1 = std::complex<Sample>{Sample(0), Sample(0)};

    push(freqNormalized);
    a1.catchUp();
    b.catchUp();
  }

  Sample process(Sample x0)
  {
    y1 = b.process() * (x0 + x1) + a1.process() * y1;
    x1 = x0;
    return y1.real();
  }
};

template<typename Sample, size_t length> class ParallelMatchedBandpass {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  std::array<Sample, length> x1{};
  std::array<Sample, length> x2{};
  std::array<Sample, length> y1{};
  std::array<Sample, length> y2{};

public:
  void reset()
  {
    x1.fill({});
    x2.fill({});
    y1.fill({});
    y2.fill({});
  }

  void process(
    std::array<Sample, length> &x0,
    const std::array<Sample, length> &cutoffNormalized,
    Sample Q,
    Sample mod)
  {
    using T = Sample;

    const auto q = T(0.5) / Q;
    for (size_t idx = 0; idx < length; ++idx) {
      constexpr Sample twopi = T(2) * std::numbers::pi_v<Sample>;
      const auto w0 = twopi * std::clamp(cutoffNormalized[idx] * mod, minCutoff, nyquist);

      auto a1 = T(-2) * std::exp(-q * w0)
        * (q <= T(1) ? std::cos(std::sqrt(T(1) - q * q) * w0)
                     : std::cosh(std::sqrt(q * q - T(1)) * w0));
      auto a2 = std::exp(T(-2) * q * w0);

      auto r0 = (T(1) + a1 + a2) / (w0 * Q);
      auto wQ = w0 / Q;
      auto one_ww = T(1) - w0 * w0;
      auto r1 = wQ * (T(1) - a1 + a2) / std::sqrt(one_ww * one_ww + wQ * wQ);

      auto b0 = T(0.5) * r0 + T(0.25) * r1;
      auto b1 = T(-0.5) * r1;
      auto b2 = -b0 - b1;

      auto y0 = b0 * x0[idx] + b1 * x1[idx] + b2 * x2[idx] - a1 * y1[idx] - a2 * y2[idx];

      x2[idx] = x1[idx];
      x1[idx] = x0[idx];
      y2[idx] = y1[idx];
      y1[idx] = y0;

      x0[idx] = y0;
    }
  }
};

// This implementation only updates internal values at control rate.
template<typename Sample> class SVFHighpass {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  Sample s1 = 0;
  Sample s2 = 0;

  ExpSmoother<Sample> g;
  ExpSmoother<Sample> d;
  ExpSmoother<Sample> k;

public:
  void push(Sample freqNormalized, Sample Q)
  {
    g.push(std::tan(
      std::numbers::pi_v<Sample> * std::clamp(freqNormalized, minCutoff, nyquist)));
    k.push(Sample(1) / Q);
    d.push(Sample(1) / (Sample(1) + g.target * (g.target + k.target)));
  }

  void reset(Sample freqNormalized, Sample Q)
  {
    s1 = 0;
    s2 = 0;

    push(freqNormalized, Q);
    g.catchUp();
    k.catchUp();
    d.catchUp();
  }

  Sample process(Sample v0)
  {
    g.process();
    d.process();
    k.process();
    auto v1 = (s1 + g.value * (v0 - s2)) * d.value;
    auto v2 = s2 + g.value * v1;
    s1 = Sample(2) * v1 - s1;
    s2 = Sample(2) * v2 - s2;
    return v0 - k.value * v1 - v2;
  }
};

template<typename Sample> class Delay {
private:
  int wptr = 0;
  std::vector<Sample> buf{Sample(0), Sample(0)};

public:
  void setup(Sample maxTimeSamples)
  {
    buf.resize(std::max(size_t(2), size_t(maxTimeSamples) + 1));
    reset();
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  Sample process(Sample input, Sample timeInSamples)
  {
    const int size = int(buf.size());

    // Set delay time. Min delay is set to 1 sample to avoid artifact of feedback.
    Sample clamped = std::clamp(timeInSamples, Sample(1), Sample(size - 1));
    const int timeInt = int(clamped);
    Sample rFraction = clamped - Sample(timeInt);

    // Write to buffer.
    buf[wptr] = input;
    if (++wptr >= size) wptr = 0;

    // Read from buffer.
    int rptr0 = wptr - timeInt;
    if (rptr0 < 0) rptr0 += size;
    return std::lerp(buf[rptr0], buf[(rptr0 != 0 ? rptr0 : size) - 1], rFraction);
  }
};

template<typename Sample, size_t length> class ParallelRateLimiter {
public:
  static constexpr Sample rate = Sample(0.5); // Fixed for delays.

  std::array<Sample, length> value{};
  std::array<Sample, length> target{};

  void pushAt(size_t index, Sample pushedValue = 0) { target[index] = pushedValue; }
  void push(const std::array<Sample, length> &pushedValue) { target = pushedValue; }

  void resetAt(size_t index, Sample resetValue = 0)
  {
    target[index] = resetValue;
    value[index] = resetValue;
  }

  void reset(const std::array<Sample, length> &resetValue)
  {
    target = resetValue;
    value = resetValue;
  }

  void process()
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto diff = target[idx] - value[idx];
      value[idx]
        = std::abs(diff) > rate ? value[idx] + std::copysign(rate, diff) : target[idx];
    }
  }
};

template<typename Sample, size_t nAllpass> class SerialAllpass {
private:
  std::array<Sample, nAllpass> buffer{};
  std::array<Delay<Sample>, nAllpass> delay;

public:
  static constexpr size_t size = nAllpass;
  ParallelRateLimiter<Sample, nAllpass> timeInSamples;

  void setup(Sample maxTimeSamples)
  {
    for (auto &x : delay) x.setup(maxTimeSamples);
  }

  void reset()
  {
    buffer.fill({});
    for (auto &x : delay) x.reset();
  }

  Sample process(Sample input, Sample gain)
  {
    timeInSamples.process();

    Sample sum = input;
    for (size_t idx = 0; idx < nAllpass; ++idx) {
      const auto x0 = input - gain * buffer[idx];
      input = buffer[idx] + gain * x0;
      sum += input;
      buffer[idx] = delay[idx].process(x0, timeInSamples.value[idx]);
    }
    return sum;
  }
};

template<typename Sample> class EnergyStoreDecay {
private:
  static constexpr Sample eps = std::numeric_limits<Sample>::epsilon();
  Sample sum = 0;
  Sample decay = 0;
  Sample gain = 0;

public:
  void setup(Sample decayTimeSamples)
  {
    sum = 0;
    decay = -std::log(eps) / decayTimeSamples;
    gain = std::exp(-decay);
  }

  void reset() { sum = 0; }

  Sample process(Sample value)
  {
    const auto absed = std::abs(value);
    if (absed > eps) sum = (sum + value) * decay;
    return sum *= gain;
  }
};

template<typename Sample, typename Rng> class EnergyStoreNoise {
private:
  Sample sum = 0;

public:
  void reset() { sum = 0; }

  Sample process(Sample value, Rng &rng)
  {
    sum += std::abs(value);
    std::uniform_real_distribution<Sample> dist{Sample(-sum), Sample(sum)};
    const auto out = dist(rng);
    sum -= std::abs(out);
    return out;
  }
};

template<typename Sample, size_t length> class ParallelDelay {
private:
  std::array<int, length> wptr{};
  std::array<std::vector<Sample>, length> buffer;

public:
  ParallelDelay()
  {
    for (auto &bf : buffer) bf.resize(2);
  }

  void setup(Sample maxTimeSamples)
  {
    auto maxSize = std::max(size_t(2), size_t(maxTimeSamples) + 1);
    for (auto &bf : buffer) bf.resize(maxSize);

    reset();
  }

  void reset()
  {
    wptr.fill({});
    for (auto &bf : buffer) std::fill(bf.begin(), bf.end(), Sample(0));
  }

  void process(
    std::array<Sample, length> &input,
    const std::array<Sample, length> &timeInSamples,
    Sample timeScaler)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto &bf = buffer[idx];
      const int size = int(bf.size());

      // Set delay time. Min delay is set to 1 sample to avoid artifact of feedback.
      Sample clamped
        = std::clamp(timeInSamples[idx] / timeScaler, Sample(1), Sample(size - 1));
      int timeInt = int(clamped);
      Sample rFraction = clamped - Sample(timeInt);

      // Write to buffer.
      bf[wptr[idx]] = input[idx];
      if (++wptr[idx] >= size) wptr[idx] = 0;

      // Read from buffer.
      int rptr0 = wptr[idx] - timeInt;
      if (rptr0 < 0) rptr0 += size;
      input[idx] = std::lerp(bf[rptr0], bf[(rptr0 != 0 ? rptr0 : size) - 1], rFraction);
    }
  }
};

template<typename Sample, size_t length> class MembraneDelayRateLimiter {
private:
  static constexpr Sample rate = Sample(0.5);

public:
  std::array<Sample, length> value{};

  void process(
    const std::array<Sample, length> &base,
    const std::array<Sample, length> &modIn,
    Sample modAmount)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto target = base[idx] - std::abs(modAmount * modIn[idx]);
      auto diff = target - value[idx];
      value[idx]
        = std::abs(diff) > rate ? value[idx] + std::copysign(rate, diff) : target;
    }
  }
};

template<typename Sample, size_t length> class FeedbackMatrix {
public:
  std::array<Sample, length> seed{};
  std::array<ParallelExpSmoother<Sample, length>, length> matrix;

  // Construct Householder matrix. Call this after updating `seed`.
  //
  // `matrix` is 2D array of a square matrix.
  // `seed` is 1D array of a vector which length is the same as `matrix`.
  //
  // Reference: https://nhigham.com/2020/09/15/what-is-a-householder-matrix/
  void constructHouseholder()
  {
    Sample denom = 0;
    for (size_t i = 0; i < length; ++i) denom += seed[i] * seed[i];

    if (denom <= std::numeric_limits<Sample>::epsilon()) {
      for (size_t i = 0; i < length; ++i) {
        for (size_t j = 0; j < length; ++j) {
          matrix[i].target[j] = i == j ? Sample(1) : Sample(0);
        }
      }
      return;
    }

    auto scale = Sample(-2) / denom;

    for (size_t i = 0; i < length; ++i) {
      // Diagonal elements.
      matrix[i].target[i] = Sample(1) + scale * seed[i] * seed[i];

      // Non-diagonal elements.
      for (size_t j = i + 1; j < length; ++j) {
        auto value = scale * seed[i] * seed[j];
        matrix[i].target[j] = value;
        matrix[j].target[i] = value;
      }
    }
  }

  void reset()
  {
    for (auto &x : matrix) x.catchUp();
  }

  void process()
  {
    for (auto &x : matrix) x.process();
  }

  Sample at(size_t i, size_t j) { return matrix[i].value[j]; }
};

template<typename Sample, size_t length> class EasyFDN {
private:
  size_t bufIndex = 0;
  std::array<std::array<Sample, length>, 2> buf;

  MembraneDelayRateLimiter<Sample, length> delayTimeRateLimiter;
  ParallelDelay<Sample, length> delay;
  ParallelMatchedBandpass<Sample, length> bandpass;

  Sample safetyGain = 0;
  Sample crossDecaySteep = 0;
  Sample crossDecayGentle = 0;

public:
  // Before calling `process`,
  // 1. Fill all the parameters in the paragraph below.
  // 2. Call `constructHouseholder`.
  // 3. Call `reset`, but only when resetting.
  std::array<Sample, length> delayTimeSamples{};
  ParallelExpSmoother<Sample, length> bandpassCutoff; // Normalized in [0, 0.5).
  ExpSmoother<Sample> bandpassQ;
  ExpSmoother<Sample> crossGain;

  void setup(Sample maxTimeSamples) { delay.setup(maxTimeSamples); }

  void onSampleRateChange(Sample sampleRate)
  {
    crossDecaySteep = std::pow(Sample(0.85), Sample(48000) / sampleRate);
    crossDecayGentle
      = std::pow(std::numeric_limits<Sample>::epsilon(), Sample(0.366) / sampleRate);
  }

  void reset()
  {
    bufIndex = 0;
    for (auto &x : buf) x.fill({});

    delayTimeRateLimiter.value = delayTimeSamples;
    delay.reset();
    bandpass.reset();

    safetyGain = 0;
  }

  void noteOn() { safetyGain = Sample(1); }

  Sample process(
    Sample input,
    Sample crossGain,
    Sample pitchMod,
    Sample timeModAmount,
    FeedbackMatrix<Sample, length> &feedbackMatrix)
  {
    bufIndex ^= 1;
    auto &front = buf[bufIndex];
    auto &back = buf[bufIndex ^ 1];
    front.fill({});
    // feedbackMatrix.process();
    for (size_t i = 0; i < length; ++i) {
      feedbackMatrix.matrix[i].process();
      for (size_t j = 0; j < length; ++j) front[i] += feedbackMatrix.at(i, j) * back[j];
    }

    // input /= Sample(length);
    const auto feedbackGain = safetyGain * crossGain;
    for (size_t i = 0; i < length; ++i) front[i] = input + feedbackGain * front[i];

    bandpassCutoff.process();
    bandpass.process(front, bandpassCutoff.value, bandpassQ.process(), pitchMod);

    delayTimeRateLimiter.process(delayTimeSamples, front, timeModAmount);
    delay.process(front, delayTimeRateLimiter.value, pitchMod);

    const auto sum = std::accumulate(front.begin(), front.end(), Sample(0));
    if (Sample(length) < sum) {
      safetyGain *= sum <= Sample(100) ? crossDecayGentle : crossDecaySteep;
    }
    return sum;
  }
};

} // namespace SomeDSP
