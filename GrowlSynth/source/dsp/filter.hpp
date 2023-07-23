// (c) 2023 Takamitsu Endo
//
// This file is part of GrowlSynth.
//
// GrowlSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrowlSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrowlSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <numeric>
#include <random>
#include <vector>

namespace SomeDSP {

/**
Bilinear transformed 1-pole lowpass filter. Slower than EMAFilter, but eliminates Nyquist
frequency.

Web version of GrowlSynth clamps `cutoffNormalized` in [0.00001, 0.49998].
*/
template<typename Sample> class LP1 {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  ExpSmoother<Sample> bn;
  ExpSmoother<Sample> a1;
  Sample x1 = 0;
  Sample y1 = 0;

public:
#define ASSIGN_COEFFICINETS(METHOD)                                                      \
  auto cutoff = std::clamp(cutoffNormalized, minCutoff, nyquist);                        \
  auto k = Sample(1) / std::tan(std::numbers::pi_v<Sample> * cutoff);                    \
  auto a0 = Sample(1) + k;                                                               \
  bn.METHOD(Sample(1) / a0);                                                             \
  a1.METHOD((k - Sample(1)) / a0); /* Negated. */

  void reset(Sample cutoffNormalized)
  {
    x1 = 0;
    y1 = 0;
    ASSIGN_COEFFICINETS(reset);
  }

  void push(Sample cutoffNormalized) { ASSIGN_COEFFICINETS(push); }
#undef ASSIGN_COEFFICINETS

  Sample process(Sample x0)
  {
    y1 = bn.process() * (x0 + x1) + a1.process() * y1;
    x1 = x0;
    return y1;
  }
};

namespace SVFTool {
enum SVFType : size_t {
  lowpass,
  bandpass,
  highpass,
  notch,
  peak,
  allpass,
  bell,
  lowshelf,
  highshelf,
};
}

/**
Translation of SVF in Faust filter.lib.
https://faustlibraries.grame.fr/libs/filters/#svf-filters

List of `type`. Use `SVFTool::SVFType` when possible.
- 0: LP
- 1: BP
- 2: HP
- 3: Notch
- 4: Peak
- 5: AP
- 6: Bell
- 7: Low-shelf
- 8: High-shelf
*/
template<typename Sample, size_t type> class SVF {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  Sample s1 = Sample(0);
  Sample s2 = Sample(0);

public:
  SVF()
  {
    static_assert(
      type <= SVFTool::highshelf,
      "SVF type must be less than or equal to 8. Use SVFTool::SVFType.");
  }

  void reset()
  {
    s1 = Sample(0);
    s2 = Sample(0);
  }

  Sample
  process(Sample v0, Sample normalizedFreq, Sample Q, Sample shelvingGainAmp = Sample(1))
  {
    auto A = Sample(1);
    if constexpr (type >= SVFTool::bell) A = std::sqrt(shelvingGainAmp);

    auto g = std::tan(
      std::clamp(normalizedFreq, minCutoff, nyquist) * std::numbers::pi_v<Sample>);
    if constexpr (type == SVFTool::lowshelf)
      g /= std::sqrt(A);
    else if constexpr (type == SVFTool::highshelf)
      g *= std::sqrt(A);

    auto k = Sample(1) / Q;
    if constexpr (type == SVFTool::bell) k /= A;

    // tick.
    auto v1 = (s1 + g * (v0 - s2)) / (Sample(1) + g * (g + k));
    auto v2 = s2 + g * v1;

    s1 = Sample(2) * v1 - s1;
    s2 = Sample(2) * v2 - s2;

    // Mix.
    if constexpr (type == SVFTool::lowpass) {
      return v2;
    } else if constexpr (type == SVFTool::bandpass) {
      return v1;
    } else if constexpr (type == SVFTool::highpass) {
      return v0 - k * v1 - v2;
    } else if constexpr (type == SVFTool::notch) {
      return v0 - k * v1;
    } else if constexpr (type == SVFTool::peak) {
      return v0 - k * v1 - Sample(2) * v2;
    } else if constexpr (type == SVFTool::allpass) {
      return v0 - Sample(2) * k * v1;
    } else if constexpr (type == SVFTool::bell) {
      return v0 + k * (A * A - Sample(1)) * v1;
    } else if constexpr (type == SVFTool::lowshelf) {
      return v0 + (A - Sample(1)) * k * v1 + (A * A - Sample(1)) * v2;
    } else if constexpr (type == SVFTool::highshelf) {
      return A * A * (v0 - k * v1 - v2) + A * k * v1 + v2;
    }
    return Sample(0); // Shouldn't reach here.
  }
};

/**
This implementation only updates internal values at control rate.
*/
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
#define ASSIGN_COEFFICINETS(METHOD, EXTRA)                                               \
  void METHOD(Sample freqNormalized, Sample Q)                                           \
  {                                                                                      \
    EXTRA;                                                                               \
                                                                                         \
    g.METHOD(std::tan(                                                                   \
      std::numbers::pi_v<Sample> *std::clamp(freqNormalized, minCutoff, nyquist)));      \
    k.METHOD(Sample(1) / Q);                                                             \
    d.METHOD(Sample(1) / (Sample(1) + g.target * g.target + g.target * k.target));       \
  }

