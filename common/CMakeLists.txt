cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
endif()

project(UhhyouCommon)

add_library(UhhyouCommon OBJECT
  gui/style.cpp
  gui/splash.cpp)

# target_compile_definitions(UhhyouCommon PRIVATE VSTGUI_ENABLE_DEPRECATED_METHODS=0)
target_include_directories(UhhyouCommon PUBLIC "${SMTG_VSTGUI_ROOT}/vstgui4")
