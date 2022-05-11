// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "matrixtype.hpp"

#include <array>
#include <numeric>
#include <random>

namespace SomeDSP {

template<typename Sample> class DoubleEMAFilter {
private:
  Sample v1 = 0;
  Sample v2 = 0;

public:
  void reset(Sample value = 0)
  {
    v1 = value;
    v2 = value;
  }

  Sample process(Sample input, Sample kp)
  {
    auto &&v0 = input;
    v1 += kp * (v0 - v1);
    v2 += kp * (v1 - v2);
    return v2;
  }
};

template<typename Sample> class EMAHighpass {
private:
  Sample v1 = 0;

public:
  void reset(Sample value = 0) { v1 = value; }

  Sample process(Sample input, Sample kp)
  {
    v1 += kp * (input - v1);
    return input - v1;
  }
};

template<typename Sample> class RateLimiter {
private:
  Sample target = 0;
  Sample value = 0;

public:
  static Sample rate;

  void reset(Sample value = 0)
  {
    this->value = value;
    this->target = value;
  }

  void push(Sample target) { this->target = target; }

  Sample process()
  {
    auto diff = target - value;
    if (diff > rate) {
      value += rate;
    } else if (diff < -rate) {
      value -= rate;
    } else {
      value = target;
    }
    return value;
  }
};

template<typename Sample> Sample RateLimiter<Sample>::rate = 1;

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
If `threshold <= 0`, gate will always open.
Note that this gate opens to 0, and close to 1. Specialized to modualte stereoCross.
*/
template<typename Sample> class EasyGate {
private:
  PeakHold<Sample> peakhold;
  DoubleEMAFilter<Sample> smoother;
  Sample threshold = 0;
  Sample openKp = Sample(1);  // Smoother coefficient for opening gate.
  Sample closeKp = Sample(1); // Smoother coefficient for closing gate.

public:
  void setup(Sample sampleRate, Sample holdSeconds)
  {
    auto holdSamples = size_t(sampleRate * holdSeconds);
    peakhold.resize(holdSamples);
    peakhold.setFrames(holdSamples);

    openKp = EMAFilter<double>::cutoffToP(sampleRate, Sample(1000));
    closeKp = EMAFilter<double>::cutoffToP(sampleRate, Sample(200));
  }

  void reset()
  {
    peakhold.reset();
    smoother.reset(Sample(0));
  }

  void prepare(Sample gateThresholdAmp)
  {
    threshold = gateThresholdAmp;
    if (threshold <= 0) smoother.reset(Sample(0));
  }

  Sample process(Sample inAbs)
  {
    if (threshold <= 0) return Sample(0);
    return threshold <= peakhold.process(inAbs) ? smoother.process(Sample(0), openKp)
                                                : smoother.process(Sample(1), closeKp);
  }
};

/**
If `length` is too long, compiler might silently fail to allocate stack.
*/
template<typename Sample, size_t length> class FeedbackDelayNetwork {
private:
  std::array<std::array<Sample, length>, length> matrix{};
  std::array<std::array<Sample, length>, 2> buf{};
  std::array<Delay<Sample>, length> delay;
  std::array<DoubleEMAFilter<Sample>, length> lowpass;
  std::array<EMAHighpass<Sample>, length> highpass;

  std::array<Sample, length> splitGain;
  size_t cycle = 100000;
  size_t counter = 0;
  size_t bufIndex = 0;

public:
  std::array<RateLimiter<Sample>, length> delayTimeSample;
  std::array<Sample, length> lowpassKp{};
  std::array<Sample, length> highpassKp{};

  /**
  Randomize `H` as orthogonal matrix. This algorithm is ported from
  `scipy.stats.ortho_group` in SciPy v1.8.0.
  */
  template<size_t dim>
  void randomOrthogonal(unsigned seed, std::array<std::array<Sample, dim>, dim> &H)
  {
    pcg64 rng{};
    rng.seed(seed);
    std::normal_distribution<Sample> dist{}; // mean 0, stddev 1.

    H.fill({});
    for (size_t i = 0; i < dim; ++i) H[i][i] = Sample(1);

    std::array<Sample, dim> x;
    for (size_t n = 0; n < dim; ++n) {
      auto xRange = dim - n;
      for (size_t i = 0; i < xRange; ++i) x[i] = dist(rng);

      Sample norm2 = 0;
      for (size_t i = 0; i < xRange; ++i) norm2 += x[i] * x[i];

      Sample x0 = x[0];

      Sample D = x0 >= 0 ? Sample(1) : Sample(-1);
      x[0] += D * std::sqrt(norm2);

      Sample denom = std::sqrt((norm2 - x0 * x0 + x[0] * x[0]) / Sample(2));
      for (size_t i = 0; i < xRange; ++i) x[i] /= denom;

      for (size_t row = 0; row < dim; ++row) {
        Sample dotH = 0;
        for (size_t col = 0; col < xRange; ++col) dotH += H[col][row] * x[col];
        for (size_t col = 0; col < xRange; ++col) {
          H[col][row] = D * (H[col][row] - dotH * x[col]);
        }
      }
    }
  }

  /**
  Randomize `H` as special orthogonal matrix. This algorithm is ported from
  `scipy.stats.special_ortho_group` in SciPy v1.8.0.
  */
  template<size_t dim>
  void randomSpecialOrthogonal(unsigned seed, std::array<std::array<Sample, dim>, dim> &H)
  {
    pcg64 rng{};
    rng.seed(seed);
    std::normal_distribution<Sample> dist{}; // mean 0, stddev 1.

    H.fill({});
    for (size_t i = 0; i < dim; ++i) H[i][i] = Sample(1);

    std::array<Sample, dim> x;
    std::array<Sample, dim> D;
    for (size_t n = 0; n < dim; ++n) {
      auto xRange = dim - n;
      for (size_t i = 0; i < xRange; ++i) x[i] = dist(rng);

      Sample norm2 = 0;
      for (size_t i = 0; i < xRange; ++i) norm2 += x[i] * x[i];

      Sample x0 = x[0];

      D[n] = x0 >= 0 ? Sample(1) : Sample(-1);
      x[0] += D[n] * std::sqrt(norm2);

      Sample denom = std::sqrt((norm2 - x0 * x0 + x[0] * x[0]) / Sample(2));
      for (size_t i = 0; i < xRange; ++i) x[i] /= denom;

      for (size_t row = 0; row < dim; ++row) {
        Sample dotH = 0;
        for (size_t col = 0; col < xRange; ++col) dotH += H[col][row] * x[col];
        for (size_t col = 0; col < xRange; ++col) H[col][row] -= dotH * x[col];
      }
    }

    size_t back = dim - 1;
    D[back] = (back & 0b1) == 0 ? Sample(1) : Sample(-1);
    for (size_t i = 0; i < back; ++i) D[back] *= D[i];

    for (size_t row = 0; row < dim; ++row) {
      for (size_t col = 0; col < dim; ++col) H[col][row] *= D[row];
    }
  }

  /**
  Construct following matrix:

  ```
  [[s * g1 - 1       , s * sqrt(g1 * g2), ... , s * sqrt(g1 * gN)],
   [s * sqrt(g2 * g1), s * g2 - 1       , ... , s * sqrt(g2 * gN)],
   [                 ,                  , ... ,                  ],
   [s * sqrt(gn * g1), s * sqrt(gN * g2), ... , s * gN - 1       ],
  ```

  where `s = 2 / (g1 + g2 + ... + gN)`.

  This is an implementation of eq. (24) and (25) in following paper.

  - Rocchesso, Davide, and Julius O. Smith. "Circulant and elliptic feedback delay
  networks for artificial reverberation." IEEE Transactions on Speech and Audio
  Processing 5.1 (1997): 51-63.
  */
  template<size_t dim>
  void randomCirculantOrthogonal(
    unsigned seed, size_t band, std::array<std::array<Sample, dim>, dim> &mat)
  {
    pcg64 rng{};
    rng.seed(seed);
    std::uniform_real_distribution<Sample> dist{Sample(0), Sample(1)};

    size_t left = 0;
    if (band >= length) {
      band = length;
    } else {
      left = 1;
    }

    std::array<Sample, length> source{};
    Sample sum = 0;
    do {
      sum = 0;
      for (size_t i = left; i < band; ++i) {
        source[i] = dist(rng);
        sum += source[i];
      }
    } while (sum == 0); // Avoid 0 division.

    Sample scale = Sample(2) / sum;

    std::array<Sample, length> squared;
    for (size_t i = 0; i < length; ++i) squared[i] = std::sqrt(source[i]);

    for (size_t row = 0; row < length; ++row) {
      for (size_t col = 0; col < length; ++col) {
        mat[row][col] = row == col ? scale * source[row] - Sample(1)
                                   : scale * squared[row] * squared[col];
      }
    }
  }

  /**
  Using similar normalization technique of `randomCirculantOrthogonal`.
  */
  template<size_t dim>
  void randomUpperTriangular(
    unsigned seed, Sample low, Sample high, std::array<std::array<Sample, dim>, dim> &mat)
  {
    pcg64 rng{};
    rng.seed(seed);
    if (low > high) std::swap(low, high);
    std::uniform_real_distribution<Sample> dist{low, high};

    mat.fill({});

    for (size_t row = 0; row < length; ++row) {
      for (size_t col = row; col < length; ++col) mat[row][col] = dist(rng);
    }
    for (size_t col = 0; col < length; ++col) {
      Sample sum = 0;
      for (size_t row = 0; row < col + 1; ++row) sum += mat[row][col];
      Sample scale = Sample(2) / sum;
      mat[col][col] = scale * mat[col][col] - Sample(1);
      for (size_t row = 0; row < col; ++row) mat[row][col] *= scale;
    }
  }

  /**
  Using similar normalization technique of `randomCirculantOrthogonal`.
  Transpose of `randomUpperTriangular`.
  */
  template<size_t dim>
  void randomLowerTriangular(
    unsigned seed, Sample low, Sample high, std::array<std::array<Sample, dim>, dim> &mat)
  {
    pcg64 rng{};
    rng.seed(seed);
    if (low > high) std::swap(low, high);
    std::uniform_real_distribution<Sample> dist{low, high};

    mat.fill({});

    for (size_t row = 0; row < length; ++row) {
      for (size_t col = 0; col < row + 1; ++col) mat[row][col] = dist(rng);
    }
    for (size_t col = 0; col < length; ++col) {
      Sample sum = 0;
      for (size_t row = col; row < length; ++row) sum += mat[row][col];
      Sample scale = Sample(2) / sum;
      mat[col][col] = scale * mat[col][col] - Sample(1);
      for (size_t row = col + 1; row < length; ++row) mat[row][col] *= scale;
    }
  }

  /**
  Construct following matrix:

  ```
  [[ g1,   0,   0,   0,       0,       0],
   [  0,  g2,   0,   0,       0,       0],
   [  0,   0,  g3,   0,       0,       0],
   [  0,   0,   0,  g4,       0,       0],
   [ s5,  s5,  s5,  s5, s5 * g5,       0],
   [-v6, -v6, -v6, -v6, s6 * G6, s6 * g6]]
  ```

  where:
  - s5 = 2 / (N - 2 + g5).
  - s6 = 2 / ((N - 2) * g5 + G6 + g6).
  - v6 = -g5 * s6.
  - G6 = 1 - g5 * g5.
  - N is the size of square matrix.

  This is an implenetation of section IV. A. in following paper, which is the same one
  used in `randomAbsorbent`. Added normalization to last and second last rows for
  stability. Naive implementation is unstable with stereo cross when delay time is small.

  - Schlecht, Sebastian J., and Emanuel AP Habets. "On lossless feedback delay networks."
  IEEE Transactions on Signal Processing 65.6 (2016): 1554-1564.
  */
  template<size_t dim>
  void randomSchroeder(
    unsigned seed, Sample low, Sample high, std::array<std::array<Sample, dim>, dim> &mat)
  {
    static_assert(
      length >= 2, "FeedbackDelayNetwork::randomSchroeder(): length must be >= 2.");

    pcg64 rng{};
    rng.seed(seed);
    if (low > high) std::swap(low, high);
    std::uniform_real_distribution<Sample> dist{low, high};

    mat.fill({});

    for (size_t idx = 0; idx < length; ++idx) mat[idx][idx] = dist(rng);

    auto &&paraGain = mat[length - 2][length - 2];
    auto &&lastGain = Sample(1) - paraGain * paraGain;
    auto scale2 = Sample(2) / (Sample(length - 2) + paraGain);
    auto scale1 = Sample(2)
      / (Sample(length - 2) * paraGain + lastGain + matrix[length - 1][length - 1]);
    for (size_t col = 0; col < length - 1; ++col) {
      mat[length - 2][col] = scale2;
      mat[length - 1][col] = -paraGain * scale1;
    }
    mat[length - 1][length - 2] = lastGain * scale1;
  }

  /**
  Construct following matrix:

  ```
  [[-A * G  , A ],
   [ I - G^2, G ]]
  ```

  - I is identity matrix.
  - G is diagonal matrix represents all-pass gain. diag(g1, g2, ...).
  - A is orthogonal matrix.

  This is an implenetation of section IV. B. in following paper, which is the same one
  used in `randomSchroeder`. Generated feedback matrix is equivalent to nested allpass.

  - Schlecht, Sebastian J., and Emanuel AP Habets. "On lossless feedback delay networks."
  IEEE Transactions on Signal Processing 65.6 (2016): 1554-1564.
  */
  template<size_t dim>
  void randomAbsorbent(
    unsigned seed, Sample low, Sample high, std::array<std::array<Sample, dim>, dim> &mat)
  {
    static_assert(
      length >= 2, "FeedbackDelayNetwork::randomAbsorbent(): length must be >= 2.");
    static_assert(
      length % 2 == 0, "FeedbackDelayNetwork::randomAbsorbent(): length must be even.");

    pcg64 rng{};
    rng.seed(seed);
    if (low > high) std::swap(low, high);
    std::uniform_real_distribution<Sample> dist{low, high};
    std::uniform_int_distribution<unsigned> seeder{
      0, std::numeric_limits<unsigned>::max()};

    constexpr size_t half = length / 2;

    mat.fill({});

    std::array<std::array<Sample, half>, half> A;
    randomOrthogonal(seeder(rng), A);

    for (size_t col = 0; col < half; ++col) {
      auto gain = dist(rng);
      mat[half + col][half + col] = gain;             // Fill lower right.
      mat[half + col][col] = Sample(1) - gain * gain; // Fill lower left.
      for (size_t row = 0; row < half; ++row) {
        mat[row][half + col] = A[row][col];  // Fill top right.
        mat[row][col] = -A[row][col] * gain; // Fill top left
      }
    }
  }

  void randomizeMatrix(unsigned matrixType, unsigned seed)
  {
    if (matrixType == FeedbackMatrixType::specialOrthogonal) {
      randomSpecialOrthogonal(seed, matrix);
    } else if (matrixType == FeedbackMatrixType::circulantOrthogonal) {
      randomCirculantOrthogonal(seed, length, matrix);
    } else if (matrixType == FeedbackMatrixType::circulant4) {
      randomCirculantOrthogonal(seed, 4, matrix);
    } else if (matrixType == FeedbackMatrixType::circulant8) {
      randomCirculantOrthogonal(seed, 8, matrix);
    } else if (matrixType == FeedbackMatrixType::circulant16) {
      randomCirculantOrthogonal(seed, 16, matrix);
    } else if (matrixType == FeedbackMatrixType::circulant32) {
      randomCirculantOrthogonal(seed, 32, matrix);
    } else if (matrixType == FeedbackMatrixType::upperTriangularPositive) {
      randomUpperTriangular(seed, 0, Sample(1), matrix);
    } else if (matrixType == FeedbackMatrixType::upperTriangularNegative) {
      randomUpperTriangular(seed, Sample(-1), 0, matrix);
    } else if (matrixType == FeedbackMatrixType::lowerTriangularPositive) {
      randomLowerTriangular(seed, 0, Sample(1), matrix);
    } else if (matrixType == FeedbackMatrixType::lowerTriangularNegative) {
      randomLowerTriangular(seed, Sample(-1), 0, matrix);
    } else if (matrixType == FeedbackMatrixType::schroederPositive) {
      randomSchroeder(seed, 0, Sample(1), matrix);
    } else if (matrixType == FeedbackMatrixType::schroederNegative) {
      randomSchroeder(seed, Sample(-1), 0, matrix);
    } else if (matrixType == FeedbackMatrixType::absorbentPositive) {
      randomAbsorbent(seed, 0, Sample(1), matrix);
    } else if (matrixType == FeedbackMatrixType::absorbentNegative) {
      randomAbsorbent(seed, Sample(-1), 0, matrix);
    } else { // matrixType == FeedbackMatrixType::orthogonal, or default.
      randomOrthogonal(seed, matrix);
    }
  }

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &dl : delay) dl.setup(sampleRate, maxTime);