  ASSIGN_COEFFICINETS(reset, s1 = 0; s2 = 0;)
  ASSIGN_COEFFICINETS(push, )
#undef ASSIGN_COEFFICINETS

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
  void setup(Sample sampleRate, Sample maxTimeSeconds)
  {
    buf.resize(std::max(size_t(2), size_t(sampleRate * maxTimeSeconds) + 1));
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

template<typename Sample> class LPComb {
private:
  Sample lp = 0;
  Sample hp = 0;
  Delay<Sample> delay;

public:
  void setup(Sample sampleRate, Sample maxTimeSeconds)
  {
    delay.setup(sampleRate, maxTimeSeconds);
  }

  void
  reset(Sample combPeriodSamples, Sample feedback, Sample lowpassCut, Sample highpassCut)
  {
    lp = 0;
    hp = 0;
    delay.reset();
  }

  Sample process(
    Sample x0,
    Sample combPeriodSamples,
    Sample feedback,
    Sample lowpassCut,
    Sample highpassCut)
  {
    auto s0 = delay.process(x0 + feedback * lp, combPeriodSamples);

    auto k_lp = Sample(EMAFilter<double>::cutoffToP(lowpassCut));
    lp += k_lp * (s0 - lp);

    auto k_hp = Sample(EMAFilter<double>::cutoffToP(highpassCut));
    hp += k_hp * (lp - hp);
    return lp - hp;
  }
};

template<typename Sample, typename RandomNumberGenerator> class MaybeFormant {
public:
  static constexpr size_t nComb = 3;
  static constexpr size_t nBell = 6;
  static constexpr size_t nNotch = 2;

  // Formant parameters. Values are obtained from hand tuning.
  static constexpr std::array<Sample, nComb> baseCombSamples{
    Sample(1.0 / 360.0), Sample(1.0 / 3200.0), Sample(1.0 / 500.0)};
  static constexpr std::array<Sample, nComb> combFeedback{
    Sample(0.9), Sample(0.95), Sample(1.0)};
  static constexpr std::array<Sample, nComb> baseCombLowpassCut{
    Sample(500), Sample(2000), Sample(2000)};
  static constexpr std::array<Sample, nComb> baseCombHighpassCut{
    Sample(100), Sample(200), Sample(3000)};

  static constexpr std::array<Sample, nBell> baseBellCut{
    Sample(100), Sample(800), Sample(1100), Sample(2600), Sample(200), Sample(500)};
  static constexpr std::array<Sample, nBell> baseBellQ{
    Sample(2), Sample(2), Sample(2), Sample(1), Sample(0.5), Sample(0.25)};
  static constexpr std::array<Sample, nBell> bellGain{
    Sample(10), Sample(22.4), Sample(22.4), Sample(10), Sample(0.03125), Sample(0.1)};

  static constexpr std::array<Sample, nNotch> baseNotchCut{Sample(1500), Sample(4000)};
  static constexpr std::array<Sample, nNotch> baseNotchQ{Sample(0.25), Sample(1)};

  static constexpr Sample highshelfCut = Sample(16000);
  static constexpr Sample highshelfQ = std::numbers::sqrt2_v<Sample> / Sample(2);
  static constexpr Sample highshelfGain = Sample(0.1);

  // Randomized values.
  std::array<Sample, nComb> combSamples = baseCombSamples;
  std::array<Sample, nComb> combLowpassCut = baseCombLowpassCut;
  std::array<Sample, nComb> combHighpassCut = baseCombHighpassCut;

  std::array<Sample, nBell> bellCut = baseBellCut;
  std::array<Sample, nBell> bellQ = baseBellQ;

  std::array<Sample, nNotch> notchCut = baseNotchCut;
  std::array<Sample, nNotch> notchQ = baseNotchQ;

  // DSP components.
  std::array<LPComb<Sample>, nComb> lpComb;
  std::array<SVF<Sample, SVFTool::bell>, nBell> bell;
  std::array<SVF<Sample, SVFTool::notch>, nNotch> notch;
  SVF<Sample, SVFTool::highshelf> highshelf;

  void refresh(RandomNumberGenerator &rng)
  {
    std::uniform_real_distribution<Sample> dist{Sample(0), Sample(1)};

    for (size_t idx = 0; idx < lpComb.size(); ++idx) {
      combSamples[idx] = std::exp2(dist(rng)) * baseCombSamples[idx];
      combLowpassCut[idx] = std::exp2(dist(rng)) * baseCombLowpassCut[idx];
      combHighpassCut[idx] = std::exp2(dist(rng)) * baseCombHighpassCut[idx];
    }

    for (size_t idx = 0; idx < bell.size(); ++idx) {
      bellCut[idx] = std::exp2(dist(rng)) * baseBellCut[idx];
      bellQ[idx] = std::exp2(dist(rng)) * baseBellQ[idx];
    }

    for (size_t idx = 0; idx < notch.size(); ++idx) {
      notchCut[idx] = std::exp2(dist(rng)) * baseNotchCut[idx];
      notchQ[idx] = std::exp2(dist(rng)) * baseNotchQ[idx];
    }
  }

  // freqRatio = exp2(octave) / sampleRate.
  Sample process(Sample x0, Sample freqRatio)
  {
    for (size_t idx = 0; idx < lpComb.size(); ++idx) {
      x0 = lpComb[idx].process(
        x0, combSamples[idx], combFeedback[idx], combLowpassCut[idx] * freqRatio,
        combHighpassCut[idx] * freqRatio);
    }

    for (size_t idx = 0; idx < bell.size(); ++idx) {
      x0 = bell[idx].process(x0, bellCut[idx] * freqRatio, bellQ[idx], bellGain[idx]);
    }

    for (size_t idx = 0; idx < notch.size(); ++idx) {
      x0 = notch[idx].process(x0, notchCut[idx] * freqRatio, notchQ[idx]);
    }

    return highshelf.process(x0, highshelfCut, highshelfQ, highshelfGain);
  }
};

template<typename Sample, typename RandomNumberGenerator> class NoiseFormant {
public:
  static constexpr size_t nBandpass = 5;

  // Formant parameters. Values are obtained from hand tuning.
  static constexpr std::array<Sample, nBandpass> baseBandpassCut{
    Sample(1000), Sample(2800), Sample(3700), Sample(5500), Sample(8600)};
  static constexpr std::array<Sample, nBandpass> baseBandpassQ{
    Sample(2), Sample(2), Sample(2), Sample(1), Sample(0.5)};
  static constexpr std::array<Sample, nBandpass> bandpassGain{
    Sample(1), Sample(0.5623), Sample(0.3162), Sample(1), Sample(0.3162)};

  static constexpr Sample lowpassCut = Sample(16000);
  static constexpr Sample lowpassQ = std::numbers::sqrt2_v<Sample> / Sample(2);

  // Randomized values.
  std::array<Sample, nBandpass> bandpassCut = baseBandpassCut;
  std::array<Sample, nBandpass> bandpassQ = baseBandpassQ;

  // DSP components.
  std::array<SVF<Sample, SVFTool::bandpass>, nBandpass> bandpass;
  std::array<ExpSmoother<Sample>, nBandpass> bandGain;
  SVF<Sample, SVFTool::lowpass> lowpass;

  void reset()
  {
    for (size_t idx = 0; idx < bandGain.size(); ++idx) {
      bandGain[idx].reset(bandpassGain[idx]);
    }
  }

  void refresh(RandomNumberGenerator &rng)
  {
    std::uniform_real_distribution<Sample> dist{Sample(0), Sample(1)};
    for (size_t idx = 0; idx < nBandpass; ++idx) {
      bandpassCut[idx] = std::exp2(dist(rng)) * baseBandpassCut[idx];
      bandpassQ[idx] = std::exp2(dist(rng)) * baseBandpassQ[idx];
    }
  }

  // freqRatio = exp2(octave) / sampleRate.
  Sample process(Sample x0, Sample freqRatio)
  {
    Sample sum = 0;
    for (size_t idx = 0; idx < nBandpass; ++idx) {
      sum += bandGain[idx].process()
        * bandpass[idx].process(x0, bandpassCut[idx] * freqRatio, bandpassQ[idx]);
    }
    return lowpass.process(sum, lowpassCut * freqRatio, lowpassQ);
  }
};

template<typename Sample, size_t nTap> class MultiTapDelay {
private:
  int wptr = 0;
  std::vector<Sample> buf{Sample(0), Sample(0)};

public:
  std::array<Sample, nTap> output{};
  ParallelExpSmoother<Sample, nTap> timeInSamples;

  inline size_t size() { return nTap; }
  inline Sample sum() { return std::accumulate(output.begin(), output.end(), Sample(0)); }

  void setup(Sample sampleRate, Sample maxTimeSeconds)
  {
    buf.resize(std::max(size_t(2), size_t(sampleRate * maxTimeSeconds) + 1));
    reset();
  }

  void reset()
  {
    output.fill({});
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  void process(Sample input)
  {
    timeInSamples.process();

    const int size = int(buf.size());
    buf[wptr] = input;
    if (++wptr >= size) wptr = 0;

    for (size_t idx = 0; idx < nTap; ++idx) {
      Sample clamped = std::clamp(timeInSamples.value[idx], Sample(0), Sample(size - 1));
      const int timeInt = int(clamped);
      Sample rFraction = clamped - Sample(timeInt);

      int rptr0 = wptr - timeInt;
      if (rptr0 < 0) rptr0 += size;
      output[idx]
        = std::lerp(buf[rptr0], buf[(rptr0 != 0 ? rptr0 : size) - 1], rFraction);
    }
  }
};

template<typename Sample, size_t length> class ParallelLP1 {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  ParallelExpSmoother<Sample, length> bn;
  ParallelExpSmoother<Sample, length> a1;
  std::array<Sample, length> x1{};
  std::array<Sample, length> y1{};

public:
  void resetDSP()
  {
    x1.fill({});
    y1.fill({});
  }

#define ASSIGN_COEFFICINETS(METHOD)                                                      \
  void METHOD##At(size_t index, Sample cutoffNormalized)                                 \
  {                                                                                      \
    auto cutoff = std::clamp(cutoffNormalized, minCutoff, nyquist);                      \
    auto k = Sample(1) / std::tan(std::numbers::pi_v<Sample> * cutoff);                  \
    auto a0 = Sample(1) + k;                                                             \
    bn.METHOD##At(index, Sample(1) / a0);                                                \
    a1.METHOD##At(index, (k - Sample(1)) / a0); /* Negated. */                           \
  }

  ASSIGN_COEFFICINETS(push)
  ASSIGN_COEFFICINETS(reset)
#undef ASSIGN_COEFFICINETS

  void process(std::array<Sample, length> &x0)
  {
    bn.process();
    a1.process();
    for (size_t i = 0; i < length; ++i) {
      y1[i] = bn.value[i] * (x0[i] + x1[i]) + a1.value[i] * y1[i];
      x1[i] = x0[i];
      x0[i] = y1[i];
    }
  }
};

/**
Bilinear transformed 1-pole highpass filter. Difference to EMAHighpass is marginally.
*/
template<typename Sample, size_t length> class ParallelHP1 {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  ParallelExpSmoother<Sample, length> b0;
  ParallelExpSmoother<Sample, length> a1;
  std::array<Sample, length> x1{};
  std::array<Sample, length> y1{};

public:
  void resetDSP()
  {
    x1.fill({});
    y1.fill({});
  }

#define ASSIGN_COEFFICINETS(METHOD)                                                      \
  void METHOD##At(size_t index, Sample cutoffNormalized)                                 \
  {                                                                                      \
    auto cutoff = std::clamp(cutoffNormalized, minCutoff, nyquist);                      \
    auto k = Sample(1) / std::tan(std::numbers::pi_v<Sample> * cutoff);                  \
    auto a0 = Sample(1) + k;                                                             \
    b0.METHOD##At(index, k / a0);                                                        \
    a1.METHOD##At(index, (Sample(1) - k) / a0); /* Negated. */                           \
  }

  ASSIGN_COEFFICINETS(push)
  ASSIGN_COEFFICINETS(reset)
#undef ASSIGN_COEFFICINETS

  void process(std::array<Sample, length> &x0)
  {
    b0.process();
    a1.process();
    for (size_t i = 0; i < length; ++i) {
      y1[i] = b0.value[i] * (x0[i] - x1[i]) - a1.value[i] * y1[i];
      x1[i] = x0[i];
      x0[i] = y1[i];
    }
  }
};

template<typename Sample, size_t length> class ParallelImmediateRateLimiter {
public:
  std::array<Sample, length> value{};

  void resetAt(size_t index, Sample resetValue = 0) { value[index] = resetValue; }

  void process(const std::array<Sample, length> &target, Sample rate)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto diff = target[idx] - value[idx];
      value[idx]
        = std::abs(diff) > rate ? value[idx] + std::copysign(rate, diff) : target[idx];
    }
  }
};

template<typename Sample, size_t length> class ParallelSVFAllpass {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  std::array<Sample, length> s1{};
  std::array<Sample, length> s2{};

public:
  void reset()
  {
    s1.fill({});
    s2.fill({});
  }

  void process(
    std::array<Sample, length> &v0,
    const std::array<Sample, length> &freqMod,
    const std::array<Sample, length> &normalizedFreq,
    const std::array<Sample, length> &Q)
  {
    for (size_t i = 0; i < length; ++i) {
      auto g = std::tan(
        std::clamp(normalizedFreq[i] * std::exp2(freqMod[i] * v0[i]), minCutoff, nyquist)
        * std::numbers::pi_v<Sample>);
      auto k = Sample(1) / Q[i];

      auto v1 = (s1[i] + g * (v0[i] - s2[i])) / (Sample(1) + g * (g + k));
      auto v2 = s2[i] + g * v1;
      s1[i] = Sample(2) * v1 - s1[i];
      s2[i] = Sample(2) * v2 - s2[i];

      v0[i] = v0[i] - Sample(2) * k * v1;
    }
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

  void setup(Sample sampleRate, Sample maxTimeSeconds)
  {
    auto maxSize = std::max(size_t(2), size_t(sampleRate * maxTimeSeconds) + 1);
    for (auto &bf : buffer) bf.resize(maxSize);

    reset();
  }

  void reset()
  {
    wptr.fill({});
    for (auto &bf : buffer) std::fill(bf.begin(), bf.end(), Sample(0));
  }

  void process(
    std::array<Sample, length> &input, const std::array<Sample, length> &timeInSamples)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto &bf = buffer[idx];
      const int size = int(bf.size());

      // Set delay time. Min delay is set to 1 sample to avoid artifact of feedback.
      Sample clamped = std::clamp(timeInSamples[idx], Sample(1), Sample(size - 1));
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

template<typename T, typename RandomNumberGenerator, size_t size> struct ModCombScaler {
private:
  /**
  log2Low = log2(lowerBoundOfReturnValue).
  log2High = log2(upperBoundOfReturnValue).

  These unintuitive convension is used because `std::log2` won't be constexpr until C++26.
  */
  inline T expMap(T x, T log2Low, T log2High)
  {
    return std::exp2(std::lerp(log2Low, log2High, x));
  }

public:
  std::array<T, size> lowpassCut{};
  std::array<T, size> highpassCut{};
  std::array<T, size> allpassCut{};
  std::array<T, size> allpassQ{};
  std::array<T, size> combSamples{};
  std::array<T, size> combFeedbaackGain{};
  std::array<T, size> timeSpread{};

  void reset()
  {
    lowpassCut.fill(T(1));
    highpassCut.fill(T(1));
    allpassCut.fill(T(1));
    allpassQ.fill(T(1));
    combFeedbaackGain.fill(T(1));
    combSamples.fill(T(1));
    timeSpread.fill(T(0));
  }

  void refresh(RandomNumberGenerator &rng, T combFreqRangeOctave)
  {
    std::uniform_real_distribution<T> dist{T(0), T(1)};
    for (auto &x : lowpassCut) x = expMap(dist(rng), T(-1), T(1));
    for (auto &x : highpassCut) x = expMap(dist(rng), T(-1), T(1));
    for (auto &x : allpassCut) x = expMap(dist(rng), T(-1), T(1));
    for (auto &x : allpassQ) x = expMap(dist(rng), T(-1), T(1));
    for (auto &x : combFeedbaackGain) x = expMap(dist(rng), T(-0.03), T(0));
    for (auto &x : timeSpread) x = dist(rng);

    std::uniform_real_distribution<T> randomOctaveDist{
      -combFreqRangeOctave, combFreqRangeOctave};
    for (auto &x : combSamples) x = std::exp2(randomOctaveDist(rng));
  }
};

template<typename Sample, typename RandomNumberGenerator, size_t length>
class ParallelModComb {
private:
  static constexpr size_t nAllpass = 2;

  ExpSmoother<Sample> lossThreshold;
  ExpSmoother<Sample> timeMod;
  ExpSmoother<Sample> timeRate;
  std::array<ParallelExpSmoother<Sample, length>, nAllpass> allpassCut;
  ParallelExpSmoother<Sample, length> allpassQ;
  ParallelExpSmoother<Sample, length> allpassMod;
  ParallelExpSmoother<Sample, length> timeSamples;
  ParallelExpSmoother<Sample, length> feedbackGain;

  MultiTapDelay<Sample, length> spreadDelay;
  std::array<Sample, length> fbSig{};
  ParallelLP1<Sample, length> lowpass;
  ParallelHP1<Sample, length> highpass;
  std::array<ParallelSVFAllpass<Sample, length>, nAllpass> allpass;
  ParallelImmediateRateLimiter<Sample, length> timeLimiter;
  ParallelDelay<Sample, length> delay;

  std::array<Sample, length> tmp{};

  inline void resetDSP(Sample delayTimeSamples_)
  {
    spreadDelay.reset();
    fbSig.fill({});
    lowpass.resetDSP();
    highpass.resetDSP();
    for (auto &x : allpass) x.reset();
    timeLimiter.value.fill(delayTimeSamples_);
    delay.reset();
  }

public:
#define ASSIGN_MOD_COMB_PARAMETERS(METHOD, EXTRA_CODE)                                   \
  void METHOD(                                                                           \
    const ModCombScaler<Sample, RandomNumberGenerator, length> &scaler,                  \
    Sample lowpassCutoffNormalized, Sample highpassCutoffNormalized,                     \
    Sample allpassCutoffNormalized, Sample allpassMod_, Sample allpassQ_,                \
    Sample lossThreshold_, Sample delayTimeSamples_, Sample feedbackGain_,               \
    Sample delayTimeMod_, Sample delayTimeSlewRate_, Sample maxTimeSpreadSamples_)       \
  {                                                                                      \
    lossThreshold.METHOD(lossThreshold_);                                                \
    timeMod.METHOD(delayTimeMod_);                                                       \
    timeRate.METHOD(delayTimeSlewRate_);                                                 \
                                                                                         \
    for (size_t idx = 0; idx < length; ++idx) {                                          \
      spreadDelay.timeInSamples.METHOD##At(                                              \
        idx, scaler.timeSpread[idx] * maxTimeSpreadSamples_);                            \
                                                                                         \
      lowpass.METHOD##At(idx, scaler.lowpassCut[idx] * lowpassCutoffNormalized);         \
      highpass.METHOD##At(idx, scaler.highpassCut[idx] * highpassCutoffNormalized);      \
                                                                                         \
      for (auto &x : allpassCut)                                                         \
        x.METHOD##At(idx, scaler.allpassCut[idx] * allpassCutoffNormalized);             \
                                                                                         \
      allpassQ.METHOD##At(idx, scaler.allpassQ[idx] * allpassQ_);                        \
      allpassMod.METHOD##At(idx, allpassMod_);                                           \
      timeSamples.METHOD##At(idx, scaler.combSamples[idx] * delayTimeSamples_);          \
      feedbackGain.METHOD##At(idx, scaler.combFeedbaackGain[idx] * feedbackGain_);       \
    }                                                                                    \
                                                                                         \
    EXTRA_CODE; /* Do nothing when EXTRA_CODE is empty. */                               \
  }

  ASSIGN_MOD_COMB_PARAMETERS(reset, resetDSP(delayTimeSamples_))
  ASSIGN_MOD_COMB_PARAMETERS(push, )
#undef ASSIGN_MOD_COMB_PARAMETERS

  void setup(Sample sampleRate, Sample maxCombSeconds, Sample maxTimeSpreadSeconds)
  {
    spreadDelay.setup(sampleRate, maxTimeSpreadSeconds);
    delay.setup(sampleRate, maxCombSeconds);
  }

  Sample
  process(Sample input, Sample invPitchRatio, Sample delayModIn, Sample feedbackModIn)
  {
    spreadDelay.process(input);
    auto &x0 = spreadDelay.output;

    for (size_t i = 0; i < length; ++i) x0[i] += fbSig[i];
    highpass.process(x0);
    lowpass.process(x0);

    lossThreshold.process();
    allpassMod.process();
    allpassQ.process();
    for (size_t idx = 0; idx < nAllpass; ++idx) {
      allpassCut[idx].process();

      allpass[idx].process(x0, allpassMod.value, allpassCut[idx].value, allpassQ.value);

      for (size_t jdx = 0; jdx < length; ++jdx) {
        if (std::abs(x0[jdx]) >= lossThreshold.getValue())
          allpassMod.resetAt(jdx, allpassMod.value[jdx] * Sample(0.99));
      }
    }

    timeMod.process();
    timeSamples.process();
    for (size_t idx = 0; idx < length; ++idx) {
      tmp[idx] = timeSamples.value[idx] * invPitchRatio
        * std::exp2(std::min(timeMod.getValue() * (x0[idx] + delayModIn), Sample(1)));
    }
    timeLimiter.process(tmp, timeRate.process());

    fbSig = x0;
    delay.process(fbSig, timeLimiter.value);

    feedbackGain.process();
    for (size_t idx = 0; idx < length; ++idx) {
      fbSig[idx] *= std::min(feedbackGain.value[idx] * feedbackModIn, Sample(1));
    }

    return spreadDelay.sum();
  }
};

} // namespace SomeDSP
