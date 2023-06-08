cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

add_library(UhhyouCommon OBJECT
  gui/style.cpp
  gui/splash.cpp)

target_include_directories(UhhyouCommon PUBLIC "../lib/vst3sdk")

if(UNIX)
  if(XCODE)
    set_target_properties(UhhyouCommon
      PROPERTIES
      XCODE_ATTRIBUTE_OSX_ARCHITECTURES "x86_64;arm64;arm64e"
      XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_64_BIT)"
    )
  else() # Linux branch.
    target_compile_options(UhhyouCommon PRIVATE -fPIC)
  endif()
endif()
