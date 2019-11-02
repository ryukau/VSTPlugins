set(plug_sources
    source/parameter.cpp
    source/plugcontroller.cpp
    source/plugfactory.cpp
    source/plugprocessor.cpp
    source/dsp/dspcore.cpp
    source/gui/knob.cpp
    source/gui/plugeditor.cpp
    source/gui/splash.cpp
)

set(target FDNCymbal)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/61E7934EE5524F099269C45A3F1C754B_snapshot.png")

if(SMTG_MAC)
smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
target_sources(${target} PRIVATE resource/plug.rc)
endif()
