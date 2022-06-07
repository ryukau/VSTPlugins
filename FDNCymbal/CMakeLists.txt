set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(../common/cmake/non_simd.cmake)

if(TEST_PLUGIN)
  build_test()
else()
  set(plug_sources
    source/parameter.cpp
    source/gui/splashdraw.cpp
    source/editor.cpp
    source/plugprocessor.cpp
    source/plugfactory.cpp)
  build_vst3("${plug_sources}")
endif()
