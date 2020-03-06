cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(plug_sources
  source/parameter.cpp
  ../common/gui/barbox.cpp
  ../common/gui/button.cpp
  ../common/gui/checkbox.cpp
  ../common/gui/knob.cpp
  ../common/gui/optionmenu.cpp
  ../common/gui/rotaryknob.cpp
  ../common/gui/slider.cpp
  ../common/gui/splash.cpp
  source/gui/splashdraw.cpp
  ../common/gui/tabview.cpp
  ../common/gui/plugeditor.cpp
  source/editor.cpp
  source/dsp/dspcore.cpp
  source/plugprocessor.cpp
  source/plugfactory.cpp)

set(target FDNCymbal)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/61E7934EE5524F099269C45A3F1C754B_snapshot.png")

if(SMTG_MAC)
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
  smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
  target_sources(${target} PRIVATE resource/plug.rc)
endif()
