cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(plug_sources
  source/parameter.cpp
  source/gui/splash.cpp
  source/gui/splashdraw.cpp
  source/dsp/dspcore.cpp
  source/plugprocessor.cpp
  source/editor.cpp
  source/plugfactory.cpp)

set(target TrapezoidSynth)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
include_directories(../common)
target_link_libraries(${target} PRIVATE UhhyouCommon base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/A2A458AD3697430F9CF48840E3E721EF_snapshot.png")

if(SMTG_MAC)
  smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
  target_sources(${target} PRIVATE resource/plug.rc)
endif()
