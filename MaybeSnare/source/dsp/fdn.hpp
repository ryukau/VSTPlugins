// (c) 2022 Takamitsu Endo
//
// This file is part of MaybeSnare.
//
// MaybeSnare is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MaybeSnare is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MaybeSnare.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>

namespace SomeDSP {

template<typename Sample> class SREnvelope {
public:
  size_t sustainCounter = 0;
  DoubleEMAFilter<Sample> ema;

  void reset()
  {
    sustainCounter = 0;
    ema.reset(Sample(1));
  }

  void noteOn(Sample sampleRate, Sample sustainInSeconds, Sample releaseInSeconds)
  {
    sustainCounter = size_t(sampleRate * sustainInSeconds);
    ema.setCutoff(sampleRate, Sample(1) / releaseInSeconds);
    ema.reset(Sample(1));
  }

  Sample process()
  {
    if (sustainCounter > 0) {
      --sustainCounter;
      return ema.process(Sample(1));
    }

    return ema.process(Sample(0));
  }
};

template<typename Sample> class PulseGenerator {
public:
  static constexpr Sample epsilon = Sample(std::numeric_limits<float>::epsilon());

  Sample pulseGain = 0;
  Sample pulseDecay = Sample(1);

  Sample noiseGain = 0;
  Sample noiseDecay = Sample(1);

  EMAFilter<Sample> noiseLowpass;

  void noteOn(
    Sample sampleRate,
    Sample amplitude,
    Sample pulseDecaySamples,
    Sample noiseDecaySamples,
    Sample noiseLowpassHz)
  {
    pulseDecay = std::pow(epsilon, Sample(1) / pulseDecaySamples);
    pulseGain = amplitude / pulseDecay;

    noiseDecay = std::pow(epsilon, Sample(1) / noiseDecaySamples);
    noiseGain = amplitude / noiseDecay;
    noiseLowpass.setCutoff(sampleRate, noiseLowpassHz);
  }

  void reset()
  {
    pulseGain = 0;
    noiseLowpass.reset();
  }

  Sample process(pcg64 &rng, Sample noiseMix)
  {
    pulseGain *= pulseDecay;
    noiseGain *= noiseDecay;

    std::normal_distribution<Sample> normal(Sample(-1), Sample(1) / Sample(3));
    auto noise = noiseLowpass.process(noiseGain * normal(rng));

    return pulseGain + noiseMix * (noise - pulseGain);
  }
};

template<typename Sample, size_t length> class ParallelSVFHighpass {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  std::array<Sample, length> ic1eq{};
  std::array<Sample, length> ic2eq{};

  std::array<ExpSmoother<Sample>, length> g;
  std::array<ExpSmoother<Sample>, length> k;

public:
  void pushCutoffAt(size_t index, Sample normalizedFreq, Sample Q)
  {
    g[index].push(std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k[index].push(Sample(1) / Q);
  }

  void resetCutoffAt(size_t index, Sample normalizedFreq, Sample Q)
  {
    g[index].reset(std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k[index].reset(Sample(1) / Q);
  }

  void reset()
  {
    ic1eq.fill(0);
    ic2eq.fill(0);
  }

  void process(std::array<Sample, length> &v0)
  {
    for (size_t n = 0; n < length; ++n) {
      auto gn = g[n].process();
      auto kn = k[n].process();
      auto v1 = (ic1eq[n] + gn * (v0[n] - ic2eq[n])) / (Sample(1) + gn * (gn + kn));
      auto v2 = ic2eq[n] + gn * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] -= kn * v1 + v2;
    }
  }
};

template<typename Sample, size_t length> class ParallelSVFHighshelf {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  // `A` is square root of shelving gain. The gain is fixed to 0.5 for this application.
  static constexpr Sample A = Sample(halfSqrt2);                     // 0.5^(1/2).
  static constexpr Sample A_sqrt = Sample(0.8408964152537145430311); // 0.5^(1/4).

  std::array<Sample, length> ic1eq{};
  std::array<Sample, length> ic2eq{};

  std::array<ExpSmoother<Sample>, length> g;
  std::array<ExpSmoother<Sample>, length> k;

public:
  void pushCutoffAt(size_t index, Sample normalizedFreq, Sample Q)
  {
    g[index].push(
      A_sqrt * std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k[index].push(Sample(1) / Q);
  }

  void resetCutoffAt(size_t index, Sample normalizedFreq, Sample Q)
  {
    g[index].reset(
      A_sqrt * std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k[index].reset(Sample(1) / Q);
  }

  void reset()
  {
    ic1eq.fill(0);
    ic2eq.fill(0);
  }

  void process(std::array<Sample, length> &v0)
  {
    for (size_t n = 0; n < length; ++n) {
      auto gn = g[n].process();
      auto kn = k[n].process();
      auto v1 = (ic1eq[n] + gn * (v0[n] - ic2eq[n])) / (Sample(1) + gn * (gn + kn));
      auto v2 = ic2eq[n] + gn * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] = A * A * (v0[n] - kn * v1 - v2) + A * kn * v1 + v2;
    }
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

template<typename Sample, size_t length> class ParallelDelay {
public:
  std::array<Sample, length> neutralTime{};
  std::array<Sample, length> time{};
  std::array<size_t, length> wptr{};
  std::array<std::vector<Sample>, length> buffer;

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto size = size_t(sampleRate * maxTime) + 2;
    for (auto &bf : buffer) bf.resize(size < 4 ? 4 : size);

    reset();
  }

  void reset(Sample timeInSample = 0)
  {
    neutralTime.fill(timeInSample);
    time.fill(timeInSample);
    for (auto &bf : buffer) std::fill(bf.begin(), bf.end(), Sample(0));
  }

  void setDelayTimeAt(size_t index, Sample sampleRate, Sample overtone, Sample noteFreq)
  {
    constexpr auto eps = std::numeric_limits<Sample>::epsilon();
    overtone = std::max(eps, overtone);
    noteFreq = std::max(eps, noteFreq);
    neutralTime[index] = std::clamp(
      sampleRate / (overtone * noteFreq), Sample(0), Sample(buffer[index].size() - 1));
  }

  void resetDelayTimeAt(size_t index, Sample sampleRate, Sample overtone, Sample noteFreq)
  {
    setDelayTimeAt(index, sampleRate, overtone, noteFreq);
    time[index] = neutralTime[index];
  }

  void process(
    std::array<Sample, length> &input,
    Sample modulation,
    Sample slewRate,
    Sample minModulation)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      // Rate limit delay time.
      auto timeMod = Sample(1) - modulation * std::abs(input[idx]);
      auto targetTime = neutralTime[idx] * std::max(minModulation, timeMod);
      auto diff = targetTime - time[idx];
      if (diff > slewRate) {
        time[idx] += slewRate;
      } else if (diff < -slewRate) {
        time[idx] -= slewRate;
      } else {
        time[idx] = targetTime;
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

template<typename Sample, size_t length> class SnaredFDN {
private:
  std::array<std::array<Sample, length>, length> matrix{};
  std::array<std::array<Sample, length>, 2> buf{};
  size_t bufIndex = 0;

public:
  std::array<Sample, length> inputGain{};
  ParallelDelay<Sample, length> delay;
  ParallelSVFHighshelf<Sample, length> lowpass;
  ParallelSVFHighpass<Sample, length> highpass;

  SnaredFDN() { inputGain.fill(Sample(1) / Sample(length)); }

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

    // Lowpass cutoff is set slightly below Nyquist frequency to prevent blow up.
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

  Sample process(
    Sample input,
    Sample feedback,
    Sample modulation,
    Sample delayTimeSlewRate,
    Sample minModulation)
  {
    bufIndex ^= 1;
    auto &front = buf[bufIndex];
    auto &back = buf[bufIndex ^ 1];
    front.fill(0);
    for (size_t i = 0; i < length; ++i) {
      for (size_t j = 0; j < length; ++j) front[i] += matrix[i][j] * back[j];
    }

    for (size_t idx = 0; idx < length; ++idx) {
      front[idx] = input * inputGain[idx] + feedback * front[idx];
    }
    delay.process(front, modulation, delayTimeSlewRate, minModulation);
    lowpass.process(front);
    highpass.process(front);

    return std::accumulate(front.begin(), front.end(), Sample(0));
  }
};

} // namespace SomeDSP
