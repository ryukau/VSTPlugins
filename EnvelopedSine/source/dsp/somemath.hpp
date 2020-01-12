// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

namespace SomeDSP {

template<typename T> T somesin(T x);
template<> inline double somesin<double>(double x) { return ::sin(x); }
template<> inline float somesin<float>(float x) { return ::sinf(x); }

template<typename T> T somecos(T x);
template<> inline double somecos<double>(double x) { return ::cos(x); }
template<> inline float somecos<float>(float x) { return ::cosf(x); }

template<typename T> T sometan(T x);
template<> inline double sometan<double>(double x) { return ::tan(x); }
template<> inline float sometan<float>(float x) { return ::tanf(x); }

template<typename T> T someacos(T x);
template<> inline double someacos<double>(double x) { return ::acos(x); }
template<> inline float someacos(float x) { return ::acosf(x); }

template<typename T> T someatan2(T y, T x);
template<> inline double someatan2<double>(double y, double x) { return ::atan2(y, x); }
template<> inline float someatan2<float>(float y, float x) { return ::atan2f(y, x); }

template<typename T> T somesinh(T x);
template<> inline double somesinh<double>(double x) { return ::sinh(x); }
template<> inline float somesinh<float>(float x) { return ::sinhf(x); }

template<typename T> T sometanh(T x);
template<> inline double sometanh<double>(double x) { return ::tanh(x); }
template<> inline float sometanh<float>(float x) { return ::tanhf(x); }

template<typename T> T somepow(T x, T y);
template<> inline double somepow<double>(double x, double y) { return ::pow(x, y); }
template<> inline float somepow<float>(float x, float y) { return ::powf(x, y); }

template<typename T> T somesqrt(T x);
template<> inline double somesqrt<double>(double x) { return ::sqrt(x); }
template<> inline float somesqrt<float>(float x) { return ::sqrtf(x); }

template<typename T> T somelog(T x);
template<> inline double somelog<double>(double x) { return ::log(x); }
template<> inline float somelog<float>(float x) { return ::logf(x); }

template<typename T> T somelog10(T x);
template<> inline double somelog10<double>(double x) { return ::log10(x); }
template<> inline float somelog10<float>(float x) { return ::log10f(x); }

template<typename T> T someexp(T x);
template<> inline double someexp<double>(double x) { return ::exp(x); }
template<> inline float someexp<float>(float x) { return ::expf(x); }

template<typename T> T somefmod(T x, T y);
template<> inline double somefmod<double>(double x, double y) { return ::fmod(x, y); }
template<> inline float somefmod<float>(float x, float y) { return ::fmodf(x, y); }

template<typename T> T somefloor(T x);
template<> inline double somefloor<double>(double x) { return ::floor(x); }
template<> inline float somefloor<float>(float x) { return ::floorf(x); }

template<typename T> T someceil(T x);
template<> inline double someceil<double>(double x) { return ::ceil(x); }
template<> inline float someceil<float>(float x) { return ::ceilf(x); }

template<typename T> T someround(T x);
template<> inline double someround<double>(double x) { return ::round(x); }
template<> inline float someround<float>(float x) { return ::roundf(x); }

template<typename T> T somefabs(T x);
template<> inline double somefabs<double>(double x) { return ::fabs(x); }
template<> inline float somefabs<float>(float x) { return ::fabsf(x); }

} // namespace SomeDSP
