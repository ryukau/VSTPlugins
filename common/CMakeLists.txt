cmake_minimum_required(VERSION 3.20)

add_library(UhhyouCommon OBJECT
  gui/style.cpp
  gui/splash.cpp)

# target_compile_definitions(UhhyouCommon PRIVATE VSTGUI_ENABLE_DEPRECATED_METHODS=0)
target_include_directories(UhhyouCommon PUBLIC "../lib/vst3sdk")
