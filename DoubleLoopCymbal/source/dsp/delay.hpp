// (c) 2024 Takamitsu Endo
//
// This file is part of DoubleLoopCymbal.
//
// DoubleLoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DoubleLoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DoubleLoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "../../lib/LambertW/LambertW.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

namespace SomeDSP {

template<typename Sample> class ExpDecay {
public:
  Sample value = 0;
  Sample alpha = 0;

  void setTime(Sample decayTimeInSamples)
  {
    constexpr auto eps = Sample(std::numeric_limits<float>::epsilon());
    alpha = std::pow(eps, Sample(1) / decayTimeInSamples);
  }

  void reset() { value = 0; }
  void trigger(Sample gain = Sample(1)) { value = gain; }
  Sample process() { return value *= alpha; }
};

template<typename Sample> class ExpSREnvelope {
public:
  Sample smooth = 0;
  Sample kp = Sample(1);
  Sample value = 0;
  Sample alpha = 0;

  void setSmooth(Sample smoothKp) { kp = smoothKp; }

  void setTime(Sample decayTimeInSamples, bool sustain)
  {
    constexpr auto eps = Sample(std::numeric_limits<float>::epsilon());
    alpha = sustain ? Sample(1) : std::pow(eps, Sample(1) / decayTimeInSamples);
  }

  void reset()
  {
    value = 0;
    smooth = 0;
  }

  void trigger() { value = Sample(1); }

  Sample process()
  {
    value *= alpha;
    return smooth += kp * (value - smooth);
  }
};

template<typename Sample> class ExpDSREnvelope {
public:
  enum class State { decay, release };

private:
  static constexpr auto eps = Sample(std::numeric_limits<float>::epsilon());
  Sample timeD = Sample(1);
  Sample value = 0;
  Sample alphaD = 0;
  Sample alphaR = 0;
  Sample offset = 0;
  State state = State::release;

public:
  void setTime(Sample decayTimeInSamples, Sample releaseTimeInSamples)
  {
    alphaD = std::pow(eps, Sample(1) / decayTimeInSamples);
    timeD = decayTimeInSamples;
    alphaR = std::pow(eps, Sample(1) / releaseTimeInSamples);
  }

  void reset()
  {
    value = 0;
    alphaD = 0;
    alphaR = 0;
    offset = 0;
    state = State::release;
  }

  // `decayScaler` must be greater than 0.
  void trigger(Sample sustainLevel, Sample decayScaler)
  {
    state = State::decay;
    value = Sample(1) - sustainLevel;
    // alphaD = std::pow(eps, Sample(1) / (timeD * decayScaler));
    offset = sustainLevel;
  }

  void release()
  {
    state = State::release;
    offset = 0;
  }

  Sample process()
  {
    if (state == State::decay) {
      value *= alphaD;
      return offset + value;
    }
    return value *= alphaR;
  }
};

template<typename Sample> class TransitionReleaseSmoother {
private:
  Sample v0 = 0;
  Sample decay = 0;

public:
  // decaySamples = sampleRate * seconds.
  void setup(Sample decaySamples)
  {
    decay = std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples);
  }

  void reset() { v0 = 0; }

  void prepare(Sample value, Sample decaySamples)
  {
    v0 += value;
    decay = std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples);
  }

  Sample process() { return v0 *= decay; }
};

template<typename Sample> class ExpADEnvelope {
private:
  static constexpr Sample epsilon = std::numeric_limits<Sample>::epsilon();
  Sample targetGain = 0;
  Sample velocity = 0;
  Sample gain = Sample(1);
  Sample smoo = Sample(1);
  Sample valueA = 0;
  Sample alphaA = 0;
  Sample valueD = 0;
  Sample alphaD = 0;

public:
  bool isTerminated() { return valueD <= Sample(1e-3); }
  void setup(Sample smoothingKp) { smoo = smoothingKp; }

  void reset()
  {
    targetGain = 0;
    gain = Sample(1);
    valueA = 0;
    alphaA = 0;
    valueD = 0;
    alphaD = 0;
  }

  enum class NormalizationType { peak, energy };

  void update(
    Sample sampleRate,
    Sample peakSeconds,
    Sample releaseSeconds,
    Sample peakGain,
    NormalizationType normalization = NormalizationType::energy)
  {
    const auto decaySeconds = releaseSeconds - std::log(epsilon) * peakSeconds;
    const auto d_ = std::log(epsilon) / decaySeconds;
    const auto x_ = d_ * peakSeconds;
    const auto a_ = Sample(utl::LambertW(-1, x_ * std::exp(x_))) / peakSeconds - d_;

    const auto attackSeconds = -std::log(epsilon) / std::log(-a_);
    alphaA = std::exp(a_ / sampleRate);
    alphaD = std::exp(d_ / sampleRate);

    if (normalization == NormalizationType::energy) {
      // `area` is obtained by solving `integrate((1-%e^(-a*t))*%e^(-d*t), t, 0, +inf);`.
      const auto area = -a_ / (d_ * (d_ + a_));
      targetGain = Sample(1e-1) * peakGain / area;
    } else { // `normalization == NormalizationType::peak`.
      targetGain
        = peakGain / (-std::expm1(a_ * peakSeconds) * std::exp(d_ * peakSeconds));
    }
  }

  void trigger(
    Sample sampleRate,
    Sample peakSeconds,
    Sample releaseSeconds,
    Sample peakGain,
    Sample velocity_)
  {
    velocity = velocity_;
    valueA = Sample(1);
    valueD = Sample(1);
    update(sampleRate, peakSeconds, releaseSeconds, peakGain);
  }

  Sample process()
  {
    gain += smoo * (targetGain - gain);
    valueA *= alphaA;
    valueD *= alphaD;
    return velocity * gain * (Sample(1) - (valueA)) * valueD;
  }
};

