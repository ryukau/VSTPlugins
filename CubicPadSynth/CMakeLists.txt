cmake_minimum_required(VERSION 3.20)


set(UHHYOU_USE_FFTW True)
include(../common/cmake/simd_x86_64_and_aarch64.cmake)

if(TEST_PLUGIN)
  build_test()
else()
  set(plug_sources
    source/parameter.cpp
    ../common/gui/splashdrawempty.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)
  build_vst3("${plug_sources}")
endif()
