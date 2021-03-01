// (c) 2021 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <random>
#include <vector>

namespace SomeDSP {

template<typename Sample> struct SOCPFIR {
  constexpr static size_t bufferSize = 7;
  constexpr static size_t intDelay = 3;

  constexpr static std::array<std::array<Sample, bufferSize>, 4> coefficient{{
    {Sample(0.03396642725330925), Sample(-0.12673821137646601),
     Sample(0.5759982312324312), Sample(0.6592123095604063), Sample(-0.19435321143573606),
     Sample(0.0782612693103079), Sample(-0.025807862651826587)},
    {Sample(0.021616078095824397), Sample(-0.07539816970638001),
     Sample(0.2653441329619578), Sample(0.9081714824861011), Sample(-0.16017585860369898),
     Sample(0.059489586593950955), Sample(-0.018863293456169244)},
    {Sample(-0.018863293456169286), Sample(0.05948958659395098),
     Sample(-0.16017585860369907), Sample(0.908171482486101), Sample(0.2653441329619578),
     Sample(-0.07539816970638011), Sample(0.02161607809582444)},
    {Sample(-0.02580786265182662), Sample(0.07826126931030812),
     Sample(-0.1943532114357363), Sample(0.6592123095604064), Sample(0.5759982312324308),
     Sample(-0.12673821137646582), Sample(0.033966427253309124)},
  }};
};

template<typename Sample, typename FractionalDelayFIR> class TruePeakMeterFIR {
  std::array<Sample, FractionalDelayFIR::bufferSize> buf{};

public:
  void reset() { buf.fill(Sample(0)); }

  Sample process(Sample input)
  {
    for (size_t i = 0; i < buf.size() - 1; ++i) buf[i] = buf[i + 1];
    buf.back() = input;

    Sample max = std::fabs(buf[FractionalDelayFIR::intDelay]);
    for (const auto &phase : FractionalDelayFIR::coefficient) {
      Sample sum = 0;
      for (size_t i = 0; i < phase.size(); ++i) sum += buf[i] * phase[i];
      max = std::max(max, std::fabs(sum));
    }
    return max;
  }
};

// Integer sample delay.
template<typename Sample> class IntDelay {
public:
  std::vector<Sample> buf;
  size_t wptr = 0;
  size_t rptr = 0;

  IntDelay(size_t size = 65536) : buf(size) {}

  void resize(size_t size)
  {
    buf.resize(size);
    wptr = 0;
    rptr = 0;
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  void setFrames(size_t delayFrames)
  {
    if (delayFrames >= buf.size()) delayFrames = buf.size();
    rptr = wptr - delayFrames;
    if (rptr >= buf.size()) rptr += buf.size(); // Unsigned overflow case.
  }

  Sample process(Sample input)
  {
    if (++wptr >= buf.size()) wptr -= buf.size();
    buf[wptr] = input;

    if (++rptr >= buf.size()) rptr -= buf.size();
    return buf[rptr];
  }
};

// Replacement of std::deque with reduced memory allocation.
template<typename T> struct RingQueue {
  std::vector<T> buf;
  size_t wptr = 0;
  size_t rptr = 0;

  void resize(size_t size) { buf.resize(size); }

  void reset(T value = 0) { std::fill(buf.begin(), buf.end(), value); }

  inline size_t size()
  {
    auto sz = wptr - rptr;
    if (sz >= buf.size()) sz += buf.size(); // Unsigned overflow case.
    return sz;
  }

  inline bool empty() { return wptr == rptr; }

  T &front() { return buf[increment(rptr)]; }
  T &back() { return buf[wptr]; }

  inline size_t increment(size_t idx)
  {
    if (++idx >= buf.size()) idx -= buf.size();
    return idx;
  }

  inline size_t decrement(size_t idx)
  {
    if (--idx >= buf.size()) idx += buf.size(); // Unsigned overflow case.
    return idx;
  }

  void push_back(T value)
  {
    wptr = increment(wptr);
    buf[wptr] = value;
  }

  T pop_front()
  {
    rptr = increment(rptr);
    return buf[rptr];
  }

  T pop_back()
  {
    wptr = decrement(wptr);
    return buf[wptr];
  }
};

/*
Ideal peak hold.
- When `setFrames(0)`, all output becomes 0.
- When `setFrames(1)`, PeakHold will bypass the input.
*/
template<typename Sample> struct PeakHold {
  Sample neutral = 0;
  IntDelay<Sample> delay;
  RingQueue<Sample> hold;

  PeakHold(size_t size = 65536)
  {
    resize(size);
    setFrames(1);
  }

  void resize(size_t size)
  {
    delay.resize(size);
    hold.resize(size);
  }

  void reset()
  {
    delay.reset();
    hold.reset(neutral);
  }

  void setFrames(size_t frames) { delay.setFrames(frames); }

  Sample process(Sample x0)
  {
    if (!hold.empty()) {
      for (size_t idx = hold.size(); idx > 0; --idx) {
        if (hold.back() < x0)
          hold.pop_back();
        else
          break;
      }
    }

    hold.push_back(x0);

    auto delayOut = delay.process(x0);
    if (!hold.empty() && delayOut == hold.front()) hold.pop_front();

    return !hold.empty() ? hold.front() : neutral;
  }
};

template<typename Sample> struct DoubleAverageFilter {
  Sample sum1 = 0;
  Sample sum2 = 0;
  Sample buf = 0; // A delay to fix 1 sample forward.
  size_t halfDelayFrames = 0;
  IntDelay<Sample> delay1;
  IntDelay<Sample> delay2;

  void resize(size_t size)
  {
    delay1.resize(size / 2);
    delay2.resize(size / 2);
  }

  void reset()
  {
    sum1 = 0;
    sum2 = 0;
    buf = 0;
    delay1.reset();
    delay2.reset();
  }

  void setFrames(size_t frames)
  {
    halfDelayFrames = frames / 2;
    delay1.setFrames(halfDelayFrames);
    delay2.setFrames(halfDelayFrames);
  }

  Sample process(const Sample input)
  {
    sum1 += buf - delay1.process(buf);
    auto out1 = sum1 / halfDelayFrames;

    sum2 += out1 - delay2.process(out1);
    auto out2 = sum2 / halfDelayFrames;

    buf = input;
    return out2;
  }
};

template<typename Sample> struct Limiter {
  static constexpr Sample fixedGain = Sample(0.9965520801347684); // -0.03dB.
  static constexpr Sample releaseConstant = Sample(1e-5);

  ExpSmoother<Sample> interpThreshold;
  ExpSmoother<Sample> interpGate;
  Sample gain = Sample(1);
  Sample release = 0;
  size_t attackFrames = 0;

  PeakHold<Sample> hold;
  DoubleAverageFilter<Sample> smoother;
  IntDelay<Sample> lookaheadDelay;

  uint32_t latency() { return uint32_t(attackFrames); }

  void resize(size_t size)
  {
    size += size % 2;
    hold.resize(size);
    smoother.resize(size);
    lookaheadDelay.resize(size);
  }

  void reset(Sample threshold, Sample gate)
  {
    gain = Sample(1);
    hold.reset();
    smoother.reset();
    lookaheadDelay.reset();
    interpThreshold.reset(threshold);
    interpGate.reset(gate);
  }

  void prepare(
    Sample sampleRate,
    Sample attackSeconds,
    Sample sustainSeconds,
    Sample releaseSeconds,
    Sample threshold,
    Sample gate)
  {
    auto prevAttack = attackFrames;
    attackFrames = size_t(sampleRate * attackSeconds);
    attackFrames += attackFrames % 2; // DoubleAverageFilter requires multiple of 2.
    if (prevAttack != attackFrames) reset(threshold, gate);

    release
      = std::pow(Sample(1 / releaseConstant), Sample(1 / (releaseSeconds * sampleRate)));

    interpThreshold.push(threshold);
    interpGate.push(gate);

    hold.setFrames(attackFrames + size_t(sampleRate * sustainSeconds));
    smoother.setFrames(attackFrames);
    lookaheadDelay.setFrames(attackFrames);
  }

  inline Sample applyCharacteristicCurve(Sample x0, Sample threshold)
  {
    return x0 > threshold ? threshold / x0 : Sample(1);
  }

  inline Sample softClip(Sample x0, Sample ratio, Sample threshold)
  {
    const auto absed = std::fabs(x0);

    const auto a1 = threshold * ratio;
    if (absed <= a1) return x0;

    const auto a2 = 2 * threshold - a1;
    if (absed >= a2) return threshold;

    return std::copysign(
      threshold + (a2 - absed) * (a2 - absed) * Sample(0.25) / (a1 - threshold), x0);
  }

  Sample process(const Sample input, Sample inAbs)
  {
    const auto threshold = interpThreshold.process();
    const auto gate = interpGate.process();

    auto holdGain = hold.process(inAbs);
    auto candidate = applyCharacteristicCurve(holdGain, threshold);
    gain = std::min(gain * release, candidate);
    auto gateGain = inAbs < gate ? Sample(0) : Sample(1);

    auto smoothed = smoother.process(gateGain * gain);
    auto delayed = lookaheadDelay.process(input);
    return softClip(smoothed * delayed, fixedGain, threshold);
  }
};

} // namespace SomeDSP
