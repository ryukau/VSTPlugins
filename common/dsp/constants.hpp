// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <cmath>

namespace SomeDSP {

constexpr double halfpi = 1.57079632679489661923;
constexpr double pi = 3.14159265358979323846;
constexpr double twopi = 6.28318530717958647692;

constexpr double halfSqrt2 = 0.7071067811865476;

template<typename T> inline T noteToFreq(T note)
{
  return T(440) * std::exp2((note - 69) / T(12));
}

template<typename T> inline T freqToNote(T freq)
{
  return T(69) + T(12) * std::log2(freq / T(440));
}

} // namespace SomeDSP
