cmake_minimum_required(VERSION 3.20)

set(UHHYOU_USE_FFTW True)
include(../common/cmake/non_simd.cmake)

if(TEST_PLUGIN)
  build_test("")
else()
  # VST 3 source files.
  set(plug_sources
    source/dsp/spectralfilter.cpp
    source/parameter.cpp
    source/gui/splashdraw.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)

  build_vst3("${plug_sources}")
endif()
