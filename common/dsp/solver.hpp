// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace SomeDSP {

/**
Brent's method to find local minimum of scalar function. Translated from
`scipy.optimize.minimize_scalar`.
*/
template<typename T, typename F> std::pair<T, T> minimizeScalarBrent(F func)
{
  //////////////////////////////////////////////////////////////////
  // Bracket
  //////////////////////////////////////////////////////////////////
  constexpr T grow_limit = T(110);
  constexpr size_t maxiterBracket = 1000;

  constexpr auto _gold = T(1.618034); // golden ratio: (1.0+sqrt(5.0))/2.0
  constexpr auto _verysmall_num = std::numeric_limits<T>::epsilon();

  auto xa = T(0);
  auto xb = T(1);
  auto fa = func(xa);
  auto fb = func(xb);
  if (fa < fb) {
    std::swap(xa, xb);
    std::swap(fa, fb);
  }
  auto xc = xb + _gold * (xb - xa);
  auto fc = func(xc);
  size_t iter = 0;

  while (fc < fb) {
    auto tmp1 = (xb - xa) * (fb - fc);
    auto tmp2 = (xb - xc) * (fb - fa);
    auto val = tmp2 - tmp1;

    auto denom = std::abs(val) < _verysmall_num ? T(2) * _verysmall_num : T(2) * val;

    auto w = xb - ((xb - xc) * tmp2 - (xb - xa) * tmp1) / denom;
    auto wlim = xb + grow_limit * (xc - xb);

    if (iter > maxiterBracket) break;
    ++iter;

    T fw;
    if ((w - xc) * (xb - w) > 0) {
      fw = func(w);
      if (fw < fc) {
        xa = xb;
        xb = w;
        fa = fb;
        fb = fw;
        break;
      } else if (fw > fb) {
        xc = w;
        fc = fw;
        break;
      }
      w = xc + _gold * (xc - xb);
      fw = func(w);

    } else if ((w - wlim) * (wlim - xc) >= 0) {
      w = wlim;
      fw = func(w);
    } else if ((w - wlim) * (xc - w) > 0) {
      fw = func(w);
      if (fw < fc) {
        xb = xc;
        xc = w;
        w = xc + _gold * (xc - xb);
        fb = fc;
        fc = fw;
        fw = func(w);
      }
    } else {
      w = xc + _gold * (xc - xb);
      fw = func(w);
    }
    xa = xb;
    xb = xc;
    xc = w;
    fa = fb;
    fb = fc;
    fc = fw;
  }

  //////////////////////////////////////////////////////////////////
  // Brent's algorithm
  //////////////////////////////////////////////////////////////////
  constexpr auto _mintol = T(1.0e-11);
  constexpr auto _cg = T(0.3819660);
  constexpr size_t maxiter = 64;

  auto x = xb;
  auto w = xb;
  auto v = xb;

  auto fw = func(x);
  auto fv = fw;
  auto fx = fw;

  auto a = xa;
  auto b = xc;
  if (a >= b) std::swap(a, b);

  auto deltax = T(0);
  auto rat = T(0);
  iter = 0;

  while (iter < maxiter) {
    auto tol1 = T(1.48e-8) * std::abs(x) + _mintol;
    auto tol2 = T(2) * tol1;
    auto xmid = T(0.5) * (a + b);

    // check for convergence.
    if (std::abs(x - xmid) < (tol2 - T(0.5) * (b - a))) break;

    if (std::abs(deltax) <= tol1) {
      deltax = x >= xmid ? a - x : b - x;
      rat = _cg * deltax;
    } else {
      // do a parabolic step.
      auto tmp1 = (x - w) * (fx - fv);
      auto tmp2 = (x - v) * (fx - fw);
      auto p = (x - v) * tmp2 - (x - w) * tmp1;
      tmp2 = T(2) * (tmp2 - tmp1);
      if (tmp2 > 0) p = -p;
      tmp2 = std::abs(tmp2);
      auto dx_temp = deltax;
      deltax = rat;

      // check parabolic fit.
      if (
        p > tmp2 * (a - x) && p < tmp2 * (b - x)
        && std::abs(p) < std::abs(T(0.5) * tmp2 * dx_temp))
      {
        rat = p * T(1) / tmp2; // if parabolic step is useful.
        auto u = x + rat;
        if ((u - a) < tol2 || (b - u) < tol2) rat = xmid - x >= 0 ? tol1 : -tol1;
      } else {
        deltax = x >= xmid ? a - x : b - x;
        rat = _cg * deltax;
      }
    }

    auto u = std::abs(rat) < tol1 //
      ? (rat >= 0 ? x + tol1 : x - tol1)
      : x + rat;

    auto fu = func(u); // calculate new output value

    if (fu > fx) { // if it's bigger than current
      if (u < x)
        a = u;
      else
        b = u;

      if (fu <= fw || w == x) {
        v = w;
        w = u;
        fv = fw;
        fw = fu;
      } else if (fu <= fv || v == x || v == w) {
        v = u;
        fv = fu;
      }
    } else {
      if (u >= x)
        a = x;
      else
        b = x;

      v = w;
      w = x;
      x = u;
      fv = fw;
      fw = fx;
      fx = fu;
    }

    ++iter;
  }
  return std::pair<T, T>(x, fx);
}

} // namespace SomeDSP
