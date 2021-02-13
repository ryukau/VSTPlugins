// (c) 2020-2021 Takamitsu Endo
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

#include <cmath>

namespace SomeDSP {
constexpr double halfpi = 1.57079632679489661923;
constexpr double pi = 3.14159265358979323846;
constexpr double twopi = 6.28318530717958647692;

template<typename T> inline T noteToFreq(T note)
{
  return T(440) * std::exp2((note - 69) / T(12));
}

template<typename T> inline T freqToNote(T freq)
{
  return T(69) + T(12) * std::log2(freq / T(440));
}
} // namespace SomeDSP
