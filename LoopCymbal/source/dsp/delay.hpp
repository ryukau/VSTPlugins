// (c) 2023 Takamitsu Endo
//
// This file is part of LoopCymbal.
//
// LoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace SomeDSP {

template<typename Sample> class ExpDecay {
private:
  Sample value = Sample(0);
  Sample alpha = Sample(0);

public:
  void setTime(Sample decayTimeInSamples)
  {
    alpha = std::pow(
      Sample(std::numeric_limits<float>::epsilon()), Sample(1) / decayTimeInSamples);
  }

  void reset() { value = Sample(0); }
  void noteOn() { value = Sample(1); }
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

template<typename Sample, size_t nAllpass> class SerialAllpass {
private:
  std::array<Sample, nAllpass> buffer{};
  std::array<Delay<Sample>, nAllpass> delay;
  std::array<EmaHighShelf<Sample>, nAllpass> lowpass;

public:
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
  }

  Sample process(
    Sample input,
    Sample highShelfCut,
    Sample highShelfGain,
    Sample gain,
    Sample timeModAmount)
  {
    for (size_t idx = 0; idx < nAllpass; ++idx) {
      auto x0 = lowpass[idx].process(input, highShelfCut, highShelfGain);
      x0 -= gain * buffer[idx];
      input = buffer[idx] + gain * x0;
      buffer[idx]
        = delay[idx].process(x0, timeInSamples[idx] - timeModAmount * std::abs(x0));
    }
    return input;
  }
};

} // namespace SomeDSP