    lowpassKp.fill(Sample(1));
    highpassKp.fill(Sample(0.0006542843087824565)); // 5Hz cutoff when fs=48000Hz.

    reset();
  }

  void prepare(Sample sampleRate, Sample splitRotationHz)
  {
    auto &&inv = Sample(1) / splitRotationHz;
    cycle
      = inv >= Sample(std::numeric_limits<size_t>::max()) ? 1 : size_t(sampleRate * inv);
  }

  void reset()
  {
    buf.fill({});
    for (auto &dl : delay) dl.reset();
    for (auto &lp : lowpass) lp.reset();
    for (auto &hp : highpass) hp.reset();
  }

  /**
  `offset` is normalized phase in [0, 1].
  `skew` >= 0.
  */
  void fillSplitGain(Sample offset, Sample skew)
  {
    for (size_t idx = 0; idx < splitGain.size(); ++idx) {
      auto &&phase = offset + Sample(idx) / Sample(splitGain.size());
      splitGain[idx] = std::exp(skew * std::sin(Sample(twopi) * phase));
    }
    auto sum = std::accumulate(splitGain.begin(), splitGain.end(), Sample(0));
    for (auto &value : splitGain) value /= sum;
  }

  Sample preProcess(Sample splitPhaseOffset, Sample splitSkew)
  {
    if (++counter >= cycle) counter = 0;

    fillSplitGain(splitPhaseOffset + Sample(counter) / Sample(cycle), splitSkew);

    bufIndex ^= 1;
    auto &front = buf[bufIndex];
    auto &back = buf[bufIndex ^ 1];
    front.fill(0);
    for (size_t i = 0; i < length; ++i) {
      for (size_t j = 0; j < length; ++j) front[i] += matrix[i][j] * back[j];
    }
    return std::accumulate(front.begin(), front.end(), Sample(0));
  }

  Sample process(Sample input, Sample crossIn, Sample stereoCross, Sample feedback)
  {
    auto &front = buf[bufIndex];

    crossIn /= -Sample(length);
    for (size_t idx = 0; idx < length; ++idx) {
      auto &&crossed = front[idx] + stereoCross * (crossIn - front[idx]);
      auto &&sig = splitGain[idx] * input + feedback * crossed;
      auto &&delayed = delay[idx].process(sig, delayTimeSample[idx].process());
      auto &&lowpassed = lowpass[idx].process(delayed, lowpassKp[idx]);
      front[idx] = highpass[idx].process(lowpassed, highpassKp[idx]);
    }

    return std::accumulate(front.begin(), front.end(), Sample(0));
  }
};

} // namespace SomeDSP