template<typename T> inline T lagrange3Interp(T y0, T y1, T y2, T y3, T t)
{
  auto u = T(1) + t;
  auto d0 = y0 - y1;
  auto d1 = d0 - (y1 - y2);
  auto d2 = d1 - ((y1 - y2) - (y2 - y3));
  return y0 - u * (d0 + (T(1) - u) / T(2) * (d1 + (T(2) - u) / T(3) * d2));
}

template<typename Sample> class Delay {
private:
  int wptr = 0;
  std::vector<Sample> buf{Sample(0), Sample(0)};

public:
  void setup(Sample maxTimeSamples)
  {
    buf.resize(std::max(size_t(4), size_t(maxTimeSamples) + 4));
    reset();
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  void applyGain(Sample gain)
  {
    for (auto &x : buf) x *= gain;
  }

  Sample process(Sample input, Sample timeInSamples)
  {
    const int size = int(buf.size());
    const int clamped
      = std::clamp(timeInSamples - Sample(1), Sample(1), Sample(size - 4));
    const int timeInt = int(clamped);
    const Sample rFraction = clamped - Sample(timeInt);

    // Write to buffer.
    if (++wptr >= size) wptr = 0;
    buf[wptr] = input;

    // Read from buffer.
    auto rptr0 = wptr - timeInt;
    auto rptr1 = rptr0 - 1;
    auto rptr2 = rptr0 - 2;
    auto rptr3 = rptr0 - 3;
    if (rptr0 < 0) rptr0 += size;
    if (rptr1 < 0) rptr1 += size;
    if (rptr2 < 0) rptr2 += size;
    if (rptr3 < 0) rptr3 += size;
    return lagrange3Interp(buf[rptr0], buf[rptr1], buf[rptr2], buf[rptr3], rFraction);
  }
};

template<typename Sample> class Highpass2 {
private:
  // static constexpr Sample g = Sample(0.02);               // ~= tan(pi * 300 / 48000).
  static constexpr Sample k = Sample(0.7071067811865476); // 2 / sqrt(2).

  Sample ic1eq = 0;
  Sample ic2eq = 0;

public:
  void reset()
  {
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample process(Sample input, Sample cutoffNormalized)
  {
    const auto g = std::tan(
      std::numbers::pi_v<Sample>
      * std::clamp(cutoffNormalized, Sample(0.00001), Sample(0.49998)));
    const auto v1 = (ic1eq + g * (input - ic2eq)) / (1 + g * (g + k));
    const auto v2 = ic2eq + g * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return input - k * v1 - v2;
  }
};

template<typename Sample> class EmaHighShelf {
private:
  Sample value = 0;

public:
  void reset() { value = 0; }

  Sample process(Sample input, Sample kp, Sample shelvingGain)
  {
    value += kp * (input - value);
    return std::lerp(value, input, shelvingGain);
  }
};

template<typename Sample> class EmaLowShelf {
private:
  Sample value = 0;

public:
  void reset() { value = 0; }

  Sample process(Sample input, Sample kp, Sample shelvingGain)
  {
    value += kp * (input - value);
    return std::lerp(input - value, input, shelvingGain);
  }
};

template<typename Sample, size_t nAllpass> class SerialAllpass {
private:
  std::array<Sample, nAllpass> buffer{};
  std::array<Delay<Sample>, nAllpass> delay;
  std::array<EmaHighShelf<Sample>, nAllpass> lowpass;
  std::array<EmaLowShelf<Sample>, nAllpass> highpass;

public:
  static constexpr size_t size = nAllpass;
  size_t nDelay = nAllpass;
  std::array<Sample, nAllpass> timeInSamples{};

  void setup(Sample maxTimeSamples)
  {
    for (auto &x : delay) x.setup(maxTimeSamples);
  }

  void reset()
  {
    buffer.fill({});
    for (auto &x : delay) x.reset();
    for (auto &x : lowpass) x.reset();
    for (auto &x : highpass) x.reset();
  }

  void applyGain(Sample gain)
  {
    for (auto &x : delay) x.applyGain(gain);
  }

  Sample sum(Sample altSignMix)
  {
    Sample sumAlt = Sample(0);
    Sample sign = Sample(1);
    for (size_t i = 0; i < nDelay; ++i) {
      sumAlt += buffer[i] * sign;
      sign = -sign;
    }
    Sample sumDirect
      = std::accumulate(buffer.begin(), buffer.begin() + nDelay, Sample(0));
    return std::lerp(sumDirect, sumAlt, altSignMix) / (Sample(2) * nDelay);
  }

  Sample process(
    Sample input,
    Sample highShelfCut,
    Sample highShelfGain,
    Sample lowShelfCut,
    Sample lowShelfGain,
    Sample apGain,
    Sample delayGain,
    Sample pitchRatio,
    Sample timeModAmount)
  {
    for (size_t idx = 0; idx < nDelay; ++idx) {
      constexpr auto sign = 1;
      auto x0 = lowpass[idx].process(sign * input, highShelfCut, highShelfGain);
      x0 = highpass[idx].process(x0, lowShelfCut, lowShelfGain);
      x0 -= apGain * buffer[idx];
      input = buffer[idx] + apGain * x0;
      buffer[idx] = delay[idx].process(
        delayGain * x0, timeInSamples[idx] / pitchRatio - timeModAmount * std::abs(x0));
    }
    return input;
  }
};

template<typename Sample> class HalfClosedNoise {
private:
  Sample phase = 0;
  Sample gain = Sample(1);
  Sample decay = 0;

  Highpass2<Sample> highpass;

public:
  void reset()
  {
    phase = 0;
    gain = Sample(1);

    highpass.reset();
  }

  void setDecay(Sample timeInSample)
  {
    constexpr Sample eps = std::numeric_limits<Sample>::epsilon();
    decay = timeInSample < Sample(1) ? 0 : std::pow(eps, Sample(1) / timeInSample);
  }

  // `density` is inverse of average samples between impulses.
  // `randomGain` is in [0, 1].
  Sample process(Sample density, Sample randomGain, Sample highpassNormalized, pcg64 &rng)
  {
    std::uniform_real_distribution<Sample> jitter(Sample(0), Sample(1));
    phase += jitter(rng) * density;
    if (phase >= Sample(1)) {
      phase -= std::floor(phase);

      std::normal_distribution<Sample> distGain(Sample(0), Sample(1) / Sample(3));
      gain = Sample(1) + randomGain * (distGain(rng) - Sample(1));
    } else {
      gain *= decay;
    }

    std::uniform_real_distribution<Sample> distNoise(-Sample(1), Sample(1));
    const auto noise = distNoise(rng);
    return highpass.process(noise * noise * noise * gain, highpassNormalized);
  }
};

// Stereo spreader.
// 2-band splitter is made from bilinear transformed 1-pole filters.
template<typename Sample> class Spreader {
private:
  Sample x1 = 0;
  Sample y1Lp = 0;
  Sample y2Lp = 0;
  Sample y1Hp = 0;
  Sample y2Hp = 0;

  Sample baseTime = double(1);
  Delay<Sample> delay;

public:
  void setup(Sample maxTimeSample) { delay.setup(maxTimeSample); }
  void updateBaseTime(Sample maxTimeSample) { baseTime = Sample(0.5) * maxTimeSample; }

  void reset()
  {
    x1 = 0;
    y1Lp = 0;
    y2Lp = 0;
    y1Hp = 0;
    y2Hp = 0;

    delay.reset();
  }

  std::array<Sample, 2> process(Sample input, Sample splitFreqNormalized, Sample spread)
  {
    // 2-band splitter.
    constexpr auto minFreq = Sample(0.00001);
    constexpr auto nyquist = Sample(0.49998);
    const auto cut = std::clamp(splitFreqNormalized, minFreq, nyquist);
    const auto kp = Sample(1) / std::tan(std::numbers::pi_v<Sample> * cut);
    const auto a0 = Sample(1) + kp;
    const auto bLp = Sample(1) / a0;
    const auto bHp = kp / a0;
    const auto a1 = (Sample(1) - kp) / a0;

    const auto x2Lp = y1Lp;
    const auto x2Hp = y1Hp;

    y1Lp = bLp * (input + x1) - a1 * y1Lp;
    y1Hp = bHp * (input - x1) - a1 * y1Hp;

    y2Lp = bLp * (y1Lp + x2Lp) - a1 * y2Lp;
    y2Hp = bHp * (y1Hp - x2Hp) - a1 * y2Hp;

    x1 = input;

    // Spared stereo with delay.
    const auto delayed = delay.process(
      y2Hp * spread / (-std::numbers::sqrt2_v<Sample>), (spread + Sample(1)) * baseTime);
    const auto merged = y2Lp - y2Hp;
    return {merged + delayed, merged - delayed};
  }
};

} // namespace SomeDSP
