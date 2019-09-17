
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
    source/gui/waveview.hpp
    source/gui/waveview.cpp
    source/gui/knob.hpp
    source/gui/knob.cpp
    source/gui/splash.hpp
    source/gui/splash.cpp
)

set(target SevenDelay)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/CD3D8D67AE6E4322B39DDDC0CA4C35A1_snapshot.png")

if(SMTG_MAC)
smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
target_sources(${target} PRIVATE resource/plug.rc)
endif()
