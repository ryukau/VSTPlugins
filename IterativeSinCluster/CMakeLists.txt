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

set(target IterativeSinCluster)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    add_definitions(/arch:AVX2)
elseif(UNIX)
    add_compile_options(-O3 -mavx2 -mfma)
endif()

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/17E56F33963943F6AB265180357446E6_snapshot.png")

if(SMTG_MAC)
smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
target_sources(${target} PRIVATE resource/plug.rc)
endif()
