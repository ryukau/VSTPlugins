cmake_minimum_required(VERSION 3.10)

project(UhhyouCommon)

add_library(UhhyouCommon OBJECT
  gui/button.cpp
  gui/checkbox.cpp
  gui/knob.cpp
  gui/optionmenu.cpp
  gui/rotaryknob.cpp
  gui/slider.cpp
  gui/splash.cpp
  gui/tabview.cpp
  gui/plugeditor.cpp)

target_include_directories(UhhyouCommon PUBLIC ${VSTGUI_ROOT}/vstgui4)
