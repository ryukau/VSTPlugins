cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(../common/cmake/runtime_simd_switch.cmake)

if(TEST_PLUGIN)
  build_test()
else()
  # VST 3 source files.
  set(plug_sources
    ../lib/vcl/instrset_detect.cpp
    source/parameter.cpp
    source/gui/splashdraw.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)

  build_vst3("${plug_sources}")
endif()
