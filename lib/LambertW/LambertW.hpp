/*
  Implementation of the Lambert W function

  Copyright (C) 2015 Darko Veberic, darko.veberic@ijs.si

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef _utl_LambertW_h_
  #define _utl_LambertW_h_

  #include <cmath>
  #include <iostream>
  #include <limits>
  #include <numbers>

  #define Y2(d1, c12, d2) ((c12) ? (d1) : (d2))
  #define Y3(d1, c12, d2, c23, d3) Y2(Y2(d1, c12, d2), c23, d3)
  #define Y4(d1, c12, d2, c23, d3, c34, d4) Y3(d1, c12, d2, c23, Y2(d3, c34, d4))
  #define Y5(d1, c12, d2, c23, d3, c34, d4, c45, d5)                                     \
    Y4(Y2(d1, c12, d2), c23, d3, c34, d4, c45, d5)
  #define Y6(d1, c12, d2, c23, d3, c34, d4, c45, d5, c56, d6)                            \
    Y5(d1, c12, d2, c23, d3, c34, d4, c45, Y2(d5, c56, d6))
  #define Y7(d1, c12, d2, c23, d3, c34, d4, c45, d5, c56, d6, c67, d7)                   \
    Y6(d1, c12, d2, c23, Y2(d3, c34, d4), c45, d5, c56, d6, c67, d7)

namespace Fukushima {

inline double LambertW0(const double x);
inline double LambertWm1(const double x);

inline double LambertW(const int branch, const double x)
{
  switch (branch) {
    case -1:
      return LambertWm1(x);
    case 0:
      return LambertW0(x);
    default:
      return std::numeric_limits<double>::quiet_NaN();
  }
}

inline double LambertWSeries(const double p)
{
  static const double q[] = {
    -1,
    +1,
    -0.333333333333333333,
    +0.152777777777777778,
    -0.0796296296296296296,
    +0.0445023148148148148,
    -0.0259847148736037625,
    +0.0156356325323339212,
    -0.00961689202429943171,
    +0.00601454325295611786,
    -0.00381129803489199923,
    +0.00244087799114398267,
    -0.00157693034468678425,
    +0.00102626332050760715,
    -0.000672061631156136204,
    +0.000442473061814620910,
    -0.000292677224729627445,
    +0.000194387276054539318,
    -0.000129574266852748819,
    +0.0000866503580520812717,
    -0.0000581136075044138168};
  const double ap = std::abs(p);
  // clang-format off
    if (ap < 0.01159)
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*q[6]
        )))));
    else if (ap < 0.0766)
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*(q[6] +
        p*(q[7] +
        p*(q[8] +
        p*(q[9] +
        p*q[10]
        )))))))));
    else
      return
        -1 +
        p*(1 +
        p*(q[2] +
        p*(q[3] +
        p*(q[4] +
        p*(q[5] +
        p*(q[6] +
        p*(q[7] +
        p*(q[8] +
        p*(q[9] +
        p*(q[10] +
        p*(q[11] +
        p*(q[12] +
        p*(q[13] +
        p*(q[14] +
        p*(q[15] +
        p*(q[16] +
        p*(q[17] +
        p*(q[18] +
        p*(q[19] +
        p*q[20]
        )))))))))))))))))));
  // clang-format on
}

inline double LambertW0ZeroSeries(const double z)
{
  // clang-format off
    return
      z*(1 -
      z*(1 -
      z*(1.5 -
      z*(2.6666666666666666667 -
      z*(5.2083333333333333333 -
      z*(10.8 -
      z*(23.343055555555555556 -
      z*(52.012698412698412698 -
      z*(118.62522321428571429 -
      z*(275.57319223985890653 -
      z*(649.78717234347442681 -
      z*(1551.1605194805194805 -
      z*(3741.4497029592385495 -
      z*(9104.5002411580189358 -
      z*(22324.308512706601434 -
      z*(55103.621972903835338 -
      z*136808.86090394293563
      ))))))))))))))));
  // clang-format on
}

inline double FinalResult(const double w, const double y)
{
  const double f0 = w - y;
  const double f1 = 1 + y;
  const double f00 = f0 * f0;
  const double f11 = f1 * f1;
  const double f0y = f0 * y;
  return w
    - 4 * f0 * (6 * f1 * (f11 + f0y) + f00 * y)
    / (f11 * (24 * f11 + 36 * f0y) + f00 * (6 * y * y + 8 * f1 * y + f0y));
}

inline double LambertW0(const double z)
{
  static double e[66];
  static double g[65];
  static double a[12];
  static double b[12];

  if (!e[0]) {
    const double e1 = 1 / std::numbers::e_v<double>;
    double ej = 1;
    e[0] = std::numbers::e_v<double>;
    e[1] = 1;
    g[0] = 0;
    for (int j = 1, jj = 2; jj < 66; ++jj) {
      ej *= std::numbers::e_v<double>;
      e[jj] = e[j] * e1;
      g[j] = j * ej;
      j = jj;
    }
    a[0] = sqrt(e1);
    b[0] = 0.5;
    for (int j = 0, jj = 1; jj < 12; ++jj) {
      a[jj] = sqrt(a[j]);
      b[jj] = b[j] * 0.5;
      j = jj;
    }
  }
  if (std::abs(z) < 0.05) return LambertW0ZeroSeries(z);
  if (z < -0.35) {
    const double p2 = 2 * (std::numbers::e_v<double> * z + 1);
    if (p2 > 0) return LambertWSeries(sqrt(p2));
    if (p2 == 0) return -1;
    std::cerr << "(lambertw0) Argument out of range. z=" << z << std::endl;
    return std::numeric_limits<double>::quiet_NaN();
  }
  int n;
  for (n = 0; n <= 2; ++n)
    if (g[n] > z) goto line1;
  n = 2;
  for (int j = 1; j <= 5; ++j) {
    n *= 2;
    if (g[n] > z) goto line2;
  }
  std::cerr << "(lambertw0) Argument too large. z=" << z << std::endl;
  return std::numeric_limits<double>::quiet_NaN();
line2: {
  int nh = n / 2;
  for (int j = 1; j <= 5; ++j) {
    nh /= 2;
    if (nh <= 0) break;
    if (g[n - nh] > z) n -= nh;
  }
}
line1:
  --n;
  int jmax = 8;
  if (z <= -0.36)
    jmax = 12;
  else if (z <= -0.3)
    jmax = 11;
  else if (n <= 0)
    jmax = 10;
  else if (n <= 1)
    jmax = 9;
  double y = z * e[n + 1];
  double w = n;
  for (int j = 0; j < jmax; ++j) {
    const double wj = w + b[j];
    const double yj = y * a[j];
    if (wj < yj) {
      w = wj;
      y = yj;
    }
  }
  return FinalResult(w, y);
}

inline double LambertWm1(const double z)
{
  static double e[64];
  static double g[64];
  static double a[12];
  static double b[12];

  if (!e[0]) {
    const double e1 = 1 / std::numbers::e_v<double>;
    double ej = e1;
    e[0] = std::numbers::e_v<double>;
    g[0] = -e1;
    for (int j = 0, jj = 1; jj < 64; ++jj) {
      ej *= e1;
      e[jj] = e[j] * std::numbers::e_v<double>;
      g[jj] = -(jj + 1) * ej;
      j = jj;
    }
    a[0] = sqrt(std::numbers::e_v<double>);
    b[0] = 0.5;
    for (int j = 0, jj = 1; jj < 12; ++jj) {
      a[jj] = sqrt(a[j]);
      b[jj] = b[j] * 0.5;
      j = jj;
    }
  }
  if (z >= 0) {
    std::cerr << "(lambertwm1) Argument out of range. z=" << z << std::endl;
    return std::numeric_limits<double>::quiet_NaN();
  }
  if (z < -0.35) {
    const double p2 = 2 * (std::numbers::e_v<double> * z + 1);
    if (p2 > 0) return LambertWSeries(-sqrt(p2));
    if (p2 == 0) return -1;
    std::cerr << "(lambertwm1) Argument out of range. z=" << z << std::endl;
    return std::numeric_limits<double>::quiet_NaN();
  }
  int n = 2;
  if (g[n - 1] > z) goto line1;
  for (int j = 1; j <= 5; ++j) {
    n *= 2;
    if (g[n - 1] > z) goto line2;
  }
  std::cerr << "(lambertwm1) Argument too small. z=" << z << std::endl;
  return std::numeric_limits<double>::quiet_NaN();
line2: {
  int nh = n / 2;
  for (int j = 1; j <= 5; ++j) {
    nh /= 2;
    if (nh <= 0) break;
    if (g[n - nh - 1] > z) n -= nh;
  }
}
line1:
  --n;
  int jmax = 11;
  if (n >= 8)
    jmax = 8;
  else if (n >= 3)
    jmax = 9;
  else if (n >= 2)
    jmax = 10;
  double w = -n;
  double y = z * e[n - 1];
  for (int j = 0; j < jmax; ++j) {
    const double wj = w - b[j];
    const double yj = y * a[j];
    if (wj < yj) {
      w = wj;
      y = yj;
    }
  }
  return FinalResult(w, y);
}

} // namespace Fukushima

namespace utl {

template<typename Float, class Tag, unsigned int order> struct Polynomial {
  static inline Float Coeff();
  static inline Float Coeff(const Float x);
};

template<typename Float, class Tag, unsigned int order> struct Horner {
  static Float Recurse(const Float term, const Float x)
  {
    return Horner<Float, Tag, order - 1>::Recurse(
      term * x + Polynomial<Float, Tag, order>::Coeff(), x);
  }
  static Float RecurseAlt(const Float term, const Float x)
  {
    return Horner<Float, Tag, order - 1>::RecurseAlt(
      Polynomial<Float, Tag, order>::Coeff() + x * term, x);
  }
  static Float Eval(const Float x)
  {
    return Horner<Float, Tag, order - 1>::Recurse(
      Polynomial<Float, Tag, order>::Coeff(), x);
  }
  static Float EvalAlt(const Float x)
  {
    return Horner<Float, Tag, order - 1>::RecurseAlt(
      Polynomial<Float, Tag, order>::Coeff(), x);
  }
  //
  static Float Recurse(const Float term, const Float x, const Float y)
  {
    return Horner<Float, Tag, order - 1>::Recurse(
      term * x + Polynomial<Float, Tag, order>::Coeff(y), x, y);
  }
  static Float Eval(const Float x, const Float y)
  {
    return Horner<Float, Tag, order - 1>::Recurse(
      Polynomial<Float, Tag, order>::Coeff(y), x, y);
  }
};

template<typename Float, class Tag> struct Horner<Float, Tag, 0> {
  static Float Recurse(const Float term, const Float x)
  {
    return term * x + Polynomial<Float, Tag, 0>::Coeff();
  }
  static Float Eval(const Float x) { return Polynomial<Float, Tag, 0>::Coeff(); }
  //
  static Float Recurse(const Float term, const Float x, const Float y)
  {
    return term * x + Polynomial<Float, Tag, 0>::Coeff(y);
  }
  static Float Eval(const Float x, const Float y)
  {
    return Polynomial<Float, Tag, 0>::Coeff(y);
  }
};

  #define HORNER_COEFF(_Tag_, _i_, _c_)                                                  \
    template<typename Float> struct Polynomial<Float, _Tag_, _i_> {                      \
      static Float Coeff() { return Float(_c_); }                                        \
    }
  #define HORNER_COEFF2(_Tag_, _i_, _c_y_)                                               \
    template<typename Float> struct Polynomial<Float, _Tag_, _i_> {                      \
      static Float Coeff(const Float y) { return Float(_c_y_); }                         \
    }

  #define HORNER0(F, x, c0) (F)(c0)
  #define HORNER1(F, x, c1, c0) HORNER0(F, x, (F)(c1) * (x) + (F)(c0))
  #define HORNER2(F, x, c2, c1, c0) HORNER1(F, x, (F)(c2) * (x) + (F)(c1), c0)
  #define HORNER3(F, x, c3, c2, c1, c0) HORNER2(F, x, (F)(c3) * (x) + (F)(c2), c1, c0)
  #define HORNER4(F, x, c4, c3, c2, c1, c0)                                              \
    HORNER3(F, x, (F)(c4) * (x) + (F)(c3), c2, c1, c0)
  #define HORNER5(F, x, c5, c4, c3, c2, c1, c0)                                          \
    HORNER4(F, x, (F)(c5) * (x) + (F)(c4), c3, c2, c1, c0)
  #define HORNER6(F, x, c6, c5, c4, c3, c2, c1, c0)                                      \
    HORNER5(F, x, (F)(c6) * (x) + (F)(c5), c4, c3, c2, c1, c0)
  #define HORNER7(F, x, c7, c6, c5, c4, c3, c2, c1, c0)                                  \
    HORNER6(F, x, (F)(c7) * (x) + (F)(c6), c5, c4, c3, c2, c1, c0)
  #define HORNER8(F, x, c8, c7, c6, c5, c4, c3, c2, c1, c0)                              \
    HORNER7(F, x, (F)(c8) * (x) + (F)(c7), c6, c5, c4, c3, c2, c1, c0)
  #define HORNER9(F, x, c9, c8, c7, c6, c5, c4, c3, c2, c1, c0)                          \
    HORNER8(F, x, (F)(c9) * (x) + (F)(c8), c7, c6, c5, c4, c3, c2, c1, c0)

class BranchPoint {};
HORNER_COEFF(BranchPoint, 0, -1);
HORNER_COEFF(BranchPoint, 1, 1);
HORNER_COEFF(BranchPoint, 2, -0.333333333333333333e0);
HORNER_COEFF(BranchPoint, 3, 0.152777777777777777e0);
HORNER_COEFF(BranchPoint, 4, -0.79629629629629630e-1);
HORNER_COEFF(BranchPoint, 5, 0.44502314814814814e-1);
HORNER_COEFF(BranchPoint, 6, -0.25984714873603760e-1);
HORNER_COEFF(BranchPoint, 7, 0.15635632532333920e-1);
HORNER_COEFF(BranchPoint, 8, -0.96168920242994320e-2);
HORNER_COEFF(BranchPoint, 9, 0.60145432529561180e-2);
HORNER_COEFF(BranchPoint, 10, -0.38112980348919993e-2);
HORNER_COEFF(BranchPoint, 11, 0.24408779911439826e-2);
HORNER_COEFF(BranchPoint, 12, -0.15769303446867841e-2);
HORNER_COEFF(BranchPoint, 13, 0.10262633205076071e-2);
HORNER_COEFF(BranchPoint, 14, -0.67206163115613620e-3);
HORNER_COEFF(BranchPoint, 15, 0.44247306181462090e-3);
HORNER_COEFF(BranchPoint, 16, -0.29267722472962746e-3);
HORNER_COEFF(BranchPoint, 17, 0.19438727605453930e-3);
HORNER_COEFF(BranchPoint, 18, -0.12957426685274883e-3);
HORNER_COEFF(BranchPoint, 19, 0.86650358052081260e-4);

template<unsigned int order> class AsymptoticPolynomialB {};
// HORNER_COEFF(AsymptoticPolynomialB<0>, 0, 0);
// HORNER_COEFF(AsymptoticPolynomialB<0>, 1, -1);
// HORNER_COEFF(AsymptoticPolynomialB<1>, 0, 0);
// HORNER_COEFF(AsymptoticPolynomialB<1>, 1, 1);
HORNER_COEFF(AsymptoticPolynomialB<2>, 0, 0);
HORNER_COEFF(AsymptoticPolynomialB<2>, 1, -1);
HORNER_COEFF(AsymptoticPolynomialB<2>, 2, 1. / 2);
HORNER_COEFF(AsymptoticPolynomialB<3>, 0, 0);
HORNER_COEFF(AsymptoticPolynomialB<3>, 1, 1);
HORNER_COEFF(AsymptoticPolynomialB<3>, 2, -3. / 2);
HORNER_COEFF(AsymptoticPolynomialB<3>, 3, 1. / 3);
HORNER_COEFF(AsymptoticPolynomialB<4>, 0, 0);
HORNER_COEFF(AsymptoticPolynomialB<4>, 1, -1);
HORNER_COEFF(AsymptoticPolynomialB<4>, 2, 3);
HORNER_COEFF(AsymptoticPolynomialB<4>, 3, -11. / 6);
HORNER_COEFF(AsymptoticPolynomialB<4>, 4, 1. / 4);
HORNER_COEFF(AsymptoticPolynomialB<5>, 0, 0);
HORNER_COEFF(AsymptoticPolynomialB<5>, 1, 1);
HORNER_COEFF(AsymptoticPolynomialB<5>, 2, -5);
HORNER_COEFF(AsymptoticPolynomialB<5>, 3, 35. / 6);
HORNER_COEFF(AsymptoticPolynomialB<5>, 4, -25. / 12);
HORNER_COEFF(AsymptoticPolynomialB<5>, 5, 1. / 5);
class AsymptoticPolynomialA {};
// HORNER_COEFF2(AsymptoticPolynomialA, 0, (Horner<Float, AsymptoticPolynomialB<0>,
// 1>::Eval(y)));
HORNER_COEFF2(AsymptoticPolynomialA, 0, -y);
// HORNER_COEFF2(AsymptoticPolynomialA, 1, (Horner<Float,  AsymptoticPolynomialB<1>,
// 1>::Eval(y)));
HORNER_COEFF2(AsymptoticPolynomialA, 1, y);
HORNER_COEFF2(
  AsymptoticPolynomialA, 2, (Horner<Float, AsymptoticPolynomialB<2>, 2>::Eval(y)));
HORNER_COEFF2(
  AsymptoticPolynomialA, 3, (Horner<Float, AsymptoticPolynomialB<3>, 3>::Eval(y)));
HORNER_COEFF2(
  AsymptoticPolynomialA, 4, (Horner<Float, AsymptoticPolynomialB<4>, 4>::Eval(y)));
HORNER_COEFF2(
  AsymptoticPolynomialA, 5, (Horner<Float, AsymptoticPolynomialB<5>, 5>::Eval(y)));

template<typename Float, int order>
inline Float AsymptoticExpansionImpl(const Float a, const Float b)
{
  return a + Horner<Float, AsymptoticPolynomialA, order>::Eval(1 / a, b);
}

template<typename Float, int branch, int order> struct LogRecursionImpl {
  static constexpr int eSign = 2 * branch + 1;
  static Float Step(const Float logsx)
  {
    return logsx - log(eSign * LogRecursionImpl<Float, branch, order - 1>::Step(logsx));
  }
};

template<typename Float, int branch> struct LogRecursionImpl<Float, branch, 0> {
  static Float Step(const Float logsx) { return logsx; }
};

template<typename Float, int branch> class Branch {
public:
  template<int order> static Float BranchPointExpansion(const Float x)
  {
    return Horner<Float, BranchPoint, order>::Eval(
      eSign * sqrt(2 * (Float(std::numbers::e_v<Float>) * x + 1)));
  }

  // Asymptotic expansion: Corless et al. 1996, de Bruijn (1981)
  template<int order> static Float AsymptoticExpansion(const Float x)
  {
    const Float logsx = log(eSign * x);
    const Float logslogsx = log(eSign * logsx);
    return AsymptoticExpansionImpl<Float, order>(logsx, logslogsx);
  }

  // Logarithmic recursion
  template<int order> static Float LogRecursion(const Float x)
  {
    return LogRecursionImpl<Float, branch, order>::Step(log(eSign * x));
  }

private:
  static constexpr int eSign = 2 * branch + 1;
};

// iterations

template<typename Float> inline Float HalleyStep(const Float x, const Float w)
{
  const Float ew = exp(w);
  const Float wew = w * ew;
  const Float wewx = wew - x;
  const Float w1 = w + 1;
  return w - wewx / (ew * w1 - (w + 2) * wewx / (2 * w1));
}

template<typename Float> inline Float FritschStep(const Float x, const Float w)
{
  const Float z = log(x / w) - w;
  const Float w1 = w + 1;
  const Float q = 2 * w1 * (w1 + Float(2. / 3) * z);
  const Float eps = z / w1 * (q - z) / (q - 2 * z);
  return w * (1 + eps);
}

template<typename Float> inline Float SchroederStep(const Float x, const Float w)
{
  const Float y = x * exp(-w);
  const Float f0 = w - y;
  const Float f1 = 1 + y;
  const Float f00 = f0 * f0;
  const Float f11 = f1 * f1;
  const Float f0y = f0 * y;
  return w
    - 4 * f0 * (6 * f1 * (f11 + f0y) + f00 * y)
    / (f11 * (24 * f11 + 36 * f0y) + f00 * (6 * y * y + 8 * f1 * y + f0y));
}

template<typename Float, double IterationStep(const Float x, const Float w)>
struct Iterator {
  template<int n, class = void> struct Depth {
    static Float Recurse(const Float x, Float w)
    {
      return Depth<n - 1>::Recurse(x, IterationStep(x, w));
    }
  };

  // stop condition
  template<class T> struct Depth<1, T> {
    static Float Recurse(const Float x, const Float w) { return IterationStep(x, w); }
  };

  // identity
  template<class T> struct Depth<0, T> {
    static Float Recurse(const Float x, const Float w) { return w; }
  };
};

// Rational approximations

template<typename Float, int branch, int n> struct Pade {
  static inline Float Approximation(const Float x);
};

template<typename Float> struct Pade<Float, 0, 1> {
  static inline Float Approximation(const Float x)
  {
    return x
      * HORNER4(
             Float, x, 0.07066247420543414, 2.4326814530577687, 6.39672835731526,
             4.663365025836821, 0.99999908757381)
      / HORNER4(
             Float, x, 1.2906660139511692, 7.164571775410987, 10.559985088953114,
             5.66336307375819, 1);
  }
};

template<typename Float> struct Pade<Float, 0, 2> {
  static inline Float Approximation(const Float x)
  {
    const Float y = log(Float(0.5) * x) - 2;
    return 2
      + y
      * HORNER3(
          Float, y, 0.00006979269679670452, 0.017110368846615806, 0.19338607770900237,
          0.6666648896499793)
      / HORNER2(Float, y, 0.0188060684652668, 0.23451269827133317, 1);
  }
};

template<typename Float> struct Pade<Float, -1, 4> {
  static inline Float Approximation(const Float x)
  {
    return HORNER4(
             Float, x, -2793.4565508841197, -1987.3632221106518, 385.7992853617571,
             277.2362778379572, -7.840776922133643)
      / HORNER4(
             Float, x, 280.6156995997829, 941.9414019982657, 190.64429338894644,
             -63.93540494358966, 1);
  }
};

template<typename Float> struct Pade<Float, -1, 5> {
  static inline Float Approximation(const Float x)
  {
    const Float y = log(-x);
    return -exp(
      HORNER3(
        Float, y, 0.16415668298255184, -3.334873920301941, 2.4831415860003747,
        4.173424474574879)
      / HORNER3(
        Float, y, 0.031239411487374164, -1.2961659693400076, 4.517178492772906, 1));
  }
};

template<typename Float> struct Pade<Float, -1, 6> {
  static inline Float Approximation(const Float x)
  {
    const Float y = log(-x);
    return -exp(
      HORNER4(
        Float, y, 0.000026987243254533254, -0.007692106448267341, 0.28793461719300206,
        -1.5267058884647018, -0.5370669268991288)
      / HORNER4(
        Float, y, 3.6006502104930343e-6, -0.0015552463555591487, 0.08801194682489769,
        -0.8973922357575583, 1));
  }
};

template<typename Float> struct Pade<Float, -1, 7> {
  static inline Float Approximation(const Float x)
  {
    return -1
      - sqrt(
        HORNER4(
          Float, x, 988.0070769375508, 1619.8111957356814, 989.2017745708083,
          266.9332506485452, 26.875022558546036)
        / HORNER4(
          Float, x, -205.50469464210596, -270.0440832897079, -109.554245632316,
          -11.275355431307334, 1));
  }
};

inline double LambertW0(const double x)
{
  typedef double d;
  return Y5(
    (Branch<d, 0>::BranchPointExpansion<8>(x)), x < -0.367679,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(
      x, Branch<d, 0>::BranchPointExpansion<10>(x))),
    x < -0.311,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, 0, 1>::Approximation(x))),
    x < 1.38,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, 0, 2>::Approximation(x))),
    x < 236,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(
      x, Branch<d, 0>::AsymptoticExpansion<6 - 1>(x))));
}

inline double LambertWm1(const double x)
{
  typedef double d;
  return Y7(
    (Branch<d, -1>::BranchPointExpansion<8>(x)), x < -0.367579,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(
      x, Branch<d, -1>::BranchPointExpansion<4>(x))),
    x < -0.366079,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, -1, 7>::Approximation(x))),
    x < -0.289379,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, -1, 4>::Approximation(x))),
    x < -0.0509,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, -1, 5>::Approximation(x))),
    x < -0.000131826,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(x, Pade<d, -1, 6>::Approximation(x))),
    x < -6.30957e-31,
    (Iterator<d, HalleyStep<d>>::Depth<1>::Recurse(
      x, Branch<d, -1>::LogRecursion<3>(x))));
}

// instantiations
inline double LambertW(const int branch, const double x)
{
  switch (branch) {
    case -1:
      return LambertWm1(x);
    case 0:
      return LambertW0(x);
    default:
      return std::numeric_limits<double>::quiet_NaN();
  }
}

} // namespace utl

#endif
