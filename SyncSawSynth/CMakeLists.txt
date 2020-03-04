cmake_minimum_required(VERSION 3.10)

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
    source/dsp/dspcore.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)

set(target SyncSawSynth)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/4377ACAE737A45A89B64BA13A7C5E0B7_snapshot.png")

if(SMTG_MAC)
smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
target_sources(${target} PRIVATE resource/plug.rc)
endif()
