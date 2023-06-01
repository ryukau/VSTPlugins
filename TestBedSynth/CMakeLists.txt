cmake_minimum_required(VERSION 3.12)


set(UHHYOU_USE_FFTW True)
include(../common/cmake/non_simd.cmake)

if(TEST_PLUGIN)
  build_test(source/dsp/oscillator.cpp)
else()
  set(plug_sources
    source/dsp/oscillator.cpp
    source/parameter.cpp
    source/gui/splashdraw.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)
  build_vst3("${plug_sources}")
endif()
