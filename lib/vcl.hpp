// (c) 2019-2022 Takamitsu Endo
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

#if defined(__arm64__) || defined(__aarch64__)
  #include "sse2neon/sse2neon.h"
  #define MAX_VECTOR_SIZE 128
  #define _mm_getcsr() 1
  #define __X86INTRIN_H
  #define INSTRSET 6 // Use SSE4.2 for VCL. sse2neon doesn't support AVX or later.
#endif

#include "vcl/vectorclass.h"
