// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#define SET_PARAMETERS dsp->setParameters();

#include "../../test/fxtester.hpp"
#include "../source/dsp/dspcore.hpp"

// CMake provides this macro, but just in case.
#ifndef UHHYOU_PLUGIN_NAME
  #define UHHYOU_PLUGIN_NAME "SoftClipper"
#endif

#define OUT_DIR_PATH "snd/" UHHYOU_PLUGIN_NAME

int main()
{
#ifdef __linux__
  FxTesterSimdRuntimeDispatch<DSPInterface, DSPCore_AVX512, DSPCore_AVX2, DSPCore_AVX>
    tester(UHHYOU_PLUGIN_NAME, OUT_DIR_PATH);
#else
  FxTesterSimdRuntimeDispatch<DSPInterface, DSPCore_AVX2, DSPCore_AVX> tester(
    UHHYOU_PLUGIN_NAME, OUT_DIR_PATH);
#endif

  return tester.isFinished ? EXIT_SUCCESS : EXIT_FAILURE;
}
