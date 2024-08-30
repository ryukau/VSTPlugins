// (c) 2023 Takamitsu Endo
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

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

namespace SomeDSP {

template<typename Sample> class ExpDecay {
private:
  Sample value = Sample(0);
  Sample alpha = Sample(0);

public:
  void setTime(Sample decayTimeInSamples, bool sustain = false)
  {
    constexpr auto eps = Sample(std::numeric_limits<float>::epsilon());
    alpha = sustain ? Sample(1) : std::pow(eps, Sample(1) / decayTimeInSamples);
  }

  void reset() { value = Sample(0); }
  void noteOn(Sample gain = Sample(1)) { value = gain; }
  Sample process() { return value *= alpha; }
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

/**
Unused. This is a drop in replacement of `Delay`, but doubles the CPU load.

- Sustain of the sound becomes shorter. Perhaps second allpass loop is required.
- Rotating `phase` at right frequency makes better sound. It adds artifact due to AM.
- `inputRatio` is better to be controled by user.
*/
template<typename Sample> class FDN2 {
public:
  static constexpr size_t size = 2;

  Sample phase = 0;
  std::array<Sample, size> buffer;
  std::array<Delay<Sample>, size> delay;

  void setup(Sample maxTimeSamples)
  {
    for (auto &x : delay) x.setup(maxTimeSamples);
  }

  void reset()
  {
    buffer.fill({});
    for (auto &x : delay) x.reset();
  }

  Sample process(Sample input, Sample timeInSamples)
  {
    // Fixed parameters. [a, b] is the range of the value.
    constexpr auto phaseRatio = Sample(1) / Sample(3); // [positive small, +inf].
    constexpr auto feedback = Sample(1);               // [-1, 1].
    constexpr auto timeRatio = Sample(4) / Sample(3);  // [0, +inf].
    constexpr auto inputRatio = Sample(0.5);           // [0, 1].

    constexpr auto twopi = Sample(2) * std::numbers::pi_v<Sample>;
    const auto cs = std::cos(twopi * phase);
    const auto sn = std::sin(twopi * phase);

    phase += phaseRatio / timeInSamples;
    phase -= std::floor(phase);

    const auto sig0 = feedback * (cs * buffer[0] - sn * buffer[1]);
    const auto sig1 = feedback * (sn * buffer[0] + cs * buffer[1]);

    constexpr auto g1 = inputRatio;
    constexpr auto g2 = Sample(1) - inputRatio;
    buffer[0] = delay[0].process(g1 * input + sig0, timeInSamples);
    buffer[1] = delay[1].process(g2 * input + sig1, timeInSamples * timeRatio);

    return buffer[0] + buffer[1];
  }
};

// Unused. `SerialAllpass` became unstable when used.
template<typename Sample> class NyquistLowpass {
private:
  static constexpr Sample g = Sample(15915.494288237813); // tan(0.49998 * pi).
  static constexpr Sample k = Sample(0.7071067811865476); // 2 / sqrt(2).

  Sample ic1eq = 0;
  Sample ic2eq = 0;

public:
  void reset()
  {
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample process(Sample input)
  {
    const auto v1 = (ic1eq + g * (input - ic2eq)) / (1 + g * (g + k));
    const auto v2 = ic2eq + g * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v2;
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

// Unused.
template<typename Sample> class AdaptiveNotchAM {
public:
  static constexpr Sample mu = Sample(2) / Sample(256);
  Sample alpha = Sample(-2);

  Sample x1 = 0;
  Sample x2 = 0;
  Sample y1 = 0;
  Sample y2 = 0;

  void reset()
  {
    alpha = Sample(-2); // 0 Hz as initial guess.

    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
  }

  Sample process(Sample input, Sample narrowness)
  {
    const auto a1 = narrowness * alpha;
    const auto a2 = narrowness * narrowness;
    const auto gain = alpha >= 0 ? (Sample(1) + a1 + a2) / (Sample(2) + alpha)
                                 : (Sample(1) - a1 + a2) / (Sample(2) - alpha);

    constexpr auto clip = Sample(1) / std::numeric_limits<Sample>::epsilon();
    const auto y0 = std::clamp(input + alpha * x1 + x2 - a1 * y1 - a2 * y2, -clip, clip);
    const auto s0 = x1 * (Sample(1) - narrowness * y0);
    constexpr auto bound = Sample(2);
    alpha += Sample(0.0625) * (std::clamp(alpha - mu * y0 * s0, -bound, bound) - alpha);

    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = y0;

    return y0 * gain;
  }
};

// Unused. Unstable when put in feedback loop.
template<typename Sample> class AdaptiveNotchAP1 {
public:
  Sample alpha = Sample(-2);

  Sample v1 = 0;
  Sample v2 = 0;
  Sample q1 = 0;
  Sample r1 = 0;

  void reset()
  {
    alpha = Sample(-2); // 0 Hz as initial guess.

    v1 = 0;
    v2 = 0;
    q1 = 0;
    r1 = 0;
  }

  inline Sample approxAtoK(Sample x)
  {
    constexpr std::array<Sample, 3> b{
      Sample(0.24324073816096012), Sample(-0.2162512785673542),
      Sample(0.0473854827531673)};
    constexpr std::array<Sample, 3> a{
      Sample(-0.9569399371569998), Sample(0.23899241566503945),
      Sample(-0.005191170339007643)};

    const auto z = std::abs(x);
    const auto y
      = z * (b[0] + z * (b[1] + z * b[2])) / (1 + z * (a[0] + z * (a[1] + z * a[2])));
    return std::copysign(y, x);
  }

  Sample process(Sample input, Sample narrowness)
  {
    const auto a1 = narrowness * alpha;
    const auto a2 = narrowness * narrowness;
    const auto gain = alpha >= 0 ? (Sample(1) + a1 + a2) / (Sample(2) + alpha)
                                 : (Sample(1) - a1 + a2) / (Sample(2) - alpha);

    constexpr auto clip = Sample(1024);
    const auto x0 = std::clamp(input, -clip, clip);
    auto v0 = x0 - a1 * v1 - a2 * v2;
    const auto y0 = v0 + alpha * v1 + v2;
    const auto s0
      = (Sample(1) - narrowness) * v0 - narrowness * (Sample(1) - narrowness) * v2;
    const auto alphaCpz = y0 * s0;

    const auto omega_a = std::numbers::pi_v<Sample> - std::acos(alpha / 2);
    const auto t = std::tan(omega_a / Sample(2));
    const auto k_ap = (t - Sample(1)) / (t + Sample(1));
    // const auto k_ap = approxAtoK(alpha);
    r1 = k_ap * (x0 - r1) + q1;
    q1 = x0;
    const auto alphaAM = std::min(std::abs(y0), Sample(1)) * x0 * r1;

    constexpr auto bound = Sample(2);
    constexpr auto mu = Sample(1) / Sample(256);
    alpha = std::clamp(alpha - mu * alphaAM, -bound, bound);
    // alpha = std::clamp(alpha - mu * (alphaCpz + alphaAM), -bound, bound);

    v2 = v1;
    v1 = v0;

    return y0 * gain;
  }
};

template<typename Sample> class AdaptiveNotchCPZ {
public:
  static constexpr Sample mu = Sample(2) / Sample(1024);
  Sample alpha = Sample(-2);

  Sample v1 = 0;
  Sample v2 = 0;

  void reset()
  {
    alpha = Sample(-2); // 0 Hz as initial guess.

    v1 = 0;
    v2 = 0;
  }

  Sample process(Sample input, Sample narrowness)
  {
    const auto a1 = narrowness * alpha;
    const auto a2 = narrowness * narrowness;
    auto gain = alpha >= 0 ? (Sample(1) + a1 + a2) / (Sample(2) + alpha)
                           : (Sample(1) - a1 + a2) / (Sample(2) - alpha);

    constexpr auto clip = Sample(1) / std::numeric_limits<Sample>::epsilon();
    const auto x0 = std::clamp(input, -clip, clip);
    auto v0 = x0 - a1 * v1 - a2 * v2;
    const auto y0 = v0 + alpha * v1 + v2;
    const auto s0
      = (Sample(1) - narrowness) * v0 - narrowness * (Sample(1) - narrowness) * v2;
    constexpr auto bound = Sample(2);
    alpha = std::clamp(alpha - y0 * s0 * mu, -bound, bound);

    v2 = v1;
    v1 = v0;

    return y0 * gain;
  }
};

template<typename Sample, size_t nAllpass, size_t nAdaptiveNotch> class SerialAllpass {
private:
  std::array<Sample, nAllpass> buffer{};
  std::array<Delay<Sample>, nAllpass> delay;
  std::array<EmaHighShelf<Sample>, nAllpass> lowpass;
  std::array<EmaLowShelf<Sample>, nAllpass> highpass;

public:
  std::array<AdaptiveNotchCPZ<Sample>, nAdaptiveNotch> notch;
  static constexpr size_t size = nAllpass;
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
    for (auto &x : notch) x.reset();
  }

  Sample sum(Sample altSignMix)
  {
    Sample sumAlt = Sample(0);
    Sample sign = Sample(1);
    for (const auto &x : buffer) {
      sumAlt += x * sign;
      sign = -sign;
    }
    Sample sumDirect = std::accumulate(buffer.begin(), buffer.end(), Sample(0));
    return std::lerp(sumDirect, sumAlt, altSignMix) / (Sample(2) * nAllpass);
  }

  Sample process(
    Sample input,
    Sample highShelfCut,
    Sample highShelfGain,
    Sample lowShelfCut,
    Sample lowShelfGain,
    Sample gain,
    Sample pitchRatio,
    Sample timeModAmount,
    size_t nNotch,
    Sample notchMix,
    Sample notchNarrowness)
  {
    for (size_t idx = 0; idx < nAllpass; ++idx) {
      constexpr auto sign = 1;
      auto x0 = lowpass[idx].process(sign * input, highShelfCut, highShelfGain);
      // auto x0 = sign * input;
      x0 = highpass[idx].process(x0, lowShelfCut, lowShelfGain);
      x0 -= gain * buffer[idx];
      input = buffer[idx] + gain * x0;
      buffer[idx] = delay[idx].process(
        x0, timeInSamples[idx] / pitchRatio - timeModAmount * std::abs(x0));
    }

    // input = lowpass[0].process(input, highShelfCut, highShelfGain);

    for (size_t idx = 0; idx < nNotch; ++idx) {
      input += notchMix * (notch[idx].process(input, notchNarrowness) - input);
    }

    return input;
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

} // namespace SomeDSP
