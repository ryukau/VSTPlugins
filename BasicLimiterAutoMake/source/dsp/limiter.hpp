// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
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
    buf.resize(size + 1);
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

template<typename Sample> class PeakHold {
  static constexpr Sample lowest = std::numeric_limits<Sample>::lowest();

  std::unique_ptr<Sample[]> buffer;
  size_t bufferSize = 0;
  size_t bufferMask = 0;
  size_t head = 0;
  size_t tail = 0;
  size_t splitIndex = 0;
  Sample frontMax = lowest;

  // Force inline may trigger auto-vectorization to `process`.
#if defined(_MSC_VER)
  #define FORCE_INLINE [[msvc::forceinline]]
#elif defined(__GNUC__) || defined(__clang__)
  #define FORCE_INLINE [[gnu::always_inline]] inline
#else
  #define FORCE_INLINE inline
#endif
  FORCE_INLINE void refillBackStack()
  {
    Sample *const buf = buffer.get();
    Sample currentMax = lowest;

    const size_t h = head & bufferMask;
    const size_t t = tail & bufferMask;

    if (h > t) {
      for (size_t i = h; i > t; --i) {
        Sample &val = buf[i - 1];
        if (val > currentMax) currentMax = val;
        val = currentMax;
      }
    } else {
      for (size_t i = h; i > 0; --i) {
        Sample &val = buf[i - 1];
        if (val > currentMax) currentMax = val;
        val = currentMax;
      }
      for (size_t i = bufferSize; i > t; --i) {
        Sample &val = buf[i - 1];
        if (val > currentMax) currentMax = val;
        val = currentMax;
      }
    }
    splitIndex = head;
    frontMax = lowest;
  }
#undef FORCE_INLINE

public:
  PeakHold(size_t maxLength = 65536) { resize(maxLength); }

  size_t size() const { return head - tail; }

  void resize(size_t maxLength)
  {
    size_t newSize = 1;
    while (newSize <= maxLength) newSize *= 2;

    buffer = std::make_unique_for_overwrite<Sample[]>(newSize);
    bufferSize = newSize;
    bufferMask = newSize - 1;
    reset();
  }

  void reset(Sample fill = lowest)
  {
    std::fill(buffer.get(), buffer.get() + bufferSize, fill);
    head = 0;
    tail = 0;
    splitIndex = 0;
    frontMax = lowest;
  }

  void setFrames(size_t newSize, bool preserveCurrentPeak = false)
  {
    while (size() < newSize) {
      Sample backPrev = buffer[tail & bufferMask];
      tail--;
      Sample &back = buffer[tail & bufferMask];
      back = preserveCurrentPeak ? backPrev : std::max(back, backPrev);
    }
    while (size() > newSize) pop();
  }

  inline void push(Sample v)
  {
    buffer[head & bufferMask] = v;
    head++;
    if (v > frontMax) frontMax = v;
  }

  void pop()
  {
    if (tail == splitIndex) {
      if (head == tail) {
        frontMax = lowest;
        splitIndex = head;
        return;
      }
      refillBackStack();
    }
    tail++;
  }

  inline Sample read() const
  {
    Sample backMax = buffer[tail & bufferMask];
    return (backMax > frontMax) ? backMax : frontMax;
  }

  Sample process(Sample v)
  {
    push(v);

    if (tail == splitIndex) refillBackStack();
    tail++;

    return read();
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

  // Refer to `DoubleAverageFilter` in BasicLimiter for the details of this method.
  inline Sample add(Sample lhs, Sample rhs)
  {
    using Integer = std::conditional_t<sizeof(Sample) == 4, int32_t, int64_t>;
    constexpr int mantissaBits = std::numeric_limits<Sample>::digits - 1;
    constexpr int exponentBias = (1 << (sizeof(Sample) * 8 - mantissaBits - 2)) - 1;

    if (std::abs(lhs) < std::abs(rhs)) std::swap(lhs, rhs);
    if (lhs == 0) return rhs;

    constexpr Integer maxInt = std::numeric_limits<Integer>::max();
    auto rawExp = (std::bit_cast<Integer>(lhs) & maxInt) >> mantissaBits;
    auto scaleExp = (2 * exponentBias + mantissaBits) - rawExp;
    if (scaleExp <= 0 || scaleExp >= 2 * exponentBias) return lhs + rhs;

    auto scale = std::bit_cast<Sample>(Integer(scaleExp) << mantissaBits);
    return lhs + std::trunc(rhs * scale) / scale;
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
  Sample gateAmp = 0; // gateAmp >= 0.

  PeakHold<Sample> peakhold;
  DoubleAverageFilter<double> smoother;
  DoubleEMAFilter<Sample> releaseFilter;
  IntDelay<Sample> lookaheadDelay;

public:
  inline size_t getAttackFrames() { return attackFrames; }
  inline size_t latency(size_t upfold) { return attackFrames / upfold; }

  void resize(size_t size)
  {
    size += size % 2;

    // Assuming `maxAttackTime = maxSustainTime`. Otherwise peakhold requires the size
    // of `maxAttackTime + maxSustainTime`.
    peakhold.resize(2 * size);

    smoother.resize(size);
    lookaheadDelay.resize(size);
  }

  void reset(Sample thresholdAmplitude)
  {
    peakhold.reset();
    smoother.reset();
    releaseFilter.reset(thresholdAmplitude);
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

    if (prevAttack != attackFrames || prevSustain != sustainFrames)
      reset(thresholdAmplitude);

    releaseFilter.setCutoff(sampleRate, Sample(1) / releaseSeconds);

    gateAmp = gateAmplitude;

    peakhold.setFrames(attackFrames + sustainFrames);
    smoother.setFrames(attackFrames);
    lookaheadDelay.setFrames(attackFrames);
  }

  inline Sample applyCharacteristicCurve(Sample thresholdAmp, Sample peakAmp)
  {
    return peakAmp > thresholdAmp ? thresholdAmp / peakAmp : Sample(1);
  }

  inline Sample processRelease(Sample gain)
  {
    releaseFilter.setMin(gain);
    return releaseFilter.process(gain);
  }

  Sample process(const Sample input, Sample inAbs, Sample thresholdAmplitude)
  {
    auto peakAmp = peakhold.process(inAbs);
    auto candidate = applyCharacteristicCurve(thresholdAmplitude, peakAmp);
    auto released = processRelease(candidate);
    auto gainAmp = std::min(released, candidate);
    auto targetAmp = peakAmp < gateAmp ? 0 : gainAmp;
    auto smoothed = smoother.process(targetAmp);
    auto delayed = lookaheadDelay.process(input);
    return smoothed * delayed;
  }
};

template<typename Sample> class AutoMakeUp {
public:
  size_t attackFrames = 0;
  Sample makeUpTarget = Sample(1);
  DoubleAverageFilter<double> makeUpSmoother;

  void resize(size_t size) { makeUpSmoother.resize(size); }

  void reset(bool isEnabled, Sample thresholdAmplitude, Sample targetAmplitude)
  {
    makeUpTarget = isEnabled ? targetAmplitude / thresholdAmplitude : Sample(1);
    makeUpSmoother.reset();
  }

  void prepare(
    bool isEnabled,
    Sample thresholdAmplitude,
    Sample targetAmplitude,
    size_t limiterAttackFrames)
  {
    if (attackFrames != limiterAttackFrames) makeUpSmoother.reset();
    attackFrames = limiterAttackFrames;

    makeUpTarget = isEnabled ? targetAmplitude / thresholdAmplitude : Sample(1);
    makeUpSmoother.setFrames(attackFrames);
  }

  Sample process(bool isEnabled, Sample thresholdAmplitude, Sample targetAmplitude)
  {
    return makeUpSmoother.process(
      isEnabled ? targetAmplitude / thresholdAmplitude : Sample(1));
  }

  Sample processNaive() { return makeUpSmoother.process(makeUpTarget); }
};

} // namespace SomeDSP
