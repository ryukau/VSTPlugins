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

/**
HighEliminationFir removes high frequency components near nyquist frequency to prevent
clipping by true peak.

```python
import scipy.signal as signal
fir = signal.remez(63, [0, 18000, 23000, 24000], [1, 0], [100, 1], fs=48000, maxiter=256)
```
*/
template<typename Sample> class HighEliminationFir {
private:
  constexpr static std::array<Sample, 64> fir{
    Sample(4.9957589007682075e-05), Sample(-0.00021327109340196594),
    Sample(0.0004739960362686736),  Sample(-0.0008611690024367459),
    Sample(0.001260266054786863),   Sample(-0.0015161798249005863),
    Sample(0.0014026814978336472),  Sample(-0.0007337083968697367),
    Sample(-0.0005479898948901594), Sample(0.002261800136016642),
    Sample(-0.00395430054327271),   Sample(0.004960316462021296),
    Sample(-0.004588665194329012),  Sample(0.002394558631270695),
    Sample(0.0015393306210772118),  Sample(-0.006428637618655796),
    Sample(0.010827276500916071),   Sample(-0.012938044517719046),
    Sample(0.011161288569599697),   Sample(-0.004748860750888805),
    Sample(-0.005643812356704201),  Sample(0.01772818365710279),
    Sample(-0.027832816368675142),  Sample(0.03165730883844591),
    Sample(-0.02538813018473096),   Sample(0.0068901595228859965),
    Sample(0.023381358252292393),   Sample(-0.06206344987682824),
    Sample(0.10333010971288058),    Sample(-0.14004974307017476),
    Sample(0.16538655646332506),    Sample(0.8255726297215179),
    Sample(0.16538655646332506),    Sample(-0.14004974307017476),
    Sample(0.10333010971288058),    Sample(-0.06206344987682824),
    Sample(0.023381358252292393),   Sample(0.0068901595228859965),
    Sample(-0.02538813018473096),   Sample(0.03165730883844591),
    Sample(-0.027832816368675142),  Sample(0.01772818365710279),
    Sample(-0.005643812356704201),  Sample(-0.004748860750888805),
    Sample(0.011161288569599697),   Sample(-0.012938044517719046),
    Sample(0.010827276500916071),   Sample(-0.006428637618655796),
    Sample(0.0015393306210772118),  Sample(0.002394558631270695),
    Sample(-0.004588665194329012),  Sample(0.004960316462021296),
    Sample(-0.00395430054327271),   Sample(0.002261800136016642),
    Sample(-0.0005479898948901594), Sample(-0.0007337083968697367),
    Sample(0.0014026814978336472),  Sample(-0.0015161798249005863),
    Sample(0.001260266054786863),   Sample(-0.0008611690024367459),
    Sample(0.0004739960362686736),  Sample(-0.00021327109340196594),
    Sample(4.9957589007682075e-05), Sample(0.0),
  };

  std::array<Sample, 64> buf{};

public:
  static constexpr size_t delay = 31;

  void reset() { buf.fill(Sample(0)); }

  Sample process(Sample input)
  {
    std::rotate(buf.rbegin(), buf.rbegin() + 1, buf.rend());
    buf[0] = input;

    Sample output = 0;
    for (size_t n = 0; n < fir.size(); ++n) output += buf[n] * fir[n];
    return output;
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
    sum1 = std::fmax(Sample(0), sum1 - d1);

    sum2 = add(sum2, sum1);
    Sample d2 = delay2.process(sum1);
    sum2 = std::fmax(Sample(0), sum2 - d2);

    auto output = buf;
    buf = sum2;
    return output;
  }
};

template<typename Sample> class Limiter {
private:
  static constexpr Sample releaseConstant = Sample(1e-5);

  Sample thresholdAmp = Sample(1); // thresholdAmp > 0.
  Sample gateAmp = 0;              // gateAmp >= 0.
  Sample gainAmp = Sample(1);      // Internal.
  Sample release = 0;              // Exponential curve coefficient.
  size_t attackFrames = 0;

  PeakHold<Sample> peakhold;
  DoubleAverageFilter<double> smoother;
  IntDelay<Sample> lookaheadDelay;

public:
  size_t latency(size_t upfold) { return attackFrames / upfold; }

  void resize(size_t size)
  {
    size += size % 2;
    peakhold.resize(size);
    smoother.resize(size);
    lookaheadDelay.resize(size);
  }

  void reset()
  {
    gainAmp = Sample(1);
    peakhold.reset();
    smoother.reset();
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
    if (prevAttack != attackFrames) reset();

    release
      = std::pow(Sample(1) / releaseConstant, Sample(1) / (releaseSeconds * sampleRate));

    thresholdAmp = thresholdAmplitude;
    gateAmp = gateAmplitude;

    peakhold.setFrames(attackFrames + size_t(sampleRate * sustainSeconds));
    smoother.setFrames(attackFrames);
    lookaheadDelay.setFrames(attackFrames);
  }

  inline Sample applyCharacteristicCurve(Sample peakAmp)
  {
    return peakAmp > thresholdAmp ? thresholdAmp / peakAmp : Sample(1);
  }

  Sample process(const Sample input, Sample inAbs)
  {
    auto &&peakAmp = peakhold.process(inAbs);
    auto &&candidate = applyCharacteristicCurve(peakAmp);
    gainAmp = std::min(gainAmp * release, candidate);
    auto &&targetAmp = peakAmp <= gateAmp ? 0 : gainAmp;
    auto &&smoothed = smoother.process(targetAmp);
    auto &&delayed = lookaheadDelay.process(input);
    return smoothed * delayed;
  }
};

} // namespace SomeDSP
