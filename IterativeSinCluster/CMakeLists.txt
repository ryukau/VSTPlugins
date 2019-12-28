cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(dspcore_avx OBJECT source/dsp/dspcore.cpp)
add_library(dspcore_avx2 OBJECT source/dsp/dspcore.cpp)
add_library(dspcore_avx512 OBJECT source/dsp/dspcore.cpp)

# TODO: Add debug options.
if(MSVC)
  target_compile_options(dspcore_avx PRIVATE /O2 /fp:fast /arch:AVX)
  target_compile_options(dspcore_avx2 PRIVATE /O2 /fp:fast /arch:AVX2)
  target_compile_options(dspcore_avx512 PRIVATE /O2 /fp:fast /arch:AVX512)
elseif(UNIX)
  if(APPLE)
    target_compile_options(dspcore_avx PRIVATE -O3 -fPIC -fno-aligned-allocation -mavx)
    target_compile_options(dspcore_avx2 PRIVATE -O3 -fPIC -fno-aligned-allocation -mavx2 -mfma)
    target_compile_options(dspcore_avx512 PRIVATE -O3 -fPIC -fno-aligned-allocation -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
  else()
    target_compile_options(dspcore_avx PRIVATE -O3 -fPIC -mavx)
    target_compile_options(dspcore_avx2 PRIVATE -O3 -fPIC -mavx2 -mfma)
    target_compile_options(dspcore_avx512 PRIVATE -O3 -fPIC -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
  endif()
endif()

# VST 3 settings.
set(target IterativeSinCluster)

set(plug_sources
  ../lib/vcl/instrset_detect.cpp
  source/parameter.cpp
  source/plugcontroller.cpp
  source/plugfactory.cpp
  source/plugprocessor.cpp
  source/gui/knob.cpp
  source/gui/plugeditor.cpp
  source/gui/splash.cpp)

smtg_add_vst3plugin(${target} ${plug_sources})
if(APPLE)
  target_compile_options(${target} PRIVATE -fno-aligned-allocation)
endif()
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE
  dspcore_avx
  dspcore_avx2
  dspcore_avx512
  base
  sdk
  vstgui_support)

smtg_add_vst3_snapshot(${target} "resource/17E56F33963943F6AB265180357446E6_snapshot.png")

if(SMTG_MAC)
  smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
  target_sources(${target} PRIVATE resource/plug.rc)
endif()
