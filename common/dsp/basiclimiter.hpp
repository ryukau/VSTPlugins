// (c) 2022 Takamitsu Endo
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
#include <random>
#include <vector>

namespace SomeDSP {

// Integer sample delay.
template<typename Sample> class IntDelay {
private:
  std::vector<Sample> buf;
  size_t wptr = 0;
  size_t rptr = 0;

public:
  IntDelay(size_t size = 64) : buf(size) {}

  void resize(size_t size)
  {
    buf.resize(size + 1);
    wptr = 0;
    rptr = 0;
  }

  void reset() { std::fill(buf.begin(), buf.end(), Sample(0)); }

  void setFrames(size_t delayFrames)
  {
    if (delayFrames >= buf.size()) delayFrames = buf.size();
    rptr = wptr - delayFrames;
    if (rptr >= buf.size()) rptr += buf.size(); // Unsigned negative overflow case.
  }

  Sample process(Sample input)
  {
    if (++wptr >= buf.size()) wptr = 0;
    buf[wptr] = input;

    if (++rptr >= buf.size()) rptr = 0;
    return buf[rptr];
  }
};

// Replacement of std::deque with reduced memory allocation.
template<typename T> struct RingQueue {
  std::vector<T> buf;
  size_t wptr = 0;
  size_t rptr = 0;

  RingQueue(size_t size = 64) : buf(size) {}

  void resize(size_t size) { buf.resize(size); }

  void reset(T value = 0)
  {
    std::fill(buf.begin(), buf.end(), value);
    wptr = 0;
    rptr = 0;
  }

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
  IntDelay<Sample> delay;
  RingQueue<Sample> queue;

  PeakHold(size_t size = 64)
  {
    resize(size);
    setFrames(1);
  }

  void resize(size_t size)
  {
    delay.resize(size);
    queue.resize(size);
  }

  void reset()
  {
    delay.reset();
    queue.reset();
  }

  void setFrames(size_t frames) { delay.setFrames(frames); }

  Sample process(Sample x0)
  {
    while (!queue.empty()) {
      if (queue.back() >= x0) break;
      queue.pop_back();
    }
    queue.push_back(x0);
    if (delay.process(x0) == queue.front()) queue.pop_front();
    return queue.front();
  }
};

template<typename Sample, bool fastSmoothing = true> class DoubleAverageFilter {
private:
  Sample denom = Sample(1);
  Sample sum1 = 0;
  Sample sum2 = 0;
  Sample buf = 0;
  IntDelay<Sample> delay1;
  IntDelay<Sample> delay2;

public:
  void resize(size_t size)
  {
    delay1.resize(size / 2 + 1);
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
    auto &&half = frames / 2;
    denom = 1 / Sample((half + 1) * half);
    delay1.setFrames(half + 1);
    delay2.setFrames(half);
  }

  inline Sample add(Sample lhs, Sample rhs)
  {
    if constexpr (fastSmoothing) return lhs + rhs;

    if (lhs < rhs) std::swap(lhs, rhs);
    int expL;
    std::frexp(lhs, &expL);
    auto &&cut = std::ldexp(float(1), expL - std::numeric_limits<Sample>::digits);
    auto &&rounded = rhs - std::fmod(rhs, cut);
    return lhs + rounded;
  }

  Sample process(Sample input)
  {
    input *= denom;

    sum1 = add(sum1, input);
    Sample d1 = delay1.process(input);
    sum1 = std::max(Sample(0), sum1 - d1);

    sum2 = add(sum2, sum1);
    Sample d2 = delay2.process(sum1);
    sum2 = std::max(Sample(0), sum2 - d2);

    auto output = buf;
    buf = sum2;
    return output;
  }
};

template<typename Sample, bool fastSmoothing = true> class BasicLimiter {
private:
  size_t attackFrames = 0;
  Sample thresholdAmp = Sample(1); // thresholdAmp > 0.

  PeakHold<Sample> peakhold;
  DoubleAverageFilter<double, fastSmoothing> smoother;
  DoubleEMAFilter<Sample> releaseFilter;
  IntDelay<Sample> lookaheadDelay;

public:
  size_t latency(size_t upfold) { return attackFrames / upfold; }

  void resize(size_t size)
  {
    size += size % 2; // DoubleAverageFilter requires multiple of 2.
    peakhold.resize(size);
    smoother.resize(size);
    lookaheadDelay.resize(size);
  }

  void reset()
  {
    peakhold.reset();
    smoother.reset();
    releaseFilter.reset();
    lookaheadDelay.reset();
  }

  void prepare(
    Sample sampleRate,
    Sample attackSeconds,
    Sample releaseSeconds,
    Sample thresholdAmplitude)
  {
    auto prevAttack = attackFrames;
    attackFrames = size_t(sampleRate * attackSeconds + Sample(0.5));
    attackFrames += attackFrames % 2; // DoubleAverageFilter requires multiple of 2.

    if (prevAttack != attackFrames) reset();

    releaseFilter.setCutoff(sampleRate, Sample(1) / releaseSeconds);

    thresholdAmp = thresholdAmplitude;

    peakhold.setFrames(attackFrames);
    smoother.setFrames(attackFrames);
    lookaheadDelay.setFrames(attackFrames);
  }

  inline Sample applyCharacteristicCurve(Sample peakAmp)
  {
    return peakAmp > thresholdAmp ? thresholdAmp / peakAmp : Sample(1);
  }

  inline Sample processRelease(Sample gain)
  {
    releaseFilter.setMin(gain);
    return releaseFilter.process(gain);
  }

  Sample process(const Sample input)
  {
    auto inAbs = std::fabs(input);
    auto peakAmp = peakhold.process(inAbs);
    auto candidate = applyCharacteristicCurve(peakAmp);
    auto released = processRelease(candidate);
    auto gainAmp = std::min(released, candidate);
    auto smoothed = smoother.process(gainAmp);
    auto delayed = lookaheadDelay.process(input);
    return smoothed * delayed;
  }
};

} // namespace SomeDSP
