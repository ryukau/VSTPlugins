cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(UhhyouCommon)

add_library(UhhyouCommon OBJECT
  gui/style.cpp
  gui/optionmenu.cpp
  gui/slider.cpp
  gui/splash.cpp
  gui/tabview.cpp
  gui/plugeditor.cpp)

target_include_directories(UhhyouCommon PUBLIC ${SMTG_VSTGUI_ROOT}/vstgui4)
