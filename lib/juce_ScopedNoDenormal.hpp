/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/*
Modified by Takamitsu Endo in 2023.

`ScopedNoDenormals` originates in JUCE 7. Target platforms are simplified to x86_64 with
SSE and aarch64 with NEON.

Below is list of original JUCE 7 source files.

- JUCE/modules/juce_core/system/juce_TargetPlatform.h
- JUCE/modules/juce_audio_basics/juce_audio_basics.h
- JUCE/modules/juce_audio_basics/juce_audio_basics.cpp
- JUCE/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.h
- JUCE/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.cpp
*/

#pragma once

#if defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__)       \
  || defined(__aarch64__)
  #define ARM_CPU 1
#else
  #define INTEL_CPU 1
#endif

#ifdef INTEL_CPU
  #define USE_SSE_INTRINSICS 1
#endif

#if __ARM_NEON__
  #define USE_ARM_NEON 1
#endif

#if defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(__arm64__)         \
  || defined(__aarch64__)
  #define IS_64BIT 1
#endif

#include <cstdint>

// Windows specific.
#if _MSC_VER
  #include <float.h>
#endif

#if INTEL_CPU
  #include <immintrin.h>
#endif

namespace SomeDSP {

inline intptr_t getFpStatusRegister() noexcept
{
  intptr_t fpsr = 0;
#if INTEL_CPU && USE_SSE_INTRINSICS
  fpsr = static_cast<intptr_t>(_mm_getcsr());
#elif (IS_64BIT && ARM_CPU) || USE_ARM_NEON
  #if _MSC_VER
  // _control87 returns static values for x86 bits that don't exist on arm
  // to emulate x86 behaviour. We are only ever interested in de-normal bits
  // so mask out only those.
  fpsr = (intptr_t)(_control87(0, 0) & _MCW_DN);
  #else
    #if IS_64BIT
  asm volatile("mrs %0, fpcr" : "=r"(fpsr));
    #elif USE_ARM_NEON
  asm volatile("vmrs %0, fpscr" : "=r"(fpsr));
    #endif
  #endif
#endif

  return fpsr;
}

inline void setFpStatusRegister([[maybe_unused]] intptr_t fpsr) noexcept
{
#if INTEL_CPU && USE_SSE_INTRINSICS
  // the volatile keyword here is needed to workaround a bug in AppleClang 13.0
  // which aggressively optimises away the variable otherwise
  volatile auto fpsr_w = static_cast<uint32_t>(fpsr);
  _mm_setcsr(fpsr_w);
#elif (IS_64BIT && ARM_CPU) || USE_ARM_NEON
  #if _MSC_VER
  _control87((unsigned int)fpsr, _MCW_DN);
  #else
    #if IS_64BIT
  asm volatile("msr fpcr, %0" : : "ri"(fpsr));
    #elif USE_ARM_NEON
  asm volatile("vmsr fpscr, %0" : : "ri"(fpsr));
    #endif
  #endif
#endif
}

class ScopedNoDenormals {
public:
  ScopedNoDenormals()
  {
#if USE_SSE_INTRINSICS || (USE_ARM_NEON || (IS_64BIT && ARM_CPU))
  #if USE_SSE_INTRINSICS
    intptr_t mask = 0x8040;
  #else /*USE_ARM_NEON*/
    intptr_t mask = (1 << 24 /* FZ */);
  #endif

    fpsr = getFpStatusRegister();
    setFpStatusRegister(fpsr | mask);
#endif
  }

  ~ScopedNoDenormals() noexcept
  {
#if USE_SSE_INTRINSICS || (USE_ARM_NEON || (IS_64BIT && ARM_CPU))
    setFpStatusRegister(fpsr);
#endif
  }

private:
#if USE_SSE_INTRINSICS || (USE_ARM_NEON || (IS_64BIT && ARM_CPU))
  intptr_t fpsr;
#endif
};

#undef IS_64BIT
#undef USE_ARM_NEON
#undef USE_SSE_INTRINSICS
#undef INTEL_CPU
#undef ARM_CPU

} // namespace SomeDSP
