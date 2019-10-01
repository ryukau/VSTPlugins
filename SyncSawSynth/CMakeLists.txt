set(plug_sources
    source/parameter.hpp
    source/parameter.cpp
    source/plugcontroller.hpp
    source/plugcontroller.cpp
    source/plugfactory.cpp
    source/plugprocessor.hpp
    source/plugprocessor.cpp
    source/version.hpp
    source/dsp/dspcore.hpp
    source/dsp/dspcore.cpp
    source/gui/plugeditor.hpp
    source/gui/plugeditor.cpp
    source/gui/knob.hpp
    source/gui/knob.cpp
    source/gui/splash.hpp
    source/gui/splash.cpp
)

set(target SyncSawSynth)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

if(SMTG_MAC)
smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
target_sources(${target} PRIVATE resource/plug.rc)
endif()
