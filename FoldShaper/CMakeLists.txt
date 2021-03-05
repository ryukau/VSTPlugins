cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(target FoldShaper)
set(dspcore_avx "${target}_dspcore_avx")
set(dspcore_avx2 "${target}_dspcore_avx2")
set(dspcore_avx512 "${target}_dspcore_avx512")

# SIMD compilation.
add_library(${dspcore_avx} OBJECT source/dsp/dspcore.cpp)
add_library(${dspcore_avx2} OBJECT source/dsp/dspcore.cpp)
add_library(${dspcore_avx512} OBJECT source/dsp/dspcore.cpp)

if(MSVC)
  set(dsp_msvc_flag
    $<$<CONFIG:Debug>:/W4>
    $<$<CONFIG:Release>:/O2 /fp:fast /W4 /wd4127 /wd4324 /wd4458>)
  target_compile_options(${dspcore_avx} PRIVATE ${dsp_msvc_flag} /arch:AVX)
  target_compile_options(${dspcore_avx2} PRIVATE ${dsp_msvc_flag} /arch:AVX2)
  target_compile_options(${dspcore_avx512} PRIVATE ${dsp_msvc_flag} /arch:AVX512)
elseif(UNIX)
  if(APPLE)
    set(dsp_apple_clang_flag
      $<$<CONFIG:Debug>:-g -fPIC -fno-aligned-allocation -Wall>
      $<$<CONFIG:Release>:-O3 -fPIC -fno-aligned-allocation -Wall>)
    target_compile_options(${dspcore_avx} PRIVATE ${dsp_apple_clang_flag} -mavx)
    target_compile_options(${dspcore_avx2} PRIVATE ${dsp_apple_clang_flag} -mavx2 -mfma)
    target_compile_options(${dspcore_avx512} PRIVATE ${dsp_apple_clang_flag} -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
  else()
    set(dsp_linux_gcc_flag
      $<$<CONFIG:Debug>:-g -fPIC -Wall>
      $<$<CONFIG:Release>:-O3 -fPIC -Wall>)
    target_compile_options(${dspcore_avx} PRIVATE ${dsp_linux_gcc_flag} -mavx)
    target_compile_options(${dspcore_avx2} PRIVATE ${dsp_linux_gcc_flag} -mavx2 -mfma)
    target_compile_options(${dspcore_avx512} PRIVATE ${dsp_linux_gcc_flag} -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
  endif()
endif()

# VST 3 settings.
set(plug_sources
  ../lib/vcl/instrset_detect.cpp
  source/parameter.cpp
  source/gui/splashdraw.cpp
  source/plugprocessor.cpp
  source/editor.cpp
  source/plugfactory.cpp)

smtg_add_vst3plugin(${target} ${plug_sources})
if(APPLE)
  target_compile_options(${target} PRIVATE -fno-aligned-allocation)
endif()
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
include_directories(../common)
target_link_libraries(${target} PRIVATE
  ${dspcore_avx}
  ${dspcore_avx2}
  ${dspcore_avx512}
  UhhyouCommon
  base
  sdk
  vstgui_support)

# smtg_add_vst3_snapshot(${target} "resource/A4768485385E4EECA2063A1E9160A26C_snapshot.png")

if(SMTG_MAC)
  smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
  target_sources(${target} PRIVATE resource/plug.rc)
endif()
