// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#define SET_PARAMETERS dsp->setParameters(tempo);

#include "../../test/synthtester.hpp"
#include "../source/dsp/dspcore.hpp"

// CMake provides this macro, but just in case.
#ifndef UHHYOU_PLUGIN_NAME
  #define UHHYOU_PLUGIN_NAME "LightPadSynth"
#endif

#define OUT_DIR_PATH "snd/" UHHYOU_PLUGIN_NAME

int main()
{
#ifdef __linux__
  SynthTesterSimdRuntimeDispatch<DSPInterface, DSPCore_AVX512, DSPCore_AVX2, DSPCore_AVX>
    tester(UHHYOU_PLUGIN_NAME, OUT_DIR_PATH, 1);
#else
  SynthTesterSimdRuntimeDispatch<DSPInterface, DSPCore_AVX2, DSPCore_AVX> tester(
    UHHYOU_PLUGIN_NAME, OUT_DIR_PATH, 1);
#endif

  return tester.isFinished ? EXIT_SUCCESS : EXIT_FAILURE;
}
