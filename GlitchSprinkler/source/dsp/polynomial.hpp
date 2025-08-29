// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <numeric>

namespace SomeDSP {

/**
Solve `A x = b` for `x`.

`lu` and `y` are used for meaningless intermediate values.These are required to avoid
memory allocation for each call.
*/
template<typename T, size_t size> class Solver {
private:
  static constexpr T epsilon = std::numeric_limits<T>::epsilon();

public:
  std::array<std::array<T, size>, size> lu{};
  std::array<T, size> y{};

  void solve(
    std::array<std::array<T, size>, size> &A,
    std::array<T, size> &b,
    std::array<T, size> &x)
  {
    lu = A;
    for (int i = 0; i < int(size); ++i) {
      if (std::abs(A[i][i]) <= epsilon) { // Pivoting.
        int j = i + 1;
        for (; j < int(size); ++j) {
          if (std::abs(A[j][i]) <= epsilon) continue;
          std::swap(A[i], A[j]);
          std::swap(b[i], b[j]);
          break;
        }
        if (j >= int(size)) {
          // If something goes wrong, check this branch.
          x.fill(0);
          return;
        }
      }

      for (int j = i; j < int(size); ++j) {
        T sum = T(0);
        for (int k = 0; k < i; ++k) sum += lu[i][k] * lu[k][j];
        lu[i][j] = A[i][j] - sum;
      }

      // Avoid division by 0. This is only suitable for this application.
      if (std::abs(lu[i][i]) < epsilon) lu[i][i] = std::copysign(epsilon, lu[i][i]);

      for (int j = i + 1; j < int(size); ++j) {
        T sum = T(0);
        for (int k = 0; k < i; ++k) sum += lu[j][k] * lu[k][i];
        lu[j][i] = (A[j][i] - sum) / lu[i][i];
      }
    }

    for (int i = 0; i < int(size); ++i) {
      T sum = T(0);
      for (int j = 0; j < i; ++j) sum += lu[i][j] * y[j];
      y[i] = b[i] - sum;
    }

    for (int i = int(size) - 1; i >= 0; --i) {
      T sum = T(0);
      for (int j = i + 1; j < int(size); ++j) sum += lu[i][j] * x[j];
      x[i] = (y[i] - sum) / lu[i][i];
    }
  }
};

// `a` is an array of polynomial coefficients.
template<typename T, int size> inline T computePolynomial(T x, std::array<T, size> &a)
{
  static_assert(size >= 0);
  if constexpr (size == 0) return T(0);
  if constexpr (size == 1) return a[0];

  T v = a.back();
  for (int i = size - 2; i >= 0; --i) v = v * x + a[i];
  return v;
}

template<typename Sample, size_t nControlPoint> class PolynomialCoefficientSolver {
public:
  static constexpr size_t nPolynomialPoint = nControlPoint + 2;

private:
  Solver<Sample, nPolynomialPoint> solver;
  std::array<std::array<Sample, nPolynomialPoint>, nPolynomialPoint> A{};

  static constexpr size_t nFirstDerivative = nPolynomialPoint - 1;
  std::array<Sample, nFirstDerivative> d1;
  std::array<std::array<Sample, 2>, nFirstDerivative> peaks;

public:
  std::array<Sample, nPolynomialPoint> polyX{};
  std::array<Sample, nPolynomialPoint> polyY{};

  std::array<Sample, nPolynomialPoint> coefficients{};
  Sample normalizeGain = Sample(1);

  Sample evaluate(Sample x)
  {
    return computePolynomial<Sample, nPolynomialPoint>(x, coefficients);
  }

  void updateCoefficients(bool normalize = false)
  {
    // Selection sort.
    for (size_t i = 1; i < nPolynomialPoint - 1; ++i) {
      size_t minIdx = i;
      for (size_t j = i; j < nPolynomialPoint - 1; ++j) {
        if (polyX[minIdx] > polyX[j]) minIdx = j;
      }
      if (minIdx != i) {
        std::swap(polyX[minIdx], polyX[i]);
        std::swap(polyY[minIdx], polyY[i]);
      }
    }

    // A[n] = [x[n]^0, x[n]^1, x[n]^2, x[n]^3, ...].
    A[0].fill(0);
    A[0][0] = Sample(1);
    A.back().fill(1);
    for (size_t i = 1; i < nPolynomialPoint - 1; ++i) {
      if constexpr (nPolynomialPoint >= 1) A[i][0] = Sample(1);
      if constexpr (nPolynomialPoint >= 2) A[i][1] = polyX[i];
      auto intPower = polyX[i];
      for (size_t j = 2; j < nPolynomialPoint; ++j) {
        intPower *= polyX[i];
        A[i][j] = intPower;
      }
    }

    solver.solve(A, polyY, coefficients);

    if (!normalize) return;

    // From here, it starts finding normalization gain.
    // `d1` is 1st order derivative of target polynomial.
    for (size_t i = 0; i < d1.size(); ++i) d1[i] = Sample(i + 1) * coefficients[i + 1];

    auto getPeakPoint = [&](Sample x) {
      return std::array<Sample, 2>{
        x, std::abs(computePolynomial<Sample, nPolynomialPoint>(x, coefficients))};
    };
    auto sgn = [](Sample x) { return x > 0 ? Sample(1) : x < 0 ? Sample(-1) : x; };
    constexpr size_t maxIteration = 53; // Number of significand bits.
    for (size_t i = 0; i < peaks.size(); ++i) {
      // Binary search. L: left, M: mid, R: right.
      Sample xL = polyX[i];
      Sample xR = polyX[i + 1];
      Sample xM;

      size_t iter = 0;
      do {
        Sample yL = computePolynomial<Sample, nFirstDerivative>(xL, d1);
        Sample yR = computePolynomial<Sample, nFirstDerivative>(xR, d1);

        Sample signL = sgn(yL);
        Sample signR = sgn(yR);
        if (signL == signR) {
          const auto pkL = getPeakPoint(xL);
          const auto pkR = getPeakPoint(xR);
          peaks[i] = pkL[1] >= pkR[1] ? pkL : pkR;
          break;
        }

        xM = 0.5 * (xR + xL);
        Sample yM = computePolynomial<Sample, nFirstDerivative>(xM, d1);

        Sample signM = sgn(yM);
        if (signM == 0) {
          peaks[i] = getPeakPoint(xM);
          break;
        } else if (signL == signM) {
          xL = xM;
        } else if (signR == signM) {
          xR = xM;
        }
      } while (++iter < maxIteration);

      if (iter >= maxIteration) peaks[i] = getPeakPoint(xM);
    }

    // Find max peak.
    Sample maxPeak = peaks[0][1];
    for (size_t i = 1; i < peaks.size(); ++i) {
      if (maxPeak < peaks[i][1]) maxPeak = peaks[i][1];
    }
    constexpr auto minNormalized = std::numeric_limits<Sample>::min();
    normalizeGain = maxPeak > minNormalized ? Sample(0.5) / maxPeak : Sample(1);

    for (auto &v : coefficients) v *= normalizeGain;
  }
};

template<typename Sample> class ResonantEmaLowpass1A1 {
private:
  static constexpr Sample nyquist = Sample(0.4999);

  Sample cutValue = 0;
  Sample apsValue = 0;
  Sample resValue = 0;

  Sample v1 = 0;
  Sample u1 = 0;
  Sample u2 = 0;

public:
  void reset()
  {
    cutValue = 0;
    apsValue = 0;
    resValue = 0;
    v1 = 0;
    u1 = 0;
    u2 = 0;
  }

  Sample process(
    Sample input,
    Sample interpRate,
    Sample cutoffNormalized,
    Sample resonance,
    Sample apScale)
  {
    cutValue += interpRate * (cutoffNormalized - cutValue);

    constexpr Sample pi = std::numbers::pi_v<Sample>;
    const auto freq = pi * std::clamp(cutValue, Sample(0), nyquist);

    const auto s = Sample(1) - std::cos(Sample(2) * freq);
    const auto c1 = std::sqrt(s * s + Sample(2) * s) - s;

    apsValue += interpRate * (apScale - apsValue);
    const auto t = std::tan(std::clamp(apsValue * freq, Sample(0), nyquist));
    const auto c2 = (t - 1) / (t + 1);

    resValue += interpRate * (resonance - resValue);
    const auto q = resValue * (c2 - c1 * c2 + Sample(1));

    // Bilinear allpass, order 1.
    v1 = c2 * (u1 - v1) + u2;
    u2 = u1;

    // Exponential moving average (EMA) lowpass.
    return u1 += c1 * (input - u1) - q * v1;
  }
};

// Bilinear transformed 1-pole highpass.
template<typename Sample> class SafetyFilter {
private:
  Sample b = Sample(1);
  Sample a1 = Sample(1);
  Sample x1 = Sample(0);
  Sample y1 = Sample(0);

public:
  void reset()
  {
    x1 = 0;
    y1 = 0;
  }

  void setup(Sample sampleRate)
  {
    constexpr Sample pi = std::numbers::pi_v<Sample>;
    constexpr Sample minCutoff = Sample(0.00001);
    constexpr Sample nyquist = Sample(0.49998);
    constexpr Sample highpassCutoffHz = Sample(16);

    const auto k = Sample(1)
      / std::tan(pi * std::clamp(highpassCutoffHz / sampleRate, minCutoff, nyquist));
    const auto a0 = Sample(1) + k;
    b = k / a0;
    a1 = (Sample(1) - k) / a0;
  }

  Sample process(Sample input)
  {
    y1 = b * (input - x1) - a1 * y1;
    x1 = input;
    return y1;
  }
};

} // namespace SomeDSP
