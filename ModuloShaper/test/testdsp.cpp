// (c) 2021 Takamitsu Endo
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

#define SET_PARAMETERS dsp->setParameters();

#include "../../test/fxtester.hpp"
#include "../source/dsp/dspcore.hpp"

// CMake provides this macro, but just in case.
#ifndef UHHYOU_PLUGIN_NAME
#define UHHYOU_PLUGIN_NAME "ModuloShaper"
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
