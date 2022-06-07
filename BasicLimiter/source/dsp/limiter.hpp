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

// Integer sample delay.
template<typename Sample> class IntDelay {
private:
  std::vector<Sample> buf;
  size_t wptr = 0;
  size_t rptr = 0;

public:
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

  PeakHold(size_t size = 65536)
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

/**
Double moving average filter.

Output of `process()` is equivalent to the following Python 3 code. `size` is the value
passed to `resize()`.

```python
import scipy.signal as signal
import numpy as np
fir = signal.get_window("bartlett", size + 1)
fir /= np.sum(fir)
output = signal.convolve(getSomeSignal(), fir)
```

For limiter, use `double` for accuracy. Using `float` may cause over-limiting.
Over-limiting here means that if the input amplitude far exceeds threshold, output tends
to be more quiet. This is caused by the rounding of floating point number. Rounding used
in `DoubleAverageFilter` makes float sum to be lower than true sum, and this makes output
gain to be lower than target gain.
*/
template<typename Sample> class DoubleAverageFilter {
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

  /**
  Floating point addition with rounding towards 0 for positive number.
  It must be that `lhs >= 0` and `rhs >= 0`.

  Assuming IEEE 754. It was only tested where
  `std::numeric_limits<float>::round_style == std::round_to_nearest`. On the platform
  using other type of floating point rounding or representation, it may not work, or may
  be unnecessary. Negative number input is not tested.

  Following explanation uses 4 bit significand. Numbers are binary. Consider addition of
  significand like following:

  ```
    1000
  + 0011 11 // last 11 will be rounded.
  ---------
    1???
  ```

  There are 2 possible answer depending on rounding mode: 1100 or 1011.

  This `add()` method outputs 1011 in cases like above, to prevent smoothed output
  exceeds decimal +1.0.

  If `std::numeric_limits<float>::round_style == std::round_to_nearest`, then the number
  will be rounded towards nearest even number. In this case, the answer on above case
  becomes 1100.
  */
  inline Sample add(Sample lhs, Sample rhs)
  {
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

template<typename Sample> class Limiter {
private:
  size_t attackFrames = 0;
  size_t sustainFrames = 0;
  Sample thresholdAmp = Sample(1); // thresholdAmp > 0.
  Sample gateAmp = 0;              // gateAmp >= 0.

  PeakHold<Sample> peakhold;
  DoubleAverageFilter<double> smoother;
  DoubleEMAFilter<Sample> releaseFilter;
  IntDelay<Sample> lookaheadDelay;

public:
  size_t latency(size_t upfold) { return attackFrames / upfold; }

  void resize(size_t size)
  {
    size += size % 2;

    // Assuming `maxAttackTime = maxSustainTime`. Otherwise peakhold requires the size
    // of `maxAttackTime + maxSustainTime`.
    peakhold.resize(2 * size);

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
    Sample sustainSeconds,
    Sample releaseSeconds,
    Sample thresholdAmplitude,
    Sample gateAmplitude)
  {
    auto prevAttack = attackFrames;
    attackFrames = size_t(sampleRate * attackSeconds);
    attackFrames += attackFrames % 2; // DoubleAverageFilter requires multiple of 2.

    auto prevSustain = sustainFrames;
    sustainFrames = size_t(sampleRate * sustainSeconds);

    if (prevAttack != attackFrames || prevSustain != sustainFrames) reset();

    releaseFilter.setCutoff(sampleRate, Sample(1) / releaseSeconds);

    thresholdAmp = thresholdAmplitude;
    gateAmp = gateAmplitude;

    peakhold.setFrames(attackFrames + sustainFrames);
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

  Sample process(const Sample input, Sample inAbs)
  {
    auto &&peakAmp = peakhold.process(inAbs);
    auto &&candidate = applyCharacteristicCurve(peakAmp);
    auto &&released = processRelease(candidate);
    auto &&gainAmp = std::min(released, candidate);
    auto &&targetAmp = peakAmp < gateAmp ? 0 : gainAmp;
    auto &&smoothed = smoother.process(targetAmp);
    auto &&delayed = lookaheadDelay.process(input);
    return smoothed * delayed;
  }
};

} // namespace SomeDSP
